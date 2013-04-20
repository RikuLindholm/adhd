/*
 * Java GUI for a DHT Client
 */
package GUI;

import java.io.File;
import javax.swing.JFileChooser;

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
         this.setLocationRelativeTo(null);
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * It consists of primary form and a modal dialogue window for network connection
     */
    @SuppressWarnings("unchecked")
    
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
        jSeparator1 = new javax.swing.JSeparator();
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
        connectButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/GUI/Icons/connect.png"))); 
        connectButton.setText("Connect");
        connectButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                connectButtonMouseClicked(evt);
            }
        });

        // Disconnect from server button
        disconnectButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/GUI/Icons/disconnect.png"))); 
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
        loadFileButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/GUI/Icons/download.png")));  
        loadFileButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                loadFileButtonMouseClicked(evt);
            }
        });

        // Save file to DHT button
        saveFileButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/GUI/Icons/upload.png"))); 
        saveFileButton.setText("Save file to DHT");
        saveFileButton.setEnabled(false);
        saveFileButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                saveFileButtonMouseClicked(evt);
            }
        });
        
        // Submit network settings button
        submitButton.setText("Submit");
        submitButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/GUI/Icons/submit.png")));
        submitButton.setMaximumSize(new java.awt.Dimension(84, 38));
        submitButton.setMinimumSize(new java.awt.Dimension(84, 38));
        submitButton.setPreferredSize(new java.awt.Dimension(84, 38)); 
        submitButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                submitButtonMouseClicked(evt);
            }
        });
        
        // Cancel network settings button
        cancelButton.setText("Cancel");
        cancelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/GUI/Icons/cancel.png")));
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

        // Initial text for progress terminal & allignement to top
        progressLabel.setText("<html><body>Waiting for connection...<br>");
        progressLabel.setVerticalAlignment(javax.swing.SwingConstants.TOP);
        progressPane.setViewportView(progressLabel);

        // Set layout for the main form & dialog window
        org.jdesktop.layout.GroupLayout connectDialogLayout = new org.jdesktop.layout.GroupLayout(connectDialog.getContentPane());
        connectDialog.getContentPane().setLayout(connectDialogLayout);
        connectDialogLayout.setHorizontalGroup(
            connectDialogLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(connectDialogLayout.createSequentialGroup()
                .add(connectDialogLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(connectDialogLayout.createSequentialGroup()
                        .addContainerGap()
                        .add(connectDialogLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(serverAddressLabel)
                            .add(serverPortLabel)
                            .add(ownAddressLabel)
                            .add(ownPortLabel))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                        .add(connectDialogLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(ownAddressTextField, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 127, Short.MAX_VALUE)
                            .add(connectDialogLayout.createSequentialGroup()
                                .add(3, 3, 3)
                                .add(serverPortTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 50, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                            .add(ownPortTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 50, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(serverAddressTextField)))
                    .add(connectDialogLayout.createSequentialGroup()
                        .add(45, 45, 45)
                        .add(submitButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(cancelButton)))
                .addContainerGap(36, Short.MAX_VALUE))
        );
        connectDialogLayout.setVerticalGroup(
            connectDialogLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(connectDialogLayout.createSequentialGroup()
                .addContainerGap()
                .add(connectDialogLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(ownPortLabel)
                    .add(ownPortTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(connectDialogLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(ownAddressLabel)
                    .add(ownAddressTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(connectDialogLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(serverPortTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(serverPortLabel, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 16, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(connectDialogLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(serverAddressLabel)
                    .add(serverAddressTextField, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                .add(18, 18, 18)
                .add(connectDialogLayout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(submitButton, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                    .add(cancelButton))
                .addContainerGap(25, Short.MAX_VALUE))
        );
    
        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(layout.createSequentialGroup()
                        .add(6, 6, 6)
                        .add(raportLabel)
                        .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .add(layout.createSequentialGroup()
                        .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(jSeparator1)
                            .add(org.jdesktop.layout.GroupLayout.TRAILING, progressPane)
                            .add(layout.createSequentialGroup()
                                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                                    .add(layout.createSequentialGroup()
                                        .add(connectButton)
                                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                        .add(disconnectButton))
                                    .add(layout.createSequentialGroup()
                                        .add(saveFileButton)
                                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                                        .add(loadFileButton)))
                                .add(0, 61, Short.MAX_VALUE)))
                        .addContainerGap())))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(connectButton)
                    .add(disconnectButton))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                .add(jSeparator1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                .add(layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                    .add(loadFileButton)
                    .add(saveFileButton))
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                .add(raportLabel)
                .addPreferredGap(org.jdesktop.layout.LayoutStyle.UNRELATED)
                .add(progressPane, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 135, Short.MAX_VALUE)
                .addContainerGap())
        );

        pack();
    }

    
    

    // Coonect to network button click event handler
    private void connectButtonMouseClicked(java.awt.event.MouseEvent evt) {
        if (evt.getSource() == connectButton) {
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
        if (evt.getSource() == saveFileButton) {
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
        /* Set the Nimbus look and feel */
        /* If Nimbus (introduced in Java SE 6) is not available, stay with the default look and feel.
         */
        try {
            for (javax.swing.UIManager.LookAndFeelInfo info : javax.swing.UIManager.getInstalledLookAndFeels()) {
                if ("Nimbus".equals(info.getName())) {
                    javax.swing.UIManager.setLookAndFeel(info.getClassName());
                    break;
                }
            }
        } catch (ClassNotFoundException ex) {
            java.util.logging.Logger.getLogger(Client_GUI.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(Client_GUI.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(Client_GUI.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(Client_GUI.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        
        
        /* Create and display the form */
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
    private javax.swing.JSeparator jSeparator1;
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
