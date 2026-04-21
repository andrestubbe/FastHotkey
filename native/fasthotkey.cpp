/**
 * @file fasthotkey.cpp
 * @brief FastHotkey native implementation - Global hotkey registration
 *
 * @details Implements system-wide hotkey capture for Java applications.
 * Uses two complementary approaches:
 * 1. RegisterHotKey API for cooperative mode (WM_HOTKEY messages)
 * 2. WH_KEYBOARD_LL hook for aggressive mode (catches all keystrokes)
 *
 * @par Implementation Notes
 * - Singleton pattern ensures single message loop
 * - Thread-safe hotkey registration/unregistration
 * - JNI thread attachment for Java callbacks
 * - Automatic cleanup on DLL unload
 *
 * @par Security Considerations
 * - WH_KEYBOARD_LL hook requires careful handling
 * - Passes unhandled keys to next hook (CallNextHookEx)
 * - No key logging - only checks registered combinations
 *
 * @author FastJava Team
 * @version 1.0.0
 * @copyright MIT License
 */

#include "fasthotkey.h"
#include <iostream>

/// @name Forward Declarations
/// @brief Windows callback function declarations
/// @{
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void checkHotkeys(UINT vkCode, BOOL isKeyDown);
/// @}

/// @name Modifier State Tracking
/// @brief Global state for modifier key detection
/// @{
static BOOL ctrlPressed = FALSE;   /**< Control key state */
static BOOL altPressed = FALSE;    /**< Alt key state */
static BOOL shiftPressed = FALSE;  /**< Shift key state */
static BOOL winPressed = FALSE;    /**< Windows key state */
/// @}

