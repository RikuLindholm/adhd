package gui;

import javax.swing.Icon;
import javax.swing.ImageIcon;

public class IconButton extends javax.swing.JButton {

  public IconButton(String labelText, String iconUrl) {
    this.setText(labelText);
    if (iconUrl != null)
      this.setIcon(new javax.swing.ImageIcon(iconUrl));
  }

}
