package fasthotkey;

/**
 * Interactive Demo for FastHotkey.
 *
 * Demonstrates global hotkey registration, interception mode (COOPERATIVE and AGGRESSIVE),
 * and live event handling.
 */
public class Demo {

    public static void main(String[] args) throws Exception {
        System.out.println("==================================================");
        System.out.println("            ⚡ FastHotkey Interactive Demo         ");
        System.out.println("==================================================");

        // 1. Initialize native library
        System.out.print("Loading native library... ");
        FastHotkey.loadLibrary();
        System.out.println("[OK]");

        // 2. Register Hotkey 1: Ctrl+Alt+H (Aggressive)
        int hotkey1 = 1;
        boolean reg1 = FastHotkey.register(
            hotkey1,
            ModifierKeys.MOD_CONTROL | ModifierKeys.MOD_ALT,
            KeyCodes.VK_H,
            id -> System.out.println("🔥 [HOTKEY TRIGGERED] -> Ctrl + Alt + H (ID: " + id + ")"),
            HotkeyMode.AGGRESSIVE
        );

        // 3. Register Hotkey 2: Ctrl+Space (Aggressive override of Windows IME)
        int hotkey2 = 2;
        boolean reg2 = FastHotkey.register(
            hotkey2,
            ModifierKeys.MOD_CONTROL,
            KeyCodes.VK_SPACE,
            id -> System.out.println("⚡ [HOTKEY TRIGGERED] -> Ctrl + Space (ID: " + id + ")"),
            HotkeyMode.AGGRESSIVE
        );

        // 4. Register Hotkey 3: Ctrl+Shift+F12 (Cooperative)
        int hotkey3 = 3;
        boolean reg3 = FastHotkey.register(
            hotkey3,
            ModifierKeys.MOD_CONTROL | ModifierKeys.MOD_SHIFT,
            KeyCodes.VK_F12,
            id -> System.out.println("🎯 [HOTKEY TRIGGERED] -> Ctrl + Shift + F12 (ID: " + id + ")"),
            HotkeyMode.COOPERATIVE
        );

        System.out.println("\nRegistered Hotkeys:");
        System.out.println("  1. [Ctrl + Alt + H]      Status: " + (reg1 ? "ACTIVE (AGGRESSIVE)" : "FAILED"));
        System.out.println("  2. [Ctrl + Space]        Status: " + (reg2 ? "ACTIVE (AGGRESSIVE)" : "FAILED"));
        System.out.println("  3. [Ctrl + Shift + F12]  Status: " + (reg3 ? "ACTIVE (COOPERATIVE)" : "FAILED"));

        // 5. Start background message loop
        FastHotkey.start();
        System.out.println("\n[INFO] FastHotkey message loop is running.");
        System.out.println("Press registered hotkey combinations from any application window.");
        System.out.println("Press [Ctrl+C] to stop.\n");

        // Keep demo alive
        while (FastHotkey.isRunning()) {
            Thread.sleep(1000);
        }
    }
}