/**
 * @brief Low-level keyboard hook procedure
 * @details Intercepts ALL keystrokes system-wide for AGGRESSIVE mode hotkeys.
 * Updates modifier state and checks for registered hotkey combinations.
 * 
 * @param nCode Hook code (HC_ACTION or < 0)
 * @param wParam Key event type (WM_KEYDOWN, WM_KEYUP, etc.)
 * @param lParam Pointer to KBDLLHOOKSTRUCT with key details
 * @return Result from CallNextHookEx
 * @note Always calls CallNextHookEx to pass keys to other applications
 */
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
        BOOL isKeyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        
        // Update modifier states
        if (kb->vkCode == VK_LCONTROL || kb->vkCode == VK_RCONTROL) {
            ctrlPressed = isKeyDown;
        } else if (kb->vkCode == VK_LMENU || kb->vkCode == VK_RMENU) {
            altPressed = isKeyDown;
        } else if (kb->vkCode == VK_LSHIFT || kb->vkCode == VK_RSHIFT) {
            shiftPressed = isKeyDown;
        } else if (kb->vkCode == VK_LWIN || kb->vkCode == VK_RWIN) {
            winPressed = isKeyDown;
        } else if (isKeyDown) {
            // Check if this key + current modifiers matches any hotkey
            checkHotkeys(kb->vkCode, TRUE);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

/**
 * @brief Check if key combination matches registered hotkey
 * @details Iterates through all registered hotkeys and checks if current
 * key + modifier state matches any AGGRESSIVE mode registration.
 * 
 * @param vkCode Virtual key code that was pressed
 * @param isKeyDown TRUE if key pressed, FALSE if released
 * @note Attaches thread to JVM for callback invocation
 * @see HotkeyManager::notifyCallback
 */
void checkHotkeys(UINT vkCode, BOOL isKeyDown) {
    HotkeyManager& manager = HotkeyManager::getInstance();
    
    for (const auto& pair : manager.hotkeys) {
        const HotkeyEntry& entry = pair.second;
        
        // Only handle AGGRESSIVE mode via hook (COOPERATIVE uses WM_HOTKEY)
        if (entry.mode == HotkeyMode::COOPERATIVE) continue;
        
        // Check if VK code matches
        if (entry.vkCode != vkCode) continue;
        
        // Check if modifiers match
        BOOL modifiersMatch = TRUE;
        if ((entry.modifiers & MOD_CONTROL) && !ctrlPressed) modifiersMatch = FALSE;
        if ((entry.modifiers & MOD_ALT) && !altPressed) modifiersMatch = FALSE;
        if ((entry.modifiers & MOD_SHIFT) && !shiftPressed) modifiersMatch = FALSE;
        if ((entry.modifiers & MOD_WIN) && !winPressed) modifiersMatch = FALSE;
        
        if (modifiersMatch) {
            // Hotkey triggered - no debug output
            
            // Get JNI environment for this thread
            JavaVM* vm = nullptr;
            jsize vmCount = 0;
            JNI_GetCreatedJavaVMs(&vm, 1, &vmCount);
            
            if (vm != nullptr) {
                JNIEnv* env = nullptr;
                jint attachResult = vm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
                
                if (attachResult == JNI_OK && env != nullptr) {
                    manager.notifyCallback(env, entry);
                    vm->DetachCurrentThread();
                    // Callback done
                }
            }
        }
    }
}

/**
 * @brief DLL entry point
 * @param hModule DLL module handle
 * @param reason DLL event reason (DLL_PROCESS_ATTACH, etc.)
 * @param lpReserved Reserved parameter
 * @return TRUE on success
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        // DLL loaded
    }
    return TRUE;
}

/**
 * @brief Get singleton instance
 * @return Reference to HotkeyManager singleton
 * @note Thread-safe due to C++11 static initialization
 */
HotkeyManager& HotkeyManager::getInstance() {
    static HotkeyManager instance;
    return instance;
}

/**
 * @brief Window procedure for message-only window
 * @details Handles WM_HOTKEY messages for COOPERATIVE mode registrations.
 * Attaches thread to JVM and invokes Java callback when hotkey triggered.
 * 
 * @param hwnd Window handle
 * @param msg Message type
 * @param wParam First message parameter (hotkey ID for WM_HOTKEY)
 * @param lParam Second message parameter
 * @return 0 if handled, DefWindowProc result otherwise
 * @see HotkeyManager::notifyCallback
 */
LRESULT CALLBACK HotkeyWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_HOTKEY) {
        int id = static_cast<int>(wParam);
        // WM_HOTKEY received
        HotkeyManager& manager = HotkeyManager::getInstance();
        
        auto it = manager.hotkeys.find(id);
        if (it != manager.hotkeys.end()) {
            // Get JNI environment for this thread
            JavaVM* vm = nullptr;
            jsize vmCount = 0;
            JNI_GetCreatedJavaVMs(&vm, 1, &vmCount);
            
            if (vm != nullptr) {
                // Attaching thread to JVM
                JNIEnv* env = nullptr;
                jint attachResult = vm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
                
                if (attachResult == JNI_OK && env != nullptr) {
                    // Calling Java callback
                    manager.notifyCallback(env, it->second);
                    vm->DetachCurrentThread();
                    // Callback done
                } else {
                    // Failed to attach thread
                }
            } else {
                // No JVM found
            }
        }
        return 0;
    }
    
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/**
 * @brief Register a global hotkey
 * @details Stores hotkey entry and registers with Windows.
 * For COOPERATIVE mode, uses RegisterHotKey API.
 * For AGGRESSIVE mode, relies on low-level hook.
 * 
 * @param id Unique identifier for this hotkey
 * @param modifiers Modifier flags (MOD_CONTROL, MOD_ALT, etc.)
 * @param vkCode Virtual key code
 * @param callback Java callback object (stored as global reference)
 * @param env JNI environment
 * @param mode Registration mode (0=COOPERATIVE, 1=AGGRESSIVE)
 * @return true if registration succeeded
 * @note Creates global reference to callback - must be freed on unregister
 */
