package edu.cmu.sphinx.tools.corpusBrowser;

import edu.cmu.sphinx.tools.audio.Player;
import edu.cmu.sphinx.tools.corpus.Corpus;
import edu.cmu.sphinx.tools.corpus.Word;
import edu.cmu.sphinx.tools.corpus.xml.CorpusXMLReader;
import edu.cmu.sphinx.util.props.ConfigurationManager;
import edu.cmu.sphinx.util.props.PropertyException;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.List;

/**
 * Copyright 1999-2006 Carnegie Mellon University.
 * Portions Copyright 2002 Sun Microsystems, Inc.
 * Portions Copyright 2002 Mitsubishi Electric Research Laboratories.
 * All Rights Reserved.  Use is subject to license terms.
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * User: Peter Wolf
 * Date: Jan 25, 2006
 * Time: 3:02:10 PM
 */
public class WordBrowser {
    WordSpectrogramPanel spectrogram;
    private JCheckBox excluded;
    private JButton play;
    JPanel mainPane;
    JSlider zoom;
    Player player;
    Word word;

    WordBrowser( ConfigurationManager cm, Word w ) {
        this.word = w;
        player = new Player(word.getRegionOfAudioData());

        zoom.addChangeListener(new ChangeListener() {
            public void stateChanged(ChangeEvent event) {
                JSlider slider = (JSlider) event.getSource();
                int max = slider.getMaximum();
                int min = slider.getMinimum();
                int val = slider.getValue();
                //double z = Math.pow(100.0,((double)val)/(max-min));
                //w.spectrogram.setZoom(z);
                spectrogram.setOffset(((double) val) / (max - min));
            }
        });

        play.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent event) {
                if( player.isPlaying() ) {
                    player.stop();
                }
                else {
                    player.start();
                }
            }
        });

        spectrogram.setWord(cm, word);
    }


    public static void main(String[] args) {

        if (args.length != 3) {
            System.out.println(
                    "Usage: WordBrowser propertiesFile corpusFile word");
            System.exit(1);
        }

        String propertiesFile = args[0];
        String corpusFile = args[1];
        String spelling = args[2];

        try {
            URL url = new File(propertiesFile).toURI().toURL();
            ConfigurationManager cm = new ConfigurationManager(url);

            Corpus corpus = new CorpusXMLReader( new FileInputStream(corpusFile)).read();

            List<Word> words = corpus.getWords(spelling);

            Word word = words.get(0);

            final WordBrowser w = new WordBrowser(cm,word);
            JFrame f = new JFrame("WordBrowser");

            f.setContentPane(w.mainPane);
            f.pack();
            f.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);

            f.setVisible(true);


        } catch (MalformedURLException e) {
            throw new Error(e);
        } catch (PropertyException e) {
            throw new Error(e);
        } catch (IOException e) {
            throw new Error(e);
        }
    }


}
