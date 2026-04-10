package fasthotkey;

import java.io.*;
import java.nio.file.*;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * FastHotkey - Ultra-fast global hotkey library for Java via JNI.
 * 
 * <p>Register system-wide hotkeys that work even when your Java application is not focused.
 * Unlike AWT's {@code Toolkit.getDefaultToolkit().addAWTEventListener()}, these hotkeys are
 * global and use the native Windows API.</p>
 * 
 * <p><strong>Usage example:</strong></p>
 * <pre>{@code
 * // Load native library
 * FastHotkey.loadLibrary();
 * 
 * // Register Ctrl+Shift+F1 as hotkey ID 1
 * FastHotkey.register(1, 
 *     ModifierKeys.MOD_CONTROL | ModifierKeys.MOD_SHIFT, 
 *     KeyCodes.VK_F1,
 *     id -> System.out.println("Hotkey " + id + " pressed!")
 * );
 * 
 * // Start listening for hotkey events (this blocks or runs in background)
 * FastHotkey.start();
 * }</pre>
 * 
 * @see <a href="https://github.com/andrestubbe/fasthotkey">GitHub Repository</a>
 * @version 1.0.0
 */
public final class FastHotkey {
    
    private static final AtomicBoolean loaded = new AtomicBoolean(false);
    private static final AtomicBoolean running = new AtomicBoolean(false);
    
    private FastHotkey() {}
    
    /**
     * Loads the native library from the JAR resources or from java.library.path.
     * Must be called before any other FastHotkey methods.
     * 
     * @throws UnsatisfiedLinkError if the native library cannot be loaded
     * @throws IOException if extracting the library from JAR fails
     */
    public static synchronized void loadLibrary() throws UnsatisfiedLinkError, IOException {
        if (loaded.get()) {
            return;
        }
        
        // Try to extract and load from JAR resources
        String libName = System.mapLibraryName("fasthotkey");
        String resourcePath = "/native/" + libName;
        
        try (InputStream in = FastHotkey.class.getResourceAsStream(resourcePath)) {
            if (in != null) {
                // Extract to temp directory
                Path tempDir = Files.createTempDirectory("fasthotkey");
                tempDir.toFile().deleteOnExit();
                
                Path libPath = tempDir.resolve(libName);
                Files.copy(in, libPath, StandardCopyOption.REPLACE_EXISTING);
                libPath.toFile().deleteOnExit();
                
                System.load(libPath.toAbsolutePath().toString());
            } else {
                // Fall back to java.library.path
                System.loadLibrary("fasthotkey");
            }
        }
        
        loaded.set(true);
        
        // Add shutdown hook to cleanup
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            if (running.get()) {
                stop();
            }
        }));
    }
    
    /**
     * Registers a global hotkey.
     * 
     * @param id a unique identifier for this hotkey (1-0xBFFF)
     * @param modifiers modifier keys (use {@link ModifierKeys} constants, combine with |)
     * @param vkCode the virtual key code (use {@link KeyCodes} constants)
     * @param callback the callback to invoke when hotkey is pressed
     * @return true if registration succeeded
     * @throws IllegalStateException if loadLibrary() was not called first
     * @throws IllegalArgumentException if id is out of range or callback is null
     */
    public static boolean register(int id, int modifiers, int vkCode, HotkeyCallback callback) {
        checkLoaded();
        if (id < 1 || id > 0xBFFF) {
            throw new IllegalArgumentException("Hotkey ID must be between 1 and 0xBFFF");
        }
        if (callback == null) {
            throw new IllegalArgumentException("Callback cannot be null");
        }
        return nativeRegisterHotkey(id, modifiers, vkCode, callback, HotkeyMode.AGGRESSIVE.ordinal());
    }
    
    /**
     * Registers a global hotkey with specified interception mode.
     * 
     * <p>Use {@link HotkeyMode#COOPERATIVE} for "well-behaved" hotkeys that should fail
     * if already registered by Windows or other apps.</p>
     * 
     * <p>Use {@link HotkeyMode#AGGRESSIVE} to override Windows hotkeys like Ctrl+Space,
     * Win+L, or any hotkey already in use.</p>
     * 
     * @param id unique hotkey identifier (1 to 0xBFFF)
     * @param modifiers modifier key combination (e.g., MOD_CONTROL | MOD_SHIFT)
     * @param vkCode virtual key code (e.g., KeyCodes.VK_SPACE)
     * @param callback lambda called when hotkey is pressed
     * @param mode COOPERATIVE or AGGRESSIVE interception mode
     * @return true if registration succeeded (COOPERATIVE may fail if hotkey taken)
     * @throws IllegalArgumentException if id is out of range or callback is null
     * @throws IllegalStateException if loadLibrary() was not called first
     * @see HotkeyMode
     * @since 1.0.0
     */
    public static boolean register(int id, int modifiers, int vkCode, HotkeyCallback callback, HotkeyMode mode) {
        checkLoaded();
        if (id < 1 || id > 0xBFFF) {
            throw new IllegalArgumentException("Hotkey ID must be between 1 and 0xBFFF");
        }
        if (callback == null) {
            throw new IllegalArgumentException("Callback cannot be null");
        }
        if (mode == null) {
            throw new IllegalArgumentException("Mode cannot be null");
        }
        return nativeRegisterHotkey(id, modifiers, vkCode, callback, mode.ordinal());
    }
    
    /**
     * Unregisters a previously registered hotkey.
     * 
     * @param id the hotkey ID to unregister
     * @return true if unregistration succeeded
     * @throws IllegalStateException if loadLibrary() was not called first
     */
    public static boolean unregister(int id) {
        checkLoaded();
        return nativeUnregisterHotkey(id);
    }
    
    /**
     * Unregisters all hotkeys. Called automatically on shutdown.
     * 
     * @throws IllegalStateException if loadLibrary() was not called first
     */
    public static void unregisterAll() {
        checkLoaded();
        nativeUnregisterAll();
    }
    
    /**
     * Starts the hotkey message loop. This must be called after registering hotkeys.
     * This method spawns a background thread and returns immediately.
     * 
     * @throws IllegalStateException if loadLibrary() was not called first
     */
    public static void start() {
        checkLoaded();
        if (running.get()) {
            return;
        }
        nativeStartMessageLoop();
        running.set(true);
    }
    
    /**
     * Stops the hotkey message loop and unregisters all hotkeys.
     * Called automatically on JVM shutdown.
     * 
     * @throws IllegalStateException if loadLibrary() was not called first
     */
    public static void stop() {
        checkLoaded();
        if (!running.get()) {
            return;
        }
        nativeStopMessageLoop();
        running.set(false);
    }
    
    /**
     * Returns whether the message loop is running.
     * 
     * @return true if the hotkey listener is active
     */
    public static boolean isRunning() {
        return running.get();
    }
    
    private static void checkLoaded() {
        if (!loaded.get()) {
            throw new IllegalStateException("loadLibrary() must be called first");
        }
    }
    
    // Native methods
    private static native boolean nativeRegisterHotkey(int id, int modifiers, int vkCode, HotkeyCallback callback, int mode);
    private static native boolean nativeUnregisterHotkey(int id);
    private static native void nativeStartMessageLoop();
    private static native void nativeStopMessageLoop();
    private static native void nativeUnregisterAll();
}
