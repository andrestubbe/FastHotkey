package fasthotkey;

/**
 * Minimal CLI demo for FastHotkey.
 * 
 * Run: java -cp "target/fasthotkey-1.0.0.jar" fasthotkey.Demo
 * 
 * Press Ctrl+Space to trigger. Press Ctrl+C to exit.
 */
public class Demo {
    
    public static void main(String[] args) throws Exception {
        System.out.println("Loading FastHotkey...");
        FastHotkey.loadLibrary();
        
        System.out.println("Registering Ctrl+Space hotkey...");
        FastHotkey.register(1, ModifierKeys.MOD_CONTROL, KeyCodes.VK_SPACE, id -> {
            System.out.println("Hotkey triggered! (id=" + id + ")");
        });
        
        FastHotkey.start();
        System.out.println("Running. Press Ctrl+Space to trigger, Ctrl+C to exit.");
        
        Thread.sleep(Long.MAX_VALUE);
    }
}
