import javax.swing.*;
import java.awt.event.MouseEvent;
import java.io.File;

public class SaveAsDialog extends javax.swing.JDialog {

    private JLabel fileNameLabel;
    private JTextField fileNameField;
    private JLabel targetPathLabel;
    private JTextField targetPathField;
    private IconButton browseButton;
    public IconButton fetchButton;
    public IconButton cancelButton;
    private String targetPath = null;
    private String fileName = null;
    private String[] values = {null, null};

    public SaveAsDialog() {
        fileNameLabel = new JLabel("File name to fetch:");
        fileNameField = new JTextField();
        targetPathLabel = new JLabel("Save as:");
        targetPathField = new JTextField();
        browseButton = new IconButton("Browse...", null);
        fetchButton = new IconButton("Fetch", null);
        cancelButton = new IconButton("Cancel", null);
        bindEvents();
        render();
    }

    public void render() {
        setLayout(null);
        setTitle("Load file from DHT");

        fileNameLabel.setBounds(10, 10, 230, 30);
        fileNameField.setBounds(10, 40, 230, 30);
        targetPathLabel.setBounds(10, 75, 230, 30);
        targetPathField.setBounds(10, 105, 140, 30);
        browseButton.setBounds(150, 105, 90, 30);
        fetchButton.setBounds(20, 150, 110, 30);
        cancelButton.setBounds(130, 150, 110, 30);

        add(fileNameLabel);
        add(fileNameField);
        add(targetPathLabel);
        add(targetPathField);
        add(browseButton);
        add(fetchButton);
        add(cancelButton);

        setMinimumSize(new java.awt.Dimension(250, 220));
        setModal(true);
        setResizable(false);
        repaint();
    }

    private void bindEvents() {
        browseButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
              showTargetDialog(evt);
            }
        });
        fetchButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                setValues();
                closeDialog(evt);
            }
        });
        cancelButton.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                clearValues();
                closeDialog(evt);
            }
        });
    }

    public String[] showDialog() {
        setVisible(true);
        return values;
    }

    public void showTargetDialog(MouseEvent e) {
        final JFileChooser fc = new JFileChooser();
        int retval = fc.showSaveDialog(null);
        if (retval == JFileChooser.APPROVE_OPTION) {
            targetPathField.setText(fc.getSelectedFile().getAbsolutePath());
        }
    }

    public void closeDialog(MouseEvent e) {
        setVisible(false);
        dispose();
    }

    private void clearValues() {
        values[0] = null;
        values[1] = null;
    }

    private void setValues() {
        values[0] = getFileName();
        values[1] = getTargetPath();
    }

    private String getTargetPath() {
        return exists(targetPathField.getText());
    }

    private String getFileName() {
        return exists(fileNameField.getText());
    }

    private String exists(String text) {
        if (text != null && text.length() > 0)
            return text;
        else
            return null;
    }
}