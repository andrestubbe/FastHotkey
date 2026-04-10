package fasthotkey;

/**
 * Hotkey registration mode determining how the hotkey interacts with Windows.
 * 
 * <p><strong>COOPERATIVE mode</strong> uses {@code RegisterHotKey} API:
 * <ul>
 *   <li>Windows can block it if another app registered the same hotkey</li>
 *   <li>Ctrl+Space, Win+L may fail (already owned by Windows)</li>
 *   <li>Better system citizen - doesn't override Windows shortcuts</li>
 * </ul>
 * 
 * <p><strong>AGGRESSIVE mode</strong> uses low-level keyboard hook ({@code WH_KEYBOARD_LL}):
 * <ul>
 *   <li>Always works, even for Windows-owned hotkeys</li>
 *   <li>Can override Ctrl+Space (IME), Win+L, etc.</li>
 *   <li>Intercepts ALL keystrokes system-wide</li>
 * </ul>
 * 
 * @since 1.0.0
 */
public enum HotkeyMode {
    
    /**
     * Cooperative mode using Windows {@code RegisterHotKey}.
     * Fails if hotkey is already registered by Windows or another app.
     * Good for "well-behaved" global shortcuts.
     */
    COOPERATIVE,
    
    /**
     * Aggressive mode using low-level keyboard hook.
     * Always intercepts the hotkey, even if Windows owns it.
     * Required for Ctrl+Space, Win+L and other system hotkeys.
     */
    AGGRESSIVE
}
