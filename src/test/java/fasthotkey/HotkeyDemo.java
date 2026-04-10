package fasthotkey;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.RoundRectangle2D;

/**
 * FastHotkey Demo - Background service with modern notification popup
 * 
 * Run: java -cp "target/fasthotkey-1.0.0.jar" -Djava.library.path=build fasthotkey.HotkeyDemo
 * 
 * Press Ctrl+Space to trigger the hotkey and see a modern Windows-style notification.
 */
public class HotkeyDemo {
    
    private static TrayIcon trayIcon;
    private static int triggerCount = 0;
    private static JWindow popupWindow;
    
    public static void main(String[] args) {
        System.out.println("[DEMO] FastHotkey Demo Starting...");
        
        if (!SystemTray.isSupported()) {
            System.err.println("[DEMO] System tray not supported!");
            System.exit(1);
        }
        
        try {
            setupTrayIcon();
            setupHotkey();
            System.out.println("[DEMO] Running. Press Ctrl+Space.");
            
            // Keep main thread alive
            while (true) {
                Thread.sleep(1000);
            }
        } catch (Exception e) {
            System.err.println("[DEMO] Error: " + e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
    
    private static void setupTrayIcon() throws AWTException {
        SystemTray tray = SystemTray.getSystemTray();
        
        Image image = createTrayImage();
        trayIcon = new TrayIcon(image, "FastHotkey Demo");
        trayIcon.setImageAutoSize(true);
        
        PopupMenu menu = new PopupMenu();
        MenuItem exitItem = new MenuItem("Exit");
        exitItem.addActionListener(e -> {
            System.out.println("[DEMO] Exiting...");
            FastHotkey.stop();
            System.exit(0);
        });
        menu.add(exitItem);
        trayIcon.setPopupMenu(menu);
        
        trayIcon.addActionListener(e -> showInfo());
        tray.add(trayIcon);
        
        System.out.println("[DEMO] Tray icon added.");
    }
    
    private static void setupHotkey() throws Exception {
        System.out.println("[DEMO] Loading library...");
        FastHotkey.loadLibrary();
        System.out.println("[DEMO] Library loaded.");
        
        System.out.println("[DEMO] Registering Ctrl+Space hotkey...");
        boolean registered = FastHotkey.register(1, 
            ModifierKeys.MOD_CONTROL, 
            KeyCodes.VK_SPACE,
            id -> {
                triggerCount++;
                System.out.println("[DEMO] Hotkey triggered! Count: " + triggerCount);
                SwingUtilities.invokeLater(() -> showNotification(id));
            }
        );
        
        if (!registered) {
            throw new RuntimeException("Failed to register hotkey!");
        }
        System.out.println("[DEMO] Hotkey registered.");
        
        FastHotkey.start();
        System.out.println("[DEMO] Message loop started.");
    }
    
    /**
     * Modern Windows 11-style notification popup
     */
    private static void showNotification(int hotkeyId) {
        // Balloon notification
        trayIcon.displayMessage(
            "FastHotkey",
            "Ctrl+Space pressed (Count: " + triggerCount + ")",
            TrayIcon.MessageType.INFO
        );
        
        // Close existing popup
        if (popupWindow != null && popupWindow.isVisible()) {
            popupWindow.dispose();
        }
        
        // Create modern popup
        popupWindow = new JWindow();
        popupWindow.setAlwaysOnTop(true);
        popupWindow.setFocusable(false);
        
        // Rounded dark panel
        JPanel panel = new JPanel() {
            @Override
            protected void paintComponent(Graphics g) {
                Graphics2D g2 = (Graphics2D) g.create();
                g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
                
                // Windows 11 style dark background
                g2.setColor(new Color(43, 43, 43, 245));
                g2.fillRoundRect(0, 0, getWidth(), getHeight(), 8, 8);
                
                // Subtle border
                g2.setColor(new Color(255, 255, 255, 30));
                g2.drawRoundRect(0, 0, getWidth()-1, getHeight()-1, 8, 8);
                
                g2.dispose();
            }
        };
        panel.setLayout(new BorderLayout(12, 8));
        panel.setBorder(BorderFactory.createEmptyBorder(14, 16, 14, 16));
        panel.setOpaque(false);
        
        // Icon
        JLabel iconLabel = new JLabel("⌨️", SwingConstants.CENTER);
        iconLabel.setFont(new Font("Segoe UI Emoji", Font.PLAIN, 28));
        
        // Title
        JLabel titleLabel = new JLabel("Hotkey Triggered");
        titleLabel.setFont(new Font("Segoe UI", Font.BOLD, 13));
        titleLabel.setForeground(Color.WHITE);
        
        // Message
        JLabel msgLabel = new JLabel("<html>Ctrl + Space<br><b>Count:</b> " + triggerCount + " | ID: " + hotkeyId + "</html>");
        msgLabel.setFont(new Font("Segoe UI", Font.PLAIN, 12));
        msgLabel.setForeground(new Color(180, 180, 180));
        
        // Text panel
        JPanel textPanel = new JPanel(new GridLayout(2, 1, 4, 4));
        textPanel.setOpaque(false);
        textPanel.add(titleLabel);
        textPanel.add(msgLabel);
        
        panel.add(iconLabel, BorderLayout.WEST);
        panel.add(textPanel, BorderLayout.CENTER);
        
        popupWindow.add(panel);
        popupWindow.setSize(260, 80);
        
        // Position: bottom-right of screen
        Rectangle screen = GraphicsEnvironment.getLocalGraphicsEnvironment()
            .getDefaultScreenDevice().getDefaultConfiguration().getBounds();
        int x = screen.x + screen.width - popupWindow.getWidth() - 16;
        int y = screen.y + screen.height - popupWindow.getHeight() - 48;
        popupWindow.setLocation(x, y);
        
        // Fade in
        popupWindow.setOpacity(0f);
        popupWindow.setVisible(true);
        
        Timer fadeIn = new Timer(16, null);
        final float[] opacity = {0f};
        fadeIn.addActionListener(e -> {
            opacity[0] += 0.08f;
            if (opacity[0] >= 1f) {
                popupWindow.setOpacity(1f);
                fadeIn.stop();
            } else {
                popupWindow.setOpacity(opacity[0]);
            }
        });
        fadeIn.start();
        
        // Auto-close after 3.5 seconds with fade-out
        Timer closeTimer = new Timer(3500, e -> {
            Timer fadeOut = new Timer(16, null);
            final float[] outOp = {1f};
            fadeOut.addActionListener(e2 -> {
                outOp[0] -= 0.1f;
                if (outOp[0] <= 0f) {
                    popupWindow.dispose();
                    fadeOut.stop();
                } else {
                    popupWindow.setOpacity(outOp[0]);
                }
            });
            fadeOut.start();
        });
        closeTimer.setRepeats(false);
        closeTimer.start();
        
        // Click to close
        panel.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseClicked(MouseEvent e) {
                popupWindow.dispose();
            }
        });
    }
    
    private static void showInfo() {
        JOptionPane.showMessageDialog(null,
            "FastHotkey Demo\n\n" +
            "Hotkey: Ctrl+Space\n" +
            "Triggers: " + triggerCount + "\n\n" +
            "Right-click tray icon to exit.",
            "FastHotkey",
            JOptionPane.INFORMATION_MESSAGE);
    }
    
    private static Image createTrayImage() {
        java.awt.image.BufferedImage img = new java.awt.image.BufferedImage(16, 16, 
            java.awt.image.BufferedImage.TYPE_INT_ARGB);
        Graphics2D g2d = img.createGraphics();
        
        g2d.setColor(new Color(0, 120, 212));
        g2d.fillRoundRect(1, 4, 14, 10, 2, 2);
        
        g2d.setColor(Color.WHITE);
        g2d.fillRect(3, 6, 2, 2);
        g2d.fillRect(7, 6, 2, 2);
        g2d.fillRect(11, 6, 2, 2);
        g2d.fillRect(3, 10, 10, 2);
        
        g2d.dispose();
        return img;
    }
}
