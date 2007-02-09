/**
 * Copyright 1999-2007 Carnegie Mellon University.
 * Portions Copyright 2002 Sun Microsystems, Inc.
 * All Rights Reserved.  Use is subject to license terms.
 * <p/>
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 * <p/>
 * <p/>
 * User: Garrett Weinberg
 * Date: Feb 8, 2007
 * Time: 11:14:38 PM
 */

package edu.cmu.sphinx.tools.riddler.types.audio;

/**
 * document me!
 */
public class ShortAudioDescriptor extends AudioDescriptor {

    short[] data;

    public ShortAudioDescriptor(int samplesPerSecond, int channelCount, short[] data) {
        super(samplesPerSecond, channelCount);
        this.data = data;
    }

    public short[] getData() {
        return data;
    }

    public void setData(short[] data) {
        this.data = data;
    }
}
