/**
 * [[[copyright]]]
 */

package tests.frontend;

import edu.cmu.sphinx.frontend.Feature;
import edu.cmu.sphinx.frontend.FeatureFrame;
import edu.cmu.sphinx.frontend.FrontEnd;
import edu.cmu.sphinx.frontend.Microphone;
import edu.cmu.sphinx.frontend.Utterance;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import java.io.IOException;

import java.util.List;
import java.util.Iterator;
import java.util.Vector;

import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.ListSelectionModel;


/**
 * Test program for the Microphone. Listens for audio data
 * on the system's audio capturing device, and does frontend
 * processing on it.
 * Running this class shows a little swing application that contains
 * a "Listen" and "Stop" button. 
 */
public class MicrophoneTest extends JFrame {

    private static String testName = "MicrophoneTest";
    private Microphone microphone;
    private FrontEnd frontend;
    private JPanel panel;
    private JTextArea featureTextArea;
    private JList featuresList;
    private DefaultListModel featuresModel;
    private AudioPlayer audioPlayer;


    /**
     * Main method to run this test.
     */
    public static void main(String[] argv) {
        MicrophoneTest microphoneTest = new MicrophoneTest();
        microphoneTest.show();
    }


    /**
     * Constructs a default MicrophoneTest.
     */
    public MicrophoneTest() {
        super(testName);
        setSize(new Dimension(500, 300));

        addWindowListener(new WindowAdapter() {
                public void windowClosing(WindowEvent e) {
                    System.exit(0);
                }
            });

        createModel();           // create the frontend components
        createPanel();           // create the GUI components
        getContentPane().add(panel, BorderLayout.CENTER);
        audioPlayer = new AudioPlayer();
    }


    /**
     * Creates the frontend components.
     */
    private void createModel() {

        microphone = new Microphone("Microphone", testName);

        // the microphone runs in a separate thread: it keeps
        // listening to and caching audio whenever it is "on"
        new Thread(microphone).start();
        
        frontend = new FrontEnd("FrontEnd", testName, microphone);
    }


    /**
     * Creates the main panel where all the swing GUI components are
     * placed.
     */
    private void createPanel() {

        panel = new JPanel(new BorderLayout());
        
        featuresModel = new DefaultListModel();
        featuresList = new JList(featuresModel);
        featuresList.setSelectionMode
            (ListSelectionModel.SINGLE_INTERVAL_SELECTION);
        JScrollPane featureScrollPane = new JScrollPane(featuresList);

        panel.add(createButtonPanel(), BorderLayout.NORTH);
        panel.add(featureScrollPane, BorderLayout.CENTER);
    }


    /**
     * Creates the top sub-Panel where the "Listen" and "Stop" buttons
     * are placed.
     */
    private JPanel createButtonPanel() {
        JPanel buttonPanel = new JPanel(new FlowLayout());

        // add the "Listen" button
        JButton startButton = new JButton("Listen");
        startButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                microphone.startRecording();
                (new FeatureConsumer()).start();
            }
        });

        // add the "Stop" button
        JButton stopButton = new JButton("Stop");
        stopButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                microphone.stopRecording();
            }
        });

        // add the "Play" button
        JButton playButton = new JButton("Play");
        playButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                int[] selected = featuresList.getSelectedIndices();
                if (selected.length > 0) {
                    int first = selected[0];
                    int last = selected[selected.length - 1];
                    Feature feature = (Feature) 
                        featuresModel.getElementAt(first);
                    Utterance utterance = (Utterance) feature.getUtterance();
                    if (utterance != null) {
                        byte[] audio = utterance.getAudio(first, last);
                        audioPlayer.play(audio);
                    } else {
                        System.out.println("No utterance");
                    }
                }
            }
        });

        // add the "Clear button
        JButton clearButton = new JButton("Clear");
        clearButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                featuresModel.removeAllElements();
            }
        });

        buttonPanel.add(startButton);
        buttonPanel.add(stopButton);
        buttonPanel.add(playButton);
        buttonPanel.add(clearButton);

        return buttonPanel;
    }
    

    /**
     * A FeatureConsumer thread that gets FeatureFrames from the
     * Frontend and simply prints them out to the featuresList.
     */
    class FeatureConsumer extends Thread {

        public void run() {

            int featureBlockSize = 25;
            int featureCount = 0;
            FeatureFrame featureFrame = null;

            do {
                try {
                    featureFrame = frontend.getFeatureFrame(featureBlockSize);
                } catch (IOException ioe) {
                    ioe.printStackTrace();
                }
                if (featureFrame != null) {
                    outputFeatureFrame(featureFrame.getFeatures());
                } else {
                    break;
                }
            } while (featureFrame.getFeatures().length == 
                     featureBlockSize);
            
            invalidate();
        }


        /**
         * Appends the given features to the featuresModel.
         *
         * @param features the features to append
         */
        private void outputFeatureFrame(Feature[] features) {
            synchronized (featuresModel) {
                for (int i = 0; i < features.length; i++) {
                    if (features[i] != null) {
                        featuresModel.addElement(features[i]);
                    }
                }
            }
        }
    }
}