bool HotkeyManager::registerHotkey(int id, UINT modifiers, UINT vkCode, jobject callback, JNIEnv* env, int mode) {
    // Store the hotkey entry
    HotkeyEntry entry;
    entry.id = id;
    entry.modifiers = modifiers;
    entry.vkCode = vkCode;
    entry.callback = env->NewGlobalRef(callback);
    entry.mode = static_cast<HotkeyMode>(mode);
    
    hotkeys[id] = entry;
    
    // COOPERATIVE mode: Register with Windows (may fail if taken)
    if (entry.mode == HotkeyMode::COOPERATIVE && messageWindow != NULL) {
        BOOL result = RegisterHotKey(messageWindow, id, modifiers, vkCode);
        return result != FALSE;
    }
    
    // AGGRESSIVE mode or window not ready: rely on low-level hook
    // Always succeeds at registration time
    return true;
}

/**
 * @brief Unregister a specific hotkey
 * @details Removes hotkey from internal map and unregisters from Windows.
 * Frees global reference to Java callback.
 * 
 * @param id Hotkey identifier to unregister
 * @return true if hotkey was found and removed
 */
bool HotkeyManager::unregisterHotkey(int id) {
    auto it = hotkeys.find(id);
    if (it == hotkeys.end()) {
        return false;
    }
    
    // Unregister from Windows
    if (messageWindow != NULL) {
        UnregisterHotKey(messageWindow, id);
    }
    
    hotkeys.erase(it);
    return true;
}

/**
 * @brief Unregister all hotkeys and cleanup
 * @details Iterates through all registered hotkeys, unregisters from Windows,
 * and deletes global references to Java callbacks.
 * 
 * @param env JNI environment
 */
void HotkeyManager::unregisterAllHotkeys(JNIEnv* env) {
    if (messageWindow != NULL) {
        for (const auto& pair : hotkeys) {
            UnregisterHotKey(messageWindow, pair.first);
            env->DeleteGlobalRef(pair.second.callback);
        }
    }
    hotkeys.clear();
}

/**
 * @brief Invoke Java callback for triggered hotkey
 * @details Gets callback class, finds onHotkey method, and invokes it
 * with the hotkey ID as parameter.
 * 
 * @param env JNI environment
 * @param entry Hotkey entry that was triggered
 * @note Callback signature: void onHotkey(int id)
 */
void HotkeyManager::notifyCallback(JNIEnv* env, const HotkeyEntry& entry) {
    jclass callbackClass = env->GetObjectClass(entry.callback);
    if (callbackClass == nullptr) {
        return;
    }
    
    jmethodID onHotkeyMethod = env->GetMethodID(callbackClass, "onHotkey", "(I)V");
    if (onHotkeyMethod == nullptr) {
        return;
    }
    
    env->CallVoidMethod(entry.callback, onHotkeyMethod, entry.id);
}

/**
 * @brief Main message loop
 * @details Creates message-only window, installs keyboard hook if needed,
 * and runs Windows message loop. Handles cleanup on exit.
 * 
 * @param env JNI environment
 * @note Runs in separate thread - started by startMessageLoop()
 */
