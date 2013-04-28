package gui;

import gui.IconButton;
import java.awt.event.*;

public class ConnectDialog extends javax.swing.JDialog {

  public IconButton submitButton;
  public IconButton cancelButton;
  private javax.swing.JLabel serverAddressLabel;
  private javax.swing.JTextField serverAddressTextField;
  private javax.swing.JLabel serverPortLabel;
  private javax.swing.JTextField serverPortTextField;
  private javax.swing.JLabel ownAddressLabel;
  private javax.swing.JTextField ownAddressTextField;
  private javax.swing.JLabel ownPortLabel;
  private javax.swing.JTextField ownPortTextField;

  public ConnectDialog() {
    ownPortLabel = new javax.swing.JLabel();
    ownAddressLabel = new javax.swing.JLabel();
    serverPortLabel = new javax.swing.JLabel();
    serverAddressLabel = new javax.swing.JLabel();
    ownPortTextField = new javax.swing.JTextField();
    ownAddressTextField = new javax.swing.JTextField();
    serverPortTextField = new javax.swing.JTextField();
    serverAddressTextField = new javax.swing.JTextField();
    submitButton = new IconButton("Submit", null); // "icons/submit.png"
    cancelButton = new IconButton("Cancel", null); //"icons/cancel.png"
    this.render();
  }

  public void render() {
    this.setLayout(null);

    // Text labels
    ownPortLabel.setBounds(10, 10, 100, 20);
    this.add(ownPortLabel);
    
    ownAddressLabel.setBounds(10, 30, 100, 20);
    this.add(ownAddressLabel);
    
    serverPortLabel.setBounds(10, 50, 100, 20);
    this.add(serverPortLabel);
    
    serverAddressLabel.setBounds(10, 70, 100, 20);
    this.add(serverAddressLabel);
    
    // Text areas
    ownPortTextField.setBounds(110, 10, 100, 20);
    this.add(ownPortTextField);
    
    ownAddressTextField.setBounds(110, 30, 100, 20);
    this.add(ownAddressTextField);
    
    serverPortTextField.setBounds(110, 50, 100, 20);
    this.add(serverPortTextField);
    
    serverAddressTextField.setBounds(110, 70, 100, 20);
    this.add(serverAddressTextField);
    
    // Submit & cancel buttons
    submitButton.setBounds(10, 100, 100, 40);
    this.add(submitButton);
    
    cancelButton.setBounds(110, 100, 100, 40);
    this.add(cancelButton);

    // Set connection dialogue to center of screen, modal and unsizable 
    this.setMinimumSize(new java.awt.Dimension(250, 220));
    this.setModal(true);
    this.setResizable(false);

    // Set texts within the connection dialogue
    ownPortLabel.setText("Own port:");
    ownAddressLabel.setText("Own address:");
    serverPortLabel.setText("Server port:");
    serverAddressLabel.setText("Server address:");
  }

  public void showDialog() {
    this.setLocationRelativeTo(null);
    this.setVisible(true);
  }

  public void hideDialog() {
    this.setVisible(false);
  }

  public String[] getValues() {
    String[] values = {
      ownPortTextField.getText(),
      ownAddressTextField.getText(),
      serverPortTextField.getText(),
      serverAddressTextField.getText()
    };
    return values;
  }

  public boolean dataIsValid() {
    String[] values = getValues();
    String portPattern = "\\b\\d{1,5}\\b";
    String ipPattern = "\\b(?:\\d{1,3}\\.){3}\\d{1,3}\\b";
    if (values[0].matches(portPattern) &&
        values[1].matches(ipPattern) &&
        values[2].matches(portPattern) &&
        values[3].matches(ipPattern))
      return true;
    else
      return false;
  }
}