package fasthotkey;

/**
 * Minimal CLI demo for FastHotkey.
 * 
 * Run: java -cp "target/fasthotkey-1.0.0.jar;target/test-classes" fasthotkey.Demo
 * 
 * Press Ctrl+Shift+H to trigger. Press Ctrl+C to exit.
 */
public class Demo {
    
    public static void main(String[] args) throws Exception {
        // Clear screen using Windows CLS command
        new ProcessBuilder("cmd", "/c", "cls").inheritIO().start().waitFor();

        System.out.println("");
        System.out.println("FastHotkey Demo");
        System.out.println("===============");
        
        FastHotkey.loadLibrary();
        
        boolean registered = FastHotkey.register(1, 
            ModifierKeys.MOD_CONTROL | ModifierKeys.MOD_SHIFT, 
            KeyCodes.VK_H,
            id -> System.out.println("Ctrl+Shift+H triggered!")
        );
        
        if (!registered) {
            System.err.println("Failed to register hotkey!");
            System.exit(1);
        }
        
        FastHotkey.start();
        System.out.println("Press Ctrl+Shift+H to test. Ctrl+C to exit.");
        System.out.println();
        
        Thread.sleep(Long.MAX_VALUE);
    }
}