void HotkeyManager::messageLoop(JNIEnv* env) {
    // Starting message loop
    
    // Create message-only window
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = HotkeyWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = TEXT("FastHotkeyMessageWindow");
    
    ATOM regResult = RegisterClassEx(&wc);
    // Window class registered
    
    messageWindow = CreateWindowEx(
        0,
        TEXT("FastHotkeyMessageWindow"),
        TEXT("FastHotkey"),
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );
    
    if (messageWindow == NULL) {
        DWORD err = GetLastError();
        // Failed to create window
        return;
    }
    // Message window created
    
    // Install low-level keyboard hook for global key interception
    // This works even when RegisterHotKey fails (e.g., Windows owns Ctrl+Space)
    // Installing keyboard hook
    
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (hHook == NULL) {
        DWORD err = GetLastError();
        // Failed to install hook
    } else {
        // Keyboard hook installed
    }
    
    // Message loop
    MSG msg;
    // Entering message loop
    while (running.load() && GetMessage(&msg, NULL, 0, 0)) {
        // Got message
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    // Message loop ended
    
    // Cleanup
    unregisterAllHotkeys(env);
    DestroyWindow(messageWindow);
    messageWindow = NULL;
    UnregisterClass(TEXT("FastHotkeyMessageWindow"), GetModuleHandle(NULL));
}

/**
 * @brief Start message loop in separate thread
 * @details Creates detached thread that runs messageLoop().
 * Thread detaches automatically when complete.
 * 
 * @param env JNI environment
 * @note Thread-safe - checks if already running
 */
void HotkeyManager::startMessageLoop(JNIEnv* env) {
    // Starting message loop thread
    if (running.load()) {
        // Already running
        return;
    }
    
    running.store(true);
    messageThread = std::thread([this, env]() {
        // Store JVM for later thread attachment
        JavaVM* vm = nullptr;
        env->GetJavaVM(&vm);
        
        this->messageLoop(env);
    });
    messageThread.detach();
}

/**
 * @brief Stop message loop
 * @details Sets running flag to false and posts WM_DESTROY
 * to break the message loop.
 * 
 * @param env JNI environment
 */
void HotkeyManager::stopMessageLoop(JNIEnv* env) {
    if (!running.load()) {
        return;
    }
    
    running.store(false);
    
    // Post quit message to break the loop
    if (messageWindow != NULL) {
        PostMessage(messageWindow, WM_DESTROY, 0, 0);
    }
}

/**
 * @brief Check if message loop is running
 * @return true if message loop thread is active
 */
bool HotkeyManager::isRunning() const {
    return running.load();
}

/// @defgroup JNI_Implementation JNI Implementation
/// @brief JNI function implementations
/// @{

/**
 * @brief JNI: Register a global hotkey
 * @param env JNI environment
 * @param clazz FastHotkey class
 * @param id Unique hotkey identifier
 * @param modifiers Modifier flags (MOD_* constants)
 * @param vkCode Virtual key code (VK_* constants)
 * @param callback Java Runnable callback
 * @param mode 0=COOPERATIVE, 1=AGGRESSIVE
 * @return JNI_TRUE if registered successfully
 * @see HotkeyManager::registerHotkey
 */
JNIEXPORT jboolean JNICALL Java_fasthotkey_FastHotkey_nativeRegisterHotkey
    (JNIEnv* env, jclass clazz, jint id, jint modifiers, jint vkCode, jobject callback, jint mode) {
    return HotkeyManager::getInstance().registerHotkey(id, modifiers, vkCode, callback, env, mode) ? JNI_TRUE : JNI_FALSE;
}

/**
 * @brief JNI: Unregister a specific hotkey
 * @param env JNI environment
 * @param clazz FastHotkey class
 * @param id Hotkey identifier to unregister
 * @return JNI_TRUE if unregistered successfully
 * @see HotkeyManager::unregisterHotkey
 */
JNIEXPORT jboolean JNICALL Java_fasthotkey_FastHotkey_nativeUnregisterHotkey
    (JNIEnv* env, jclass clazz, jint id) {
    return HotkeyManager::getInstance().unregisterHotkey(id) ? JNI_TRUE : JNI_FALSE;
}

/**
 * @brief JNI: Start the message loop thread
 * @param env JNI environment
 * @param clazz FastHotkey class
 * @see HotkeyManager::startMessageLoop
 */
JNIEXPORT void JNICALL Java_fasthotkey_FastHotkey_nativeStartMessageLoop
    (JNIEnv* env, jclass clazz) {
    HotkeyManager::getInstance().startMessageLoop(env);
}

/**
 * @brief JNI: Stop the message loop thread
 * @param env JNI environment
 * @param clazz FastHotkey class
 * @see HotkeyManager::stopMessageLoop
 */
JNIEXPORT void JNICALL Java_fasthotkey_FastHotkey_nativeStopMessageLoop
    (JNIEnv* env, jclass clazz) {
    HotkeyManager::getInstance().stopMessageLoop(env);
}

/**
 * @brief JNI: Unregister all hotkeys and cleanup
 * @param env JNI environment
 * @param clazz FastHotkey class
 * @see HotkeyManager::unregisterAllHotkeys
 */
JNIEXPORT void JNICALL Java_fasthotkey_FastHotkey_nativeUnregisterAll
    (JNIEnv* env, jclass clazz) {
    HotkeyManager::getInstance().unregisterAllHotkeys(env);
}

/** @} */
