/*
 * Java GUI for a DHT Client
 */

import java.io.*;
import java.lang.String;
import java.awt.event.*;
import java.util.Formatter;
import java.util.logging.Logger;
import java.util.logging.Level;

import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JFileChooser;
import javax.swing.JScrollPane;
import javax.swing.JOptionPane;

/**
 *
 * @author Jerome Saarinen
 */

public class Gui extends javax.swing.JFrame implements java.awt.event.WindowListener {

    //Variables
    public String myPort;
    public String myAddress;
    public String serverPort;
    public String serverAddress;
    private Connection connection;
    private IconButton connectButton;
    private IconButton disconnectButton;
    private IconButton loadFileButton;
    private IconButton saveFileButton;
    private javax.swing.JSeparator separator;
    private javax.swing.JLabel progressLabel;
    private javax.swing.JScrollPane progressPane;
    private javax.swing.JLabel raportLabel;
    private static final Logger logger = Logger.getLogger(Gui.class.getName());

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

      loadFileButton = new IconButton("Load File from DHT", "icons/download.png");
      saveFileButton = new IconButton("Save file to DHT", "icons/upload.png");

      separator = new javax.swing.JSeparator();
      raportLabel = new javax.swing.JLabel();
      progressPane = new javax.swing.JScrollPane();
      progressLabel = new javax.swing.JLabel();

       // Add default window close operation
      setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

      // Text for report label
      raportLabel.setText("Progress information             : ");

      // Initial text for progress terminal & alignment to top
      progressLabel.setVerticalAlignment(javax.swing.SwingConstants.TOP);
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
        setSize(270, 310);
        setResizable(false);

        // Button row
        saveFileButton.setBounds(10, 60, 120, 40);
        add(saveFileButton);
        loadFileButton.setBounds(140, 60, 120, 40);
        add(loadFileButton);

        // Static label
        raportLabel.setBounds(10, 100, 140, 60);
        add(raportLabel);

        // Network & file transfer pane
        progressPane.setBounds(10, 140, 250, 140);
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
    private void saveFileButtonMouseClicked(java.awt.event.MouseEvent evt) {
        //Create a file chooser
        final JFileChooser fc = new JFileChooser();
        //In response to a button click                     : 
        if ((evt.getSource() == saveFileButton) && saveFileButton.isEnabled()) {
            int returnVal = fc.showOpenDialog(Gui.this);
            if (returnVal == JFileChooser.APPROVE_OPTION) {
              File file = fc.getSelectedFile();
              FileMessage message = new FileMessage(file);
              try {
                message.save();
                progressLabel.setText("Saved file " + file.getName() + " to the DHT");
              } catch (IOException err) {
                progressLabel.setText(err.getMessage());
              }
            }
        }
    }

    /**
    * Load file button click event handler
    */
    private void loadFileButtonMouseClicked(java.awt.event.MouseEvent evt) {
        if (loadFileButton.isEnabled()) {
            String fileName = (String) JOptionPane.showInputDialog(
                            this,
                            "Type in the file name to fetch : \n",
                            "Customized Dialog",
                            JOptionPane.PLAIN_MESSAGE,
                            null,
                            null,
                            null);

            //If a string was returned, say so.
            if ((fileName != null) && (fileName.length() > 0)) {
                File file = new File(fileName);
                FileMessage message = new FileMessage(file);
                try {
                    message.fetch();
                    progressLabel.setText("Downloaded file " + fileName);
                } catch (IOException err) {
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
