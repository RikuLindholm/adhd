/*
 * Java GUI for a DHT Client
 */

import java.io.*;
import java.lang.String;
import java.awt.event.*;
import java.util.logging.Logger;
import java.util.logging.Level;
import javax.swing.*;

/**
 *
 * @author Jerome Saarinen
 */

public class Gui extends JFrame implements java.awt.event.WindowListener {

    //Variables
    private IconButton loadFileButton;
    private IconButton saveFileButton;
    private JSeparator separator;
    private JLabel progressLabel;
    private JScrollPane progressPane;
    private JLabel raportLabel;
    private static final Logger logger = Logger.getLogger("gui");

    /**
    * Creates new form Client_GUI and centers it to the screen
    */
    public Gui(int port) {
      addWindowListener(this);
      this.initialize();
      this.bindEvents();
      this.render();
      this.setLocationRelativeTo(null); // Center window
      try {
        Connection.connect(port);
      } catch (IOException err) {
        logger.severe("Could not connect to client - not running?");
        System.exit(1);
      }
    }

    /**
    * This method is called from within the constructor to initialize the form.
    * It consists of primary form and a modal dialogue window for network connection
    */
    //@SuppressWarnings("unchecked")

    private void initialize() {
      // Add title for the GUI
      setTitle("DHT Client");

      loadFileButton = new IconButton("Load File from DHT", "gui/icons/download.png");
      saveFileButton = new IconButton("Save file to DHT", "gui/icons/upload.png");

      separator = new JSeparator();
      raportLabel = new JLabel();
      progressPane = new JScrollPane();
      progressLabel = new JLabel();

       // Add default window close operation
      setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);

      // Text for report label
      raportLabel.setText("Progress information:");

      // Initial text for progress terminal & alignment to top
      progressLabel.setVerticalAlignment(SwingConstants.TOP);
      progressPane.setViewportView(progressLabel);
    }

    /**
    * Bind click events for ui elements
    */
    private void bindEvents() {
        // Store file to DHT button
        saveFileButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
              saveFileButtonMouseClicked(evt);
            }
        });

        // Load file from DHT button
        loadFileButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                loadFileButtonMouseClicked(evt);
            }
        });
    }

    /**
    * Render and position the ui componenets in the window
    */

    private void render() {
        // Center the main window, set its size and unable resize
        setLayout(null);
        setSize(290, 190);
        setResizable(false);

        // Button row
        saveFileButton.setBounds(10, 10, 130, 40);
        add(saveFileButton);
        loadFileButton.setBounds(140, 10, 140, 40);
        add(loadFileButton);

        // Static label
        raportLabel.setBounds(10, 40, 140, 60);
        add(raportLabel);

        // Network & file transfer pane
        progressPane.setBounds(10, 80, 270, 80);
        add(progressPane);
        progressPane.setLayout(null);
        progressPane.setPreferredSize(progressPane.getSize());
        progressPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        progressLabel.setBounds(5, 5, 240, 130);
        progressPane.add(progressLabel);
        progressPane.repaint();
        progressLabel.repaint();
    }

    /**
    * Implement window listener methods
    */
    public void windowClosing(WindowEvent e) {
      try {
        Connection.disconnect();
      } catch (IOException err) {
        logger.log(Level.WARNING, "Could not cleanly disconnect from client", err);
      }
    }

    public void windowClosed(WindowEvent e) {}
    public void windowOpened(WindowEvent e) {}
    public void windowIconified(WindowEvent e) {}
    public void windowDeiconified(WindowEvent e) {}
    public void windowActivated(WindowEvent e) {}
    public void windowDeactivated(WindowEvent e) {}

    /**
    * Store file button click event handler
    */
    private void saveFileButtonMouseClicked(MouseEvent evt) {
        //Create a file chooser
        final JFileChooser fc = new JFileChooser();
        //In response to a button click                     :
        if ((evt.getSource() == saveFileButton) && saveFileButton.isEnabled()) {
            int returnVal = fc.showOpenDialog(Gui.this);
            if (returnVal == JFileChooser.APPROVE_OPTION) {
                File file = fc.getSelectedFile();
                String fileName = file.getName();
                FileMessage message = new FileMessage(file);
                try {
                    message.save();
                    logger.info("Successfully stored " + fileName);
                    progressLabel.setText("Saved file " + fileName + " to the DHT");
                } catch (IOException err) {
                    logger.log(Level.WARNING, "Error storing " + fileName, err);
                    progressLabel.setText("Error storing " + fileName);
                }
            }
        }
    }

    /**
    * Load file button click event handler
    */
    private void loadFileButtonMouseClicked(MouseEvent evt) {
        if (loadFileButton.isEnabled()) {
            SaveAsDialog dialog = new SaveAsDialog();
            String[] values = dialog.showDialog();
            String fileName = values[0];
            String targetPath = values[1];
            // If file name and target path were returned, save the file
            if (values[0] != null && values[1] != null) {
                File targetFile = new File(targetPath);
                File temp = new File(fileName);
                FileMessage message = new FileMessage(temp);
                try {
                    message.fetch(targetFile);
                    logger.info("Successfully loaded " + fileName);
                    progressLabel.setText("Successfully loaded " + fileName);
                } catch (IOException err) {
                    logger.warning(err.getMessage());
                    progressLabel.setText(err.getMessage());
                }
            }
        }
    }

    /**
    * Main method
    *
    * @param args the command line arguments
    */
    public static void main(String args[]) {
        // Set up logger
        logger.setLevel(Level.INFO);

        // Require ui port param
        if (args.length < 1) {
            logger.info("Usage: make run-gui <PORT=UIPORT>");
            return;
        }

        // Get ui port
        final int port = Integer.parseInt(args[0]);

        /* Create the DHT Client GUI and display the main window  */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new Gui(port).setVisible(true);
            }
        });
    }
}
