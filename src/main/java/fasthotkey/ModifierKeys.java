package fasthotkey;

/**
 * Modifier key flags for Windows API.
 * Combine with bitwise OR when registering hotkeys.
 */
public final class ModifierKeys {
    private ModifierKeys() {}

    /** Either ALT key must be held down. */
    public static final int MOD_ALT = 0x0001;

    /** Either CTRL key must be held down. */
    public static final int MOD_CONTROL = 0x0002;

    /** Either SHIFT key must be held down. */
    public static final int MOD_SHIFT = 0x0004;

    /** Either WINDOWS key must be held down. */
    public static final int MOD_WIN = 0x0008;

    /** Changes the hotkey behavior so that the keyboard auto-repeat does not yield multiple hotkey notifications. */
    public static final int MOD_NOREPEAT = 0x4000;

    /**
     * Combines multiple modifiers into a single value.
     *
     * @param modifiers the modifiers to combine
     * @return the combined modifier value
     */
    public static int combine(int... modifiers) {
        int result = 0;
        for (int mod : modifiers) {
            result |= mod;
        }
        return result;
    }
}
