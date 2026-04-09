package fasthotkey;

/**
 * Demo application for FastHotkey library.
 * 
 * <p>Registers several global hotkeys and prints when they are pressed.
 * Press Ctrl+C in the console to exit.</p>
 */
public class Demo {
    
    public static void main(String[] args) throws Exception {
        System.out.println("========================================");
        System.out.println("FastHotkey v1.0 Demo");
        System.out.println("========================================");
        System.out.println();
        
        // Load native library
        System.out.println("Loading native library...");
        FastHotkey.loadLibrary();
        System.out.println("Library loaded successfully!");
        System.out.println();
        
        // Register hotkeys
        System.out.println("Registering hotkeys:");
        System.out.println("  Ctrl+Shift+F1  -> Show notification");
        System.out.println("  Ctrl+Alt+F2    -> Show timestamp");
        System.out.println("  Win+Q          -> Quit demo");
        System.out.println();
        
        // Hotkey 1: Ctrl+Shift+F1
        boolean ok1 = FastHotkey.register(1, 
            ModifierKeys.MOD_CONTROL | ModifierKeys.MOD_SHIFT, 
            KeyCodes.VK_F1,
            id -> {
                System.out.println("[" + getTimestamp() + "] Hotkey 1 pressed: Ctrl+Shift+F1");
                System.out.println("  -> This could show a notification!");
            }
        );
        
        // Hotkey 2: Ctrl+Alt+F2
        boolean ok2 = FastHotkey.register(2, 
            ModifierKeys.MOD_CONTROL | ModifierKeys.MOD_ALT, 
            KeyCodes.VK_F2,
            id -> {
                System.out.println("[" + getTimestamp() + "] Hotkey 2 pressed: Ctrl+Alt+F2");
                System.out.println("  -> Current timestamp: " + System.currentTimeMillis());
            }
        );
        
        // Hotkey 3: Win+Q to quit
        boolean ok3 = FastHotkey.register(3, 
            ModifierKeys.MOD_WIN, 
            KeyCodes.VK_Q,
            id -> {
                System.out.println("[" + getTimestamp() + "] Hotkey 3 pressed: Win+Q");
                System.out.println("  -> Quit signal received!");
                System.exit(0);
            }
        );
        
        // Check registration results
        if (!ok1) {
            System.err.println("Warning: Failed to register hotkey 1 (Ctrl+Shift+F1)");
        }
        if (!ok2) {
            System.err.println("Warning: Failed to register hotkey 2 (Ctrl+Alt+F2)");
        }
        if (!ok3) {
            System.err.println("Warning: Failed to register hotkey 3 (Win+Q)");
        }
        
        if (!ok1 && !ok2 && !ok3) {
            System.err.println("Error: Could not register any hotkeys. Exiting.");
            System.exit(1);
        }
        
        // Start listening
        System.out.println();
        System.out.println("Starting hotkey listener...");
        FastHotkey.start();
        System.out.println("Hotkey listener active!");
        System.out.println();
        
        // Keep running
        System.out.println("Press Win+Q or close this window to exit.");
        System.out.println("The hotkeys work globally - even when this window is not focused!");
        System.out.println();
        
        // Keep the main thread alive
        while (true) {
            Thread.sleep(1000);
        }
    }
    
    private static String getTimestamp() {
        return java.time.LocalTime.now().toString();
    }
}
