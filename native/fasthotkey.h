/**
 * @file fasthotkey.h
 * @brief FastHotkey JNI Header - Global hotkey registration for Java
 *
 * @details Provides system-wide hotkey registration using Windows RegisterHotKey
 * and low-level keyboard hooks. Supports two modes:
 * - COOPERATIVE: Uses Windows hotkey API (shares with other apps)
 * - AGGRESSIVE: Uses WH_KEYBOARD_LL hook (guaranteed to work)
 *
 * @par Architecture
 * - Singleton HotkeyManager for global state
 * - Message-only window for WM_HOTKEY messages
 * - Low-level keyboard hook for aggressive mode
 * - JNI thread attachment for callbacks
 *
 * @par Platform Requirements
 * - Windows 2000 or later
 * - Admin privileges not required for most hotkeys
 *
 * @author FastJava Team
 * @version 1.0.0
 * @copyright MIT License
 */

#ifndef FASTHOTKEY_H
#define FASTHOTKEY_H

#include <jni.h>
#include <windows.h>
#include <map>
#include <string>
#include <thread>
#include <atomic>
#include <functional>

/** @defgroup HotkeyModes Hotkey Registration Modes
 *  @brief Two strategies for global hotkey capture
 *  @{ */

/**
 * @brief Hotkey registration mode
 * @details COOPERATIVE uses Windows RegisterHotKey API and shares with other apps.
 *          AGGRESSIVE uses WH_KEYBOARD_LL hook and always works but may conflict.
 */
enum class HotkeyMode {
    COOPERATIVE = 0,  /**< Uses RegisterHotKey - can fail if taken */
    AGGRESSIVE = 1    /**< Uses low-level hook - always works */
};

/** @} */

/** @defgroup Structures Data Structures
 *  @brief Internal data structures
 *  @{ */

/**
 * @brief Registered hotkey entry
 * @details Stores all information needed to identify and callback
 *          when a hotkey is triggered.
 */
struct HotkeyEntry {
    int id;              /**< Unique hotkey identifier */
    UINT modifiers;      /**< Modifier flags (MOD_CONTROL, MOD_ALT, etc.) */
    UINT vkCode;         /**< Virtual key code */
    jobject callback;    /**< Java callback object (global reference) */
    HotkeyMode mode;     /**< Registration mode (COOPERATIVE/AGGRESSIVE) */
};

/** @} */

// Forward declaration for friend access
void checkHotkeys(UINT vkCode, BOOL isKeyDown);

/** @defgroup Manager Hotkey Manager
 *  @brief Singleton manager for all hotkey operations
 *  @{ */

/**
 * @brief Singleton hotkey manager class
 * @details Manages all hotkey registrations, message loop, and callbacks.
 *          Thread-safe singleton pattern with automatic cleanup.
 */
class HotkeyManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to HotkeyManager instance
     */
    static HotkeyManager& getInstance();
    
    /**
     * @brief Register a global hotkey
     * @param id Unique identifier for this hotkey
     * @param modifiers Modifier flags (MOD_CONTROL, MOD_ALT, MOD_SHIFT, MOD_WIN)
     * @param vkCode Virtual key code (VK_* constants)
     * @param callback Java callback object (stored as global ref)
     * @param env JNI environment
     * @param mode 0=COOPERATIVE, 1=AGGRESSIVE
     * @return true if registration succeeded
     */
    bool registerHotkey(int id, UINT modifiers, UINT vkCode, jobject callback, JNIEnv* env, int mode = 1);
    
    /**
     * @brief Unregister a specific hotkey
     * @param id Hotkey identifier to unregister
     * @return true if unregistration succeeded
     */
    bool unregisterHotkey(int id);
    
    /**
     * @brief Unregister all hotkeys and cleanup
     * @param env JNI environment
     */
    void unregisterAllHotkeys(JNIEnv* env);
    
    /**
     * @brief Start message loop in separate thread
     * @param env JNI environment
     * @note Creates message window and installs hook if needed
     */
    void startMessageLoop(JNIEnv* env);
    
    /**
     * @brief Stop message loop
     * @param env JNI environment
     */
    void stopMessageLoop(JNIEnv* env);
    
    /**
     * @brief Check if message loop is running
     * @return true if running
     */
    bool isRunning() const;
    
    /**
     * @brief Invoke Java callback for triggered hotkey
     * @param env JNI environment
     * @param entry Hotkey entry that was triggered
     */
    void notifyCallback(JNIEnv* env, const HotkeyEntry& entry);
    
private:
    HotkeyManager() = default;
    ~HotkeyManager() = default;
    HotkeyManager(const HotkeyManager&) = delete;
    HotkeyManager& operator=(const HotkeyManager&) = delete;
    
    std::map<int, HotkeyEntry> hotkeys;      /**< Map of registered hotkeys */
    std::atomic<bool> running{false};        /**< Message loop running flag */
    std::thread messageThread;               /**< Message loop thread */
    HWND messageWindow = NULL;               /**< Message-only window handle */
    HHOOK hHook = NULL;                      /**< Low-level keyboard hook handle */
    
    friend LRESULT CALLBACK HotkeyWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    friend LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    friend void checkHotkeys(UINT vkCode, BOOL isKeyDown);
    
    /**
     * @brief Main message loop
     * @param env JNI environment
     */
    void messageLoop(JNIEnv* env);
};

/** @} */

/** @defgroup JNI JNI Functions
 *  @brief Java Native Interface exports
 *  @{ */

/**
 * @brief Register a global hotkey
 * @param env JNI environment
 * @param clazz FastHotkey class
 * @param id Unique hotkey identifier
 * @param modifiers Modifier flags
 * @param vkCode Virtual key code
 * @param callback Java callback Runnable
 * @param mode 0=COOPERATIVE, 1=AGGRESSIVE
 * @return JNI_TRUE if registered successfully
 */
JNIEXPORT jboolean JNICALL Java_fasthotkey_FastHotkey_nativeRegisterHotkey
    (JNIEnv* env, jclass clazz, jint id, jint modifiers, jint vkCode, jobject callback, jint mode);

/**
 * @brief Unregister a specific hotkey
 * @param env JNI environment
 * @param clazz FastHotkey class
 * @param id Hotkey identifier to unregister
 * @return JNI_TRUE if unregistered successfully
 */
JNIEXPORT jboolean JNICALL Java_fasthotkey_FastHotkey_nativeUnregisterHotkey
    (JNIEnv* env, jclass clazz, jint id);

/**
 * @brief Start the message loop thread
 * @param env JNI environment
 * @param clazz FastHotkey class
 */
JNIEXPORT void JNICALL Java_fasthotkey_FastHotkey_nativeStartMessageLoop
    (JNIEnv* env, jclass clazz);

/**
 * @brief Stop the message loop thread
 * @param env JNI environment
 * @param clazz FastHotkey class
 */
JNIEXPORT void JNICALL Java_fasthotkey_FastHotkey_nativeStopMessageLoop
    (JNIEnv* env, jclass clazz);

/**
 * @brief Unregister all hotkeys and cleanup
 * @param env JNI environment
 * @param clazz FastHotkey class
 */
JNIEXPORT void JNICALL Java_fasthotkey_FastHotkey_nativeUnregisterAll
    (JNIEnv* env, jclass clazz);

/** @} */

#endif // FASTHOTKEY_H
