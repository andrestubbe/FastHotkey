#include "fasthotkey.h"
#include <iostream>

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        std::cout << "[DLL] FastHotkey DLL loaded!" << std::endl;
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
        std::cout << "[WINPROC] WM_HOTKEY received! ID=" << id << std::endl;
        std::cout.flush();
        HotkeyManager& manager = HotkeyManager::getInstance();
        
        auto it = manager.hotkeys.find(id);
        if (it != manager.hotkeys.end()) {
            // Get JNI environment for this thread
            JavaVM* vm = nullptr;
            jsize vmCount = 0;
            JNI_GetCreatedJavaVMs(&vm, 1, &vmCount);
            
            if (vm != nullptr) {
                std::cout << "[WINPROC] Attaching thread to JVM..." << std::endl;
                JNIEnv* env = nullptr;
                jint attachResult = vm->AttachCurrentThread(reinterpret_cast<void**>(&env), nullptr);
                
                if (attachResult == JNI_OK && env != nullptr) {
                    std::cout << "[WINPROC] Calling Java callback..." << std::endl;
                    manager.notifyCallback(env, it->second);
                    vm->DetachCurrentThread();
                    std::cout << "[WINPROC] Callback done!" << std::endl;
                } else {
                    std::cout << "[WINPROC] Failed to attach thread! Result=" << attachResult << std::endl;
                }
            } else {
                std::cout << "[WINPROC] No JVM found!" << std::endl;
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
    std::cout << "[REGISTER] Registering hotkey ID=" << id << ", modifiers=" << modifiers << ", vkCode=" << vkCode << std::endl;
    
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
        std::cout << "[REGISTER] RegisterHotKey result=" << result << std::endl;
        return result != FALSE;
    }
    
    std::cout << "[REGISTER] Stored hotkey (window not ready yet)" << std::endl;
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
    std::cout << "[MSGLOOP] Starting message loop..." << std::endl;
    std::cout.flush();
    
    // Create message-only window
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = HotkeyWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = TEXT("FastHotkeyMessageWindow");
    
    ATOM regResult = RegisterClassEx(&wc);
    std::cout << "[MSGLOOP] RegisterClassEx result=" << regResult << std::endl;
    std::cout.flush();
    
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
        std::cout << "[MSGLOOP] FAILED to create window! Error=" << err << std::endl;
        std::cout.flush();
        return;
    }
    std::cout << "[MSGLOOP] Message window created: " << messageWindow << std::endl;
    std::cout.flush();
    
    // Register all pending hotkeys
    std::cout << "[MSGLOOP] Registering " << hotkeys.size() << " pending hotkeys..." << std::endl;
    std::cout.flush();
    for (const auto& pair : hotkeys) {
        BOOL result = RegisterHotKey(messageWindow, pair.first, pair.second.modifiers, pair.second.vkCode);
        DWORD err = GetLastError();
        std::cout << "[MSGLOOP] RegisterHotKey ID=" << pair.first << " modifiers=" << pair.second.modifiers 
                  << " vk=" << pair.second.vkCode << " result=" << result;
        if (!result) {
            std::cout << " ERROR=" << err;
        }
        std::cout << std::endl;
        std::cout.flush();
    }
    
    // Message loop
    MSG msg;
    std::cout << "[MSGLOOP] Entering GetMessage loop..." << std::endl;
    std::cout.flush();
    while (running.load() && GetMessage(&msg, NULL, 0, 0)) {
        std::cout << "[MSGLOOP] Got message: " << msg.message << " wParam=" << msg.wParam << std::endl;
        std::cout.flush();
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    std::cout << "[MSGLOOP] Message loop ended!" << std::endl;
    std::cout.flush();
    
    // Cleanup
    unregisterAllHotkeys(env);
    DestroyWindow(messageWindow);
    messageWindow = NULL;
    UnregisterClass(TEXT("FastHotkeyMessageWindow"), GetModuleHandle(NULL));
}

// Start message loop
void HotkeyManager::startMessageLoop(JNIEnv* env) {
    std::cout << "[START] Starting message loop thread..." << std::endl;
    if (running.load()) {
        std::cout << "[START] Already running!" << std::endl;
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
