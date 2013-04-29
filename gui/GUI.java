/*
 * Java GUI for a DHT Client
 */

package gui;

import java.io.*;
import java.lang.String;
import java.awt.event.*;

import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JFileChooser;
import javax.swing.JScrollPane;
import javax.swing.JOptionPane;

import gui.MessageTypes;
import gui.Connection;
import gui.IconButton;
import gui.ConnectDialog;

/**
 *
 * @author Jerome Saarinen
 */

public class GUI extends javax.swing.JFrame implements java.awt.event.WindowListener {

    //Variables
    public String myPort;
    public String myAddress;
    public String serverPort;
    public String serverAddress;
    private Connection connection;
    private ConnectDialog connectDialog;
    private IconButton connectButton;
    private IconButton disconnectButton;
    private IconButton loadFileButton;
    private IconButton saveFileButton;
    private javax.swing.JSeparator separator;
    private javax.swing.JLabel progressLabel;
    private javax.swing.JScrollPane progressPane;
    private javax.swing.JLabel raportLabel;

    /**
     * Creates new form Client_GUI and centers it to the screen
     */
    public GUI() {
      addWindowListener(this);
      this.connection = new Connection();
      this.initComponents();
      this.bindEvents();
      this.render();
      this.setLocationRelativeTo(null); // Center window
    }

    /**
    * Implement window listener methods
    */

    public void windowClosing(WindowEvent e) {
      disconnect();
    }

    public void windowClosed(WindowEvent e) {}
    public void windowOpened(WindowEvent e) {}
    public void windowIconified(WindowEvent e) {}
    public void windowDeiconified(WindowEvent e) {}
    public void windowActivated(WindowEvent e) {}
    public void windowDeactivated(WindowEvent e) {}

    private void render() {
        // Main window
        // Center the main window, set its size and unable resize
        setLayout(null);
        setSize(270, 310);
        setResizable(false);
        
        // First button row
        connectButton.setBounds(10, 10, 120, 40);
        add(connectButton);
        disconnectButton.setBounds(140, 10, 120, 40);
        add(disconnectButton);
        
        // Second button row
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
     * This method is called from within the constructor to initialize the form.
     * It consists of primary form and a modal dialogue window for network connection
     */
    //@SuppressWarnings("unchecked")
    
    private void initComponents() {
        // Add title for the GUI
        setTitle("DHT Client");

        // Create the GUI component variables
        connectDialog = new ConnectDialog();

        connectButton = new IconButton("Connect", "icons/connect.png");
        disconnectButton = new IconButton("Disconnect", "icons/disconnect.png");
        loadFileButton = new IconButton("Load File from DHT", "icons/download.png");
        saveFileButton = new IconButton("Save file to DHT", "icons/upload.png");

        separator = new javax.swing.JSeparator();
        raportLabel = new javax.swing.JLabel();
        progressPane = new javax.swing.JScrollPane();
        progressLabel = new javax.swing.JLabel();

         // Add default window close operation 
        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);

        // Set texts, icons, dimensions and event listeners for buttons    
        // First 4 buttons are for the main window 
        // Buttons 5 & 6 are for the connection dialog window    
        loadFileButton.disable(); 
        saveFileButton.disable();
        //Icon imgicon = new ImageIcon("icons/disconnect.png");
        disconnectButton.disable();
        
        // Text for report label
        raportLabel.setText("Progress information:");

        // Initial text for progress terminal & alignment to top
        progressLabel.setText("<html><body>Waiting for connection...<br>");
        progressLabel.setVerticalAlignment(javax.swing.SwingConstants.TOP);
        progressPane.setViewportView(progressLabel);
    }

    private void connect() {
      this.connection.sendMessage(gui.MessageTypes.CONNECT, null);
      disconnectButton.enable();
      connectButton.disable();
      saveFileButton.enable();
      loadFileButton.enable();

      // Report connection info
      progressLabel.setText(progressLabel.getText() + "--- CONNECTION SUCCESSFUL --- <br> Waiting for files...<br>");
    }

    private void disconnect() {
      this.connection.sendMessage(gui.MessageTypes.DISCONNECT, null);
      disconnectButton.disable();
      connectButton.enable();
      saveFileButton.disable();
      loadFileButton.disable();

      // Report connection info
      progressLabel.setText(progressLabel.getText() + "--- DISCONNECTION SUCCESSFUL ---<br> Waiting for connection...<br>");
    }

    /* Bind UI elements click events
    */

    private void bindEvents() {
      connectDialog.submitButton.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          if (connectDialog.dataIsValid()) {
            connectDialog.hide();
            connect();
          } else {
            JOptionPane.showMessageDialog(null, "Please check your values.");
          }
        }
      });

      connectDialog.cancelButton.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          connectDialog.hideDialog();
        }
      });

      // Connect to server button
      connectButton.addMouseListener(new java.awt.event.MouseAdapter() {
        public void mouseClicked(java.awt.event.MouseEvent evt) {
          if (connectButton.isEnabled()) connectDialog.showDialog();
        }
      });

      // Disconnect from server button
      disconnectButton.addMouseListener(new java.awt.event.MouseAdapter() {
        public void mouseClicked(java.awt.event.MouseEvent evt) {
          if (disconnectButton.isEnabled()) disconnect();
        }
      });

      // Load file to DHT button
      loadFileButton.addMouseListener(new java.awt.event.MouseAdapter() {
        public void mouseClicked(java.awt.event.MouseEvent evt) {
          loadFileButtonMouseClicked(evt);
        }
      });

      // Save file to DHT button
      saveFileButton.addMouseListener(new java.awt.event.MouseAdapter() {
        public void mouseClicked(java.awt.event.MouseEvent evt) {
          saveFileButtonMouseClicked(evt);
        }
      });
    }

    // Connect to network button click event handler
    private void connectButtonMouseClicked(java.awt.event.MouseEvent evt) {
        if ((evt.getSource() == connectButton) && (connectButton.isEnabled() == true)) {
          connectDialog.showDialog();
        }
    }

    // Save file button click event handler
    private void saveFileButtonMouseClicked(java.awt.event.MouseEvent evt) {
        //Create a file chooser
        final JFileChooser fc = new JFileChooser();
        //In response to a button click:
        if ((evt.getSource() == saveFileButton) && saveFileButton.isEnabled()) {
            int returnVal = fc.showOpenDialog(GUI.this);
            if (returnVal == JFileChooser.APPROVE_OPTION) {
              File file = fc.getSelectedFile();
              connection.sendMessage(gui.MessageTypes.UPLOAD, file.getAbsolutePath());
              progressLabel.setText(progressLabel.getText()+ "\n" +"Saved file " + file.getName()+".<br>");
            }
        }
    }
   
    private void loadFileButtonMouseClicked(java.awt.event.MouseEvent evt) {
      if (loadFileButton.isEnabled()) {
        String fileName = (String) JOptionPane.showInputDialog(
                            this,
                            "Type in the file name to fetch:\n",
                            "Customized Dialog",
                            JOptionPane.PLAIN_MESSAGE,
                            null,
                            null,
                            null);

        //If a string was returned, say so.
        if ((fileName != null) && (fileName.length() > 0)) {
          connection.sendMessage(gui.MessageTypes.DOWNLOAD, fileName);
          progressLabel.setText(progressLabel.getText() + "\n" +"Downloaded file " + fileName + ".<br>");
        }
      }
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        /* Create the DHT Client GUI and display the main window  */
        java.awt.EventQueue.invokeLater(new Runnable() {
          public void run() {
              new GUI().setVisible(true);
          }
        });
    }
}
