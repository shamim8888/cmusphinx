/* ====================================================================
 * Copyright (c) 1997-2000 Carnegie Mellon University.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*********************************************************************
 *
 * File: agg_all_seg.c
 * 
 * Description: 
 * 
 * Author: 
 * 
 *********************************************************************/

#include "agg_all_seg.h"

#include <s3/corpus.h>
#include <s3/s3io.h>
#include <s3/feat.h>
#include <s3/ck_seg.h>
#include <s3/mk_phone_seq.h>
#include <s3/ckd_alloc.h>
#include <s3/err.h>
#include <s3/s3.h>
#include <s3/cmd_ln.h>

#include <sys_compat/misc.h>

#include <stdio.h>

static FILE *
open_dmp(const char *fn)
{
    FILE *fp;

    s3clr_fattr();
    fp = s3open(fn, "wb", NULL);
    if (fp == NULL) {
	E_ERROR_SYSTEM("Unable to open %s for writing.", fn);
    }

    return fp;
}

int
agg_all_seg(segdmp_type_t type,
	    const char *fn,
	    uint32 stride)
{
    uint32 seq_no;
    vector_t *mfcc = NULL;
    uint32 mfc_veclen = cmd_ln_int32("-ceplen");
    uint32 n_frame;
    uint32 n_out_frame;
    uint32 blksz=0;
    vector_t **feat = NULL;
    uint32 i, j;
    uint32 t;
    int32 sv_feat = FALSE;
    int32 sv_mfcc = FALSE;
    int32 sv_vq = FALSE;
    uint32 n_stream;
    const uint32 *veclen;
    FILE *fp;
    uint32 ignore = 0;
    long start;
    int32 no_retries=0;
    
    if (type == SEGDMP_TYPE_FEAT) {
	sv_feat = TRUE;
	n_stream = feat_n_stream();
	veclen = feat_vecsize();
	for (i = 0, blksz = 0; i < n_stream; i++)
	    blksz += veclen[i];
    }
    else if (type == SEGDMP_TYPE_MFCC) {
	sv_mfcc = TRUE;
    }
    else if (type == SEGDMP_TYPE_VQ) {
	sv_vq = TRUE;
    }

    if (sv_vq) {
	E_FATAL("VQ aggregation of states not supported\n");
    }

    fp = open_dmp(fn);

    start = ftell(fp);

    if (s3write(&i, sizeof(uint32), 1, fp, &ignore) != 1) {
	E_ERROR_SYSTEM("Unable to write to dmp file");

	return S3_ERROR;
    }

    for (seq_no = corpus_get_begin(), j = 0, n_out_frame = 0;
	 corpus_next_utt(); seq_no++) {
	if (sv_feat || sv_mfcc) {
	    if (mfcc) {
		free(mfcc[0]);
		ckd_free(mfcc);

		mfcc = NULL;
	    }

	    /* get the MFCC data for the utterance */
	    if (corpus_get_generic_featurevec(&mfcc, &n_frame, mfc_veclen) < 0) {
	      E_FATAL("Can't read input features from %s\n", corpus_utt());
	    }
	}

	if ((seq_no % 1000) == 0) {
	    E_INFO("[%u]\n", seq_no);
	}

	if (sv_feat) {
	    if (feat) {
		feat_free(feat);
		feat = NULL;
	    }
	    
	    if (n_frame < 9) {
	      E_WARN("utt %s too short\n", corpus_utt());
	      if (mfcc) {
		ckd_free(mfcc[0]);
		ckd_free(mfcc);
		mfcc = NULL;
	      }
	      continue;
	    }

	    feat = feat_compute(mfcc, &n_frame);

	    for (t = 0; t < n_frame; t++, j++) {
		if ((j % stride) == 0) {
		    while (s3write(&feat[t][0][0],
				   sizeof(float32),
				   blksz,
				   fp, &ignore) != blksz) {
			static int rpt = 0;

			if (!rpt) {
			    E_ERROR_SYSTEM("Unable to write to dmp file");
			    E_INFO("sleeping...\n");
			    no_retries++;
			}
			sleep(3);

			if(no_retries > 10){
			  E_FATAL("Failed to write to a dmp file after 10 retries of getting MFCC(about 30 seconds)\n ");
			}
		    }
		    ++n_out_frame;
		}
	    }
	}
	else if (sv_mfcc) {
	    E_FATAL("Dumping MFCC not yet supported\n");
	}	    
    }

    if (fseek(fp, start, SEEK_SET) < 0) {
	E_ERROR_SYSTEM("Unable to seek to begin of dmp");

	return S3_ERROR;
    }

    E_INFO("Wrote %u frames to %s\n", n_out_frame, fn);

    if (s3write((void *)&n_out_frame, sizeof(uint32), 1, fp, &ignore) != 1) {
	E_ERROR_SYSTEM("Unable to write to dmp file");
	
	return S3_ERROR;
    }

    return S3_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.6  2005/09/27  02:02:47  arthchan2003
 * Check whether utterance is too short in init_gau, bw and agg_seg.
 * 
 * Revision 1.5  2004/11/17 01:46:58  arthchan2003
 * Change the sleeping time to be at most 30 seconds. No one will know whether the code dies or not if keep the code loop infinitely.
 *
 * Revision 1.4  2004/07/21 18:30:32  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 *
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/07/16  11:36:22  eht
 * Initial revision
 * 
 *
 */
