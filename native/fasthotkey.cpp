#include "fasthotkey.h"
#include <iostream>

// Forward declarations
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
void checkHotkeys(UINT vkCode, BOOL isKeyDown);

// Track key states for modifier detection
static BOOL ctrlPressed = FALSE;
static BOOL altPressed = FALSE;
static BOOL shiftPressed = FALSE;
static BOOL winPressed = FALSE;

// Low-level keyboard hook procedure - intercepts ALL keys globally
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

// Check if key combination matches any registered hotkey
void checkHotkeys(UINT vkCode, BOOL isKeyDown) {
    HotkeyManager& manager = HotkeyManager::getInstance();
    
    for (const auto& pair : manager.hotkeys) {
        const HotkeyEntry& entry = pair.second;
        
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

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        // DLL loaded
    }
    return TRUE;
}

// Singleton instance
HotkeyManager& HotkeyManager::getInstance() {
    static HotkeyManager instance;
    return instance;
}

// Window procedure for handling hotkey messages
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

// Register a hotkey
bool HotkeyManager::registerHotkey(int id, UINT modifiers, UINT vkCode, jobject callback, JNIEnv* env) {
    // Registering hotkey
    
    // Store the hotkey entry
    HotkeyEntry entry;
    entry.id = id;
    entry.modifiers = modifiers;
    entry.vkCode = vkCode;
    entry.callback = env->NewGlobalRef(callback);
    
    hotkeys[id] = entry;
    
    // Register with Windows if message window exists
    if (messageWindow != NULL) {
        BOOL result = RegisterHotKey(messageWindow, id, modifiers, vkCode);
        // RegisterHotKey done
        return result != FALSE;
    }
    
    // Stored hotkey for later
    return true;
}

// Unregister a hotkey
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

// Unregister all hotkeys
void HotkeyManager::unregisterAllHotkeys(JNIEnv* env) {
    if (messageWindow != NULL) {
        for (const auto& pair : hotkeys) {
            UnregisterHotKey(messageWindow, pair.first);
            env->DeleteGlobalRef(pair.second.callback);
        }
    }
    hotkeys.clear();
}

// Notify Java callback
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

// Message loop
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

// Start message loop
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

// Stop message loop
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

// Check if running
bool HotkeyManager::isRunning() const {
    return running.load();
}

// JNI Implementation

JNIEXPORT jboolean JNICALL Java_fasthotkey_FastHotkey_nativeRegisterHotkey
    (JNIEnv* env, jclass clazz, jint id, jint modifiers, jint vkCode, jobject callback) {
    return HotkeyManager::getInstance().registerHotkey(id, modifiers, vkCode, callback, env) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_fasthotkey_FastHotkey_nativeUnregisterHotkey
    (JNIEnv* env, jclass clazz, jint id) {
    return HotkeyManager::getInstance().unregisterHotkey(id) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL Java_fasthotkey_FastHotkey_nativeStartMessageLoop
    (JNIEnv* env, jclass clazz) {
    HotkeyManager::getInstance().startMessageLoop(env);
}

JNIEXPORT void JNICALL Java_fasthotkey_FastHotkey_nativeStopMessageLoop
    (JNIEnv* env, jclass clazz) {
    HotkeyManager::getInstance().stopMessageLoop(env);
}

JNIEXPORT void JNICALL Java_fasthotkey_FastHotkey_nativeUnregisterAll
    (JNIEnv* env, jclass clazz) {
    HotkeyManager::getInstance().unregisterAllHotkeys(env);
}
