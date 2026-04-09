package fasthotkey;

/**
 * Callback interface for hotkey events.
 * Implement this interface to receive notifications when registered hotkeys are pressed.
 */
@FunctionalInterface
public interface HotkeyCallback {
    /**
     * Called when a registered hotkey is pressed.
     *
     * @param hotkeyId the ID of the hotkey that was triggered
     */
    void onHotkey(int hotkeyId);
}
