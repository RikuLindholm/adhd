/*
 * Java GUI for a DHT Client
 */

import java.io.File;

import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JFileChooser;
import javax.swing.JScrollPane;


/**
 *
 * @author Jerome Saarinen
 */
public class Client_GUI extends javax.swing.JFrame {

    /**
     * Creates new form Client_GUI and centers it to the screen
     */
    public Client_GUI() {
        initComponents();
        initUI();
        this.setLocationRelativeTo(null);
        
        
    }
    
    private void initUI() {
        // First initialize the main window then the dialog window
        
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
        
        // Dialog window
        
        // Remove layout
        connectDialog.setLayout(null);
        
        // Text labels
        ownPortLabel.setBounds(10, 10, 100, 20);
        connectDialog.add(ownPortLabel);
        
        ownAddressLabel.setBounds(10, 30, 100, 20);
        connectDialog.add(ownAddressLabel);
        
        serverPortLabel.setBounds(10, 50, 100, 20);
        connectDialog.add(serverPortLabel);
        
        serverAddressLabel.setBounds(10, 70, 100, 20);
        connectDialog.add(serverAddressLabel);
        
        // Text areas
        ownPortTextField.setBounds(110, 10, 100, 20);
        connectDialog.add(ownPortTextField);
        
        ownAddressTextField.setBounds(110, 30, 100, 20);
        connectDialog.add(ownAddressTextField);
        
        serverPortTextField.setBounds(110, 50, 100, 20);
        connectDialog.add(serverPortTextField);
        
        serverAddressTextField.setBounds(110, 70, 100, 20);
        connectDialog.add(serverAddressTextField);
        
        // Submit & cancel buttons
        submitButton.setBounds(10, 100, 100, 40);
        connectDialog.add(submitButton);
        
        cancelButton.setBounds(110, 100, 100, 40);
        connectDialog.add(cancelButton);
    
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * It consists of primary form and a modal dialogue window for network connection
     */
    //@SuppressWarnings("unchecked")
    
    private void initComponents() {

        // Create the GUI component variables
        connectDialog = new javax.swing.JDialog();
        ownPortLabel = new javax.swing.JLabel();
        ownAddressLabel = new javax.swing.JLabel();
        serverPortLabel = new javax.swing.JLabel();
        serverAddressLabel = new javax.swing.JLabel();
        ownPortTextField = new javax.swing.JTextField();
        ownAddressTextField = new javax.swing.JTextField();
        serverPortTextField = new javax.swing.JTextField();
        serverAddressTextField = new javax.swing.JTextField();
        submitButton = new javax.swing.JButton();
        cancelButton = new javax.swing.JButton();
        loadFileButton = new javax.swing.JButton();
        saveFileButton = new javax.swing.JButton();
        connectButton = new javax.swing.JButton();
        separator = new javax.swing.JSeparator();
        raportLabel = new javax.swing.JLabel();
        progressPane = new javax.swing.JScrollPane();
        progressLabel = new javax.swing.JLabel();
        disconnectButton = new javax.swing.JButton();

         // Add default window close operation 
        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        
        // Add title for the GUI
        setTitle("DHT Client");


        // Set texts, icons, dimensions and event listeners for buttons    
        // First 4 buttons are for the main window 
        // Buttons 5 & 6 are for the connection dialog window    
        
        // Connect to server button
        connectButton.setIcon(new javax.swing.ImageIcon("Icons/connect.png")); 
        connectButton.setText("Connect");
        connectButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                connectButtonMouseClicked(evt);
            }
        });

        // Disconnect from server button
        //Icon imgicon = new ImageIcon("Icons/disconnect.png");
        disconnectButton.setIcon(new javax.swing.ImageIcon("Icons/disconnect.png")); 
        disconnectButton.setText("Disconnect");
        disconnectButton.setEnabled(false);
        disconnectButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                disconnectButtonMouseClicked(evt);
            }
        });

        // Load file to DHT button
        loadFileButton.setText("Load file from DHT");
        loadFileButton.setEnabled(false);
        loadFileButton.setIcon(new javax.swing.ImageIcon("Icons/download.png"));  
        loadFileButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                loadFileButtonMouseClicked(evt);
            }
        });

        // Save file to DHT button
        saveFileButton.setIcon(new javax.swing.ImageIcon("Icons/upload.png")); 
        saveFileButton.setText("Save file to DHT");
        saveFileButton.setEnabled(false);
        saveFileButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                saveFileButtonMouseClicked(evt);
            }
        });
        
        // Submit network settings button
        submitButton.setText("Submit");
        //submitButton.setIcon(new javax.swing.ImageIcon("Icons/submit.png"));
        submitButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                submitButtonMouseClicked(evt);
            }
        });
        
        // Cancel network settings button
        cancelButton.setText("Cancel");
        //cancelButton.setIcon(new javax.swing.ImageIcon("Icons/cancel.png"));
        cancelButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                cancelButtonMouseClicked(evt);
            }
        });

        // Set connection dialogue to center of screen, modal and unsizable 
        connectDialog.setMinimumSize(new java.awt.Dimension(250, 220));
        connectDialog.setModal(true);
        connectDialog.setResizable(false);

        // Set texts within the connection dialogue
        ownPortLabel.setText("Own port:");
        ownAddressLabel.setText("Own address:");
        serverPortLabel.setText("Server port:");
        serverAddressLabel.setText("Server address:");

        // Text for report label
        raportLabel.setText("Progress information:");

        // Initial text for progress terminal & alignment to top
        progressLabel.setText("<html><body>Waiting for connection...<br>");
        progressLabel.setVerticalAlignment(javax.swing.SwingConstants.TOP);
        progressPane.setViewportView(progressLabel);

        pack();
    }
  
    // Connect to network button click event handler
    private void connectButtonMouseClicked(java.awt.event.MouseEvent evt) {
        if ((evt.getSource() == connectButton) && (connectButton.isEnabled() == true)) {
            connectDialog.setLocationRelativeTo(null);
            connectDialog.setVisible(true);
        } 
    }

    // Disconnect button click event handler
    private void disconnectButtonMouseClicked(java.awt.event.MouseEvent evt) {
            //Change button states
            disconnectButton.setEnabled(false);
            connectButton.setEnabled(true);
            saveFileButton.setEnabled(false);
            loadFileButton.setEnabled(false);
            progressLabel.setText(progressLabel.getText() + "--- DISCONNECTION SUCCESSFUL ---<br> Waiting for connection...<br>");
    }

    // Save file button click event handler
    private void saveFileButtonMouseClicked(java.awt.event.MouseEvent evt) {
        //Create a file chooser
        final JFileChooser fc = new JFileChooser();
        //In response to a button click:
        if ((evt.getSource() == saveFileButton) && (saveFileButton.isEnabled() == true)) {
            int returnVal = fc.showOpenDialog(Client_GUI.this);
            if (returnVal == JFileChooser.APPROVE_OPTION) {
                File file = fc.getSelectedFile();
                progressLabel.setText(progressLabel.getText()+ "\n" +"Saved file " + file.getName()+".<br>");
            } else {
                progressLabel.setText(progressLabel.getText() + "File save failed.<br>");
            }
        }
    }

   
    private void loadFileButtonMouseClicked(java.awt.event.MouseEvent evt) {
         /*
         *   TO-DO Load file button click event handler 
         */
    }

    // Submit network settings button click event handler
    private void submitButtonMouseClicked(java.awt.event.MouseEvent evt) {
        if (evt.getSource() == submitButton) {
            connectDialog.setVisible(false);
            
            // Catch connection info to variables
            myPort = ownPortTextField.getText();
            myAddress = ownAddressTextField.getText();
            serverPort = serverPortTextField.getText();
            serverAddress = serverAddressTextField.getText();
            
            // Change button states
            disconnectButton.setEnabled(true);
            connectButton.setEnabled(false);
            saveFileButton.setEnabled(true);
            loadFileButton.setEnabled(true);
            
            // Report connection info
            progressLabel.setText(progressLabel.getText()+"--- CONNECTION SUCCESSFUL --- <br> Waiting for files...<br>");    
        }
    }

    // Cancel network settings button click event handler
    private void cancelButtonMouseClicked(java.awt.event.MouseEvent evt) {
        if (evt.getSource() == cancelButton) {
            connectDialog.setVisible(false);
            progressLabel.setText(progressLabel.getText() + "Connection failed.<br>");
        }
    }
    
    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        
        /* Create the DHT Client GUI and display the main window  */
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new Client_GUI().setVisible(true);
               
            }
        });
    }
    //Variables
    public String myPort;
    public String myAddress;
    public String serverPort;
    public String serverAddress;
    private javax.swing.JButton cancelButton;
    private javax.swing.JButton connectButton;
    private javax.swing.JDialog connectDialog;
    private javax.swing.JButton disconnectButton;
    private javax.swing.JSeparator separator;
    private javax.swing.JButton loadFileButton;
    private javax.swing.JLabel ownAddressLabel;
    private javax.swing.JTextField ownAddressTextField;
    private javax.swing.JLabel ownPortLabel;
    private javax.swing.JTextField ownPortTextField;
    private javax.swing.JLabel progressLabel;
    private javax.swing.JScrollPane progressPane;
    private javax.swing.JLabel raportLabel;
    private javax.swing.JButton saveFileButton;
    private javax.swing.JLabel serverAddressLabel;
    private javax.swing.JTextField serverAddressTextField;
    private javax.swing.JLabel serverPortLabel;
    private javax.swing.JTextField serverPortTextField;
    private javax.swing.JButton submitButton;
    
}