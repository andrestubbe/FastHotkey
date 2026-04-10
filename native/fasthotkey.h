#ifndef FASTHOTKEY_H
#define FASTHOTKEY_H

#include <jni.h>
#include <windows.h>
#include <map>
#include <string>
#include <thread>
#include <atomic>
#include <functional>

// Hotkey entry structure
struct HotkeyEntry {
    int id;
    UINT modifiers;
    UINT vkCode;
    jobject callback;
};

// Forward declaration for friend access
void checkHotkeys(UINT vkCode, BOOL isKeyDown);

// Global state
class HotkeyManager {
public:
    static HotkeyManager& getInstance();
    
    bool registerHotkey(int id, UINT modifiers, UINT vkCode, jobject callback, JNIEnv* env);
    bool unregisterHotkey(int id);
    void unregisterAllHotkeys(JNIEnv* env);
    void startMessageLoop(JNIEnv* env);
    void stopMessageLoop(JNIEnv* env);
    bool isRunning() const;
    void notifyCallback(JNIEnv* env, const HotkeyEntry& entry);
    
private:
    HotkeyManager() = default;
    ~HotkeyManager() = default;
    HotkeyManager(const HotkeyManager&) = delete;
    HotkeyManager& operator=(const HotkeyManager&) = delete;
    
    std::map<int, HotkeyEntry> hotkeys;
    std::atomic<bool> running{false};
    std::thread messageThread;
    HWND messageWindow = NULL;
    HHOOK hHook = NULL;
    
    friend LRESULT CALLBACK HotkeyWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    friend LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    friend void checkHotkeys(UINT vkCode, BOOL isKeyDown);
    void messageLoop(JNIEnv* env);
};

// JNI function declarations
extern "C" {
    JNIEXPORT jboolean JNICALL Java_fasthotkey_FastHotkey_nativeRegisterHotkey
        (JNIEnv* env, jclass clazz, jint id, jint modifiers, jint vkCode, jobject callback);
    
    JNIEXPORT jboolean JNICALL Java_fasthotkey_FastHotkey_nativeUnregisterHotkey
        (JNIEnv* env, jclass clazz, jint id);
    
    JNIEXPORT void JNICALL Java_fasthotkey_FastHotkey_nativeStartMessageLoop
        (JNIEnv* env, jclass clazz);
    
    JNIEXPORT void JNICALL Java_fasthotkey_FastHotkey_nativeStopMessageLoop
        (JNIEnv* env, jclass clazz);
    
    JNIEXPORT void JNICALL Java_fasthotkey_FastHotkey_nativeUnregisterAll
        (JNIEnv* env, jclass clazz);
}

#endif // FASTHOTKEY_H
