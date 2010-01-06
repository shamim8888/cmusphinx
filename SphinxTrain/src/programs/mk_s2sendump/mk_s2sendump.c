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
 * File: mk_s2sendump.c
 * 
 * Description: 
 *    Make sendump file in converting S3 models to S2 models
 *
 * Author: 
 *    Alan W Black (awb@cs,cmu.edu)
 * Picked togeterh out off rkm's sen2s2.c
 *********************************************************************/

#include "parse_cmd_ln.h"

/* The SPHINX-III common library */
#include <s3/common.h>

#include <s3/model_inventory.h>
#include <s3/model_def_io.h>
#include <s3/s3mixw_io.h>
#include <s3/s3tmat_io.h>

/* Some SPHINX-II compatibility definitions */
#include <s3/s2_param.h>
#include <s3/s2_read_map.h>
#include <s3/s2_write_seno.h>
#include <s2/byteorder.h>

#include <s3/cmd_ln.h>

#include "s3/hash.h"
typedef hash_t hash_table_t;
#include "s3types.h"
#include "another_s3types.h"
#include "another_senone.h"
#include "bio.h"
#include "logs3.h"
#include "feat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

#define NO_STATE	0xffffffff

#define MIXW_PARAM_VERSION	"1.0"
#define SPDEF_PARAM_VERSION	"1.2"

static char *fmtdesc[] = {
    "BEGIN FILE FORMAT DESCRIPTION",
    "(int32) <length(string)> (including trailing 0)",
    "<string> (including trailing 0)",
    "... preceding 2 items repeated any number of times",
    "(int32) 0 (length(string)=0 terminates the header)",
    "(int32) <#codewords>",
    "(int32) <#pdfs>",
    "256 (int32) cluster-prob values for codebook-0 codeword-0",
    "#pdf (unsigned char) cluster-prob ids for codebook-0 codeword-0",
    "... preceding 2 items repeated for all codewords in codebook-0",
    "preceding 3 items repeated for codebooks 1, 2, 3.",
    "END FILE FORMAT DESCRIPTION",
    NULL,
};

#if 0
#if defined(_HPUX_SOURCE)
#define SWAPW(x)	x = ( (((x)<<8)&0x0000ff00) | (((x)>>8)&0x00ff) )
#define SWAPL(x)	x = ( (((x)<<24)&0xff000000) | (((x)<<8)&0x00ff0000) | \
    			      (((x)>>8)&0x0000ff00) | (((x)>>24)&0x000000ff) )
#else
#define SWAPW(x)
#define SWAPL(x)
#endif
#endif

/* For Pocketsphinx ONLY: Allocate 0..159 for negated quantized
 * mixture weights and 0..96 for negated normalized acoustic scores,
 * so that the combination of the two (for a single mixture) can never
 * exceed 255. */
#define MAX_NEG_MIXW 159 /**< Maximum negated mixture weight value. */
#define MAX_NEG_ASCR 96  /**< Maximum negated acoustic score value. */


float64 vector_sum_norm (float32 *vec, int32 len)
{
    float64 sum, f;
    int32 i;
    
    sum = 0.0;
    for (i = 0; i < len; i++)
	sum += vec[i];

    if (sum != 0.0) {
	f = 1.0 / sum;
	for (i = 0; i < len; i++)
	    vec[i] *= f;
    }
    
    return sum;
}

static void fwrite_int32 (fp, val)
    FILE *fp;
    int val;
{
    SWAP_L(val);
    fwrite (&val, sizeof(int), 1, fp);
}


static void senone_dump (const model_def_t *mdef, const senone_t *s, char *file)
{
    int32 i, j, k, c, m, f, n, p, sb, se;
    mixw_t *fw;
    FILE *fpout;
    int32 lut[256];
    senprob_t *sp;
    int n_emit_state,d,d2,pmax,lpmax;
    
    E_INFO("Writing S2 format sendump file: %s\n", file);
    if ((fpout = fopen(file, "wb")) == NULL)
	E_FATAL("fopen(%s,wb) failed\n", file);
    
    /* Write format description into header */
    for (i = 0; fmtdesc[i] != NULL; i++) {
	n = strlen(fmtdesc[i])+1;
	fwrite_int32 (fpout, n);
	fwrite (fmtdesc[i], sizeof(char), n, fpout);
    }

    /* Terminate header */
    fwrite_int32 (fpout, 0);

    /* Write #codewords, #pdfs */
    if (s->n_mgau != 1)
	E_FATAL("#codebooks(%d) != 1\n", s->n_mgau);
    if (s->mgau2sen[0].n_sen != s->n_sen)
	E_FATAL("#senones for mgau[0](%d) != total #senones(%d)\n",
		s->mgau2sen[0].n_sen, s->n_sen);
    if (s->n_feat != 4)
	E_FATAL("#Features(%d) != 4\n", s->n_feat);
    for (i = 0; i < s->n_feat; i++) {
	if (s->mgau2sen[0].feat_mixw[i].n_wt != s->mgau2sen[0].feat_mixw[0].n_wt)
	    E_FATAL("#Wts not same for all features\n");
    }
    if (s->mgau2sen[0].feat_mixw[0].n_wt != 256)
	E_FATAL("#Wts(%d) != 256\n", s->mgau2sen[0].feat_mixw[0].n_wt);
    
    fwrite_int32 (fpout, s->mgau2sen[0].feat_mixw[0].n_wt);
    fwrite_int32 (fpout, s->n_sen);

    /* Create lut */
    for (i = 0; i < 256; i++)
	lut[i] = -(i << s->shift);
    
    sp = (senprob_t *) ckd_calloc (s->n_sen, sizeof(senprob_t));

    /* Write PDFs (#feat x #wt x #sen) */
    if (mdef->max_n_state - 1 != 5)

	E_FATAL("#States(%d) != 5\n", mdef->max_n_state - 1);
    n_emit_state = mdef->max_n_state - 1;
    for (f = 0; f < s->n_feat; f++) {
	fw = s->mgau2sen[0].feat_mixw;

	for (c = 0; c < fw[f].n_wt; c++) {
	    /*
	     * In S3, all CI-senones (for all CI-phones) come first.  CD-senones later.
	     * But in S2, for each CI-phone, CD-senones come first and then CI-senones.
	     */
	    k = 0, d=mdef->acmod_set->n_ci;
	    lpmax = mdef->acmod_set->n_ci * n_emit_state - 1;
	    for (p = 0; p < mdef->acmod_set->n_ci; p++) {
		/* CD senones first; find start and end points in S3 data */
		for (pmax = lpmax ; mdef->defn[d].tmat == p; d++)
		{
		    for (d2=0; d2 < n_emit_state; d2++)
			if (mdef->defn[d].state[d2] > pmax)
			    pmax = mdef->defn[d].state[d2];
		}
		sb = lpmax + 1;
		se = pmax;
		lpmax = pmax;
		
		for (i = sb; i <= se; i++) {
		    m = s->sen2mgau[i];
		    assert (m == 0);
		    j = s->mgau2sen_idx[i];
		    assert (j == i);
		    
		    sp[k++] = fw[f].prob[j][c];
		}

		/* CI senones next */
		sb = p * n_emit_state;
		se = sb + n_emit_state - 1;
		
		for (i = sb; i <= se; i++) {
		    m = s->sen2mgau[i];
		    assert (m == 0);
		    j = s->mgau2sen_idx[i];
		    assert (j == i);
		    
		    sp[k++] = fw[f].prob[j][c];
		}
	    }
	    assert (k == mdef->n_tied_state);
	    
	    /* Write lut for feat f, codeword c */
	    for (i = 0; i < 256; i++)
		fwrite_int32 (fpout, lut[i]);

	    /* Write data for feat f, codeword c */
	    fwrite (sp, sizeof(uint8), s->n_sen, fpout);
	}
    }

    fclose (fpout);
}

static void pocketsphinx_senone_dump(const model_def_t *mdef,
				     const senone_t *s, char *file)
{
    FILE *fpout;
    char pshdr[256];
    int32 i, n, k, c, d, f;
    
    E_INFO("Writing PocketSphinx format sendump file: %s\n", file);
    if ((fpout = fopen(file, "wb")) == NULL)
	E_FATAL("fopen(%s,wb) failed\n", file);
    
    /* Write format description into header */
    for (i = 0; fmtdesc[i] != NULL; i++) {
	n = strlen(fmtdesc[i])+1;
	fwrite_int32 (fpout, n);
	fwrite (fmtdesc[i], sizeof(char), n, fpout);
    }
    /* Now write the cluster, feature and codebook counts */
    sprintf(pshdr, "cluster_count %d", 0);
    fwrite_int32(fpout, strlen(pshdr)+1);
    fwrite(pshdr, sizeof(char), strlen(pshdr)+1, fpout);
    sprintf(pshdr, "codebook_count %d", s->n_mgau);
    fwrite_int32(fpout, strlen(pshdr)+1);
    fwrite(pshdr, sizeof(char), strlen(pshdr)+1, fpout);
    sprintf(pshdr, "feature_count %d", s->n_feat);
    fwrite_int32(fpout, strlen(pshdr)+1);
    fwrite(pshdr, sizeof(char), strlen(pshdr)+1, fpout);

    /* Pad the header for alignment purposes */
    k = ftell(fpout) & 3;
    if (k > 0) {
        k = 4 - k;
        fwrite_int32(fpout, k);
        fwrite("!!!!", 1, k, fpout);
    }

    /* Terminate header */
    fwrite_int32(fpout, 0);

    /* For each codebook, write #codewords, #pdfs */
    for (c = 0; c < s->n_mgau; ++c) {
	/* Write #densities, #senones (indicates that they are transposed) */
	fwrite_int32 (fpout, s->mgau2sen[c].feat_mixw[0].n_wt);
	fwrite_int32 (fpout, s->mgau2sen[c].n_sen);

	/* Now write out transposed, quantized senones. */
	/* Note!  PocketSphinx puts them in Sphinx3 order. */
	for (f = 0; f < s->n_feat; f++) {
	    for (d = 0; d < s->mgau2sen[c].feat_mixw[f].n_wt; ++d) {
		for (i = 0; i < s->mgau2sen[c].n_sen; ++i) {
		    fputc(s->mgau2sen[c].feat_mixw[f].prob[i][d], fpout);
		}
	    }
	}
    }

    fclose (fpout);
}

static int32 senone_mgau_map_read (senone_t *s, char *file_name)
{
    FILE *fp;
    int32 byteswap, chksum_present, n_mgau_present;
    uint32 chksum;
    int32 i;
    char eofchk;
    char **argname, **argval;
    float32 v;
    
    E_INFO("Reading senone-codebook map file: %s\n", file_name);
    
    if ((fp = fopen(file_name, "rb")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file_name);
    
    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr (fp, &argname, &argval, &byteswap) < 0)
	E_FATAL("bio_readhdr(%s) failed\n", file_name);
    
    /* Parse argument-value list */
    chksum_present = 0;
    n_mgau_present = 0;
    for (i = 0; argname[i]; i++) {
	if (strcmp (argname[i], "version") == 0) {
	    if (strcmp(argval[i], SPDEF_PARAM_VERSION) != 0) {
		E_WARN("Version mismatch(%s): %s, expecting %s\n",
		       file_name, argval[i], SPDEF_PARAM_VERSION);
	    }
	    
	    /* HACK!! Convert version# to float32 and take appropriate action */
	    if (sscanf (argval[i], "%f", &v) != 1)
		E_FATAL("%s: Bad version no. string: %s\n", file_name, argval[i]);

	    n_mgau_present = (v > 1.1) ? 1 : 0;
	} else if (strcmp (argname[i], "chksum0") == 0) {
	    chksum_present = 1;	/* Ignore the associated value */
	}
    }
    bio_hdrarg_free (argname, argval);
    argname = argval = NULL;

    chksum = 0;
    
    /* Read #gauden (if version matches) */
    if (n_mgau_present) {
	if (bio_fread (&(s->n_mgau), sizeof(int32), 1, fp, byteswap, &chksum) != 1)
	    E_FATAL("fread(%s) (#gauden) failed\n", file_name);
    }
    
    /* Read 1d array data; s->sen2mgau allocated by called function */
    if (bio_fread_1d ((void **)(&s->sen2mgau), sizeof(int32), &(s->n_sen), fp,
		      byteswap, &chksum) < 0) {
	E_FATAL("bio_fread_1d(%s) failed\n", file_name);
    }
    
    /* Infer n_mgau if not present in this version */
    if (! n_mgau_present) {
	s->n_mgau = 1;
	for (i = 0; i < s->n_sen; i++) {
	    if (s->sen2mgau[i] >= s->n_mgau)
		s->n_mgau = s->sen2mgau[i]+1;
	}
    }
    
    if (s->n_sen >= MAX_SENID)
	E_FATAL("%s: #senones (%d) exceeds limit (%d)\n", file_name, s->n_sen, MAX_SENID);
    if (s->n_mgau >= MAX_MGAUID)
	E_FATAL("%s: #gauden (%d) exceeds limit (%d)\n", file_name, s->n_mgau, MAX_MGAUID);

    /* Check for validity of mappings */
    for (i = 0; i < s->n_sen; i++) {
	if ((s->sen2mgau[i] >= s->n_mgau) || NOT_MGAUID(s->sen2mgau[i]))
	    E_FATAL("Bad sen2mgau[%d]= %d, out of range [0, %d)\n",
		    i, s->sen2mgau[i], s->n_mgau);
    }
    
    if (chksum_present)
	bio_verify_chksum (fp, byteswap, chksum);
    
    if (fread (&eofchk, 1, 1, fp) == 1)
	E_FATAL("More data than expected in %s\n", file_name);

    fclose(fp);

    E_INFO("Read %d->%d senone-codebook mappings\n", s->n_sen, s->n_mgau);

    return 0;
}


/* In the old S3 files, all senones have the same "shape" (#codewords/senone/feat) */
static void build_mgau2sen (senone_t *s, int32 n_cw)
{
    int32 i, j, m, f;
    s3senid_t *sen;
    mixw_t *fw;
    
    /* Create mgau2sen map from sen2mgau */
    s->mgau2sen = (mgau2sen_t *) ckd_calloc (s->n_mgau, sizeof(mgau2sen_t));
    s->mgau2sen_idx = (int32 *) ckd_calloc (s->n_sen, sizeof(int32));
    for (i = 0; i < s->n_sen; i++) {
	m = s->sen2mgau[i];
	assert ((m < s->n_mgau) && (m >= 0));
	(s->mgau2sen[m].n_sen)++;
    }
    
    sen = (s3senid_t *) ckd_calloc (s->n_sen, sizeof(s3senid_t));
    for (m = 0; m < s->n_mgau; m++) {
	s->mgau2sen[m].sen = sen;
	sen += s->mgau2sen[m].n_sen;
	s->mgau2sen[m].n_sen = 0;
    }

    for (i = 0; i < s->n_sen; i++) {
	m = s->sen2mgau[i];
	j = s->mgau2sen[m].n_sen;
	s->mgau2sen[m].sen[j] = i;
	s->mgau2sen_idx[i] = j;
	(s->mgau2sen[m].n_sen)++;
    }
    
    /* Allocate space for the weights */
    for (m = 0; m < s->n_mgau; m++) {
	fw = (mixw_t *) ckd_calloc (s->n_feat, sizeof(mixw_t));
	s->mgau2sen[m].feat_mixw = fw;

	for (f = 0; f < s->n_feat; f++) {
	    fw[f].n_wt = n_cw;
	    fw[f].prob = (senprob_t **) ckd_calloc_2d (s->mgau2sen[m].n_sen, n_cw,
						       sizeof(senprob_t));
	}
    }
}


/* In the old S3 files, all senones have the same "shape" (#codewords/senone/feat) */
static int32 senone_mixw_read(senone_t *s, char *file_name, float64 mixwfloor)
{
    FILE *fp;
    char **argname, **argval;
    int32 byteswap, chksum_present;
    uint32 chksum;
    float32 *pdf;
    int32 i, j, f, m, c, p, n_sen, n_err, n_cw, nval;
    char eofchk;
    mixw_t *fw;
    int32 pocketsphinx = cmd_ln_boolean("-pocketsphinx");
    
    E_INFO("Reading senone mixture weights: %s\n", file_name);
    
    if ((fp = fopen(file_name, "rb")) == NULL)
	E_FATAL_SYSTEM("fopen(%s,rb) failed\n", file_name);
    
    /* Read header, including argument-value info and 32-bit byteorder magic */
    if (bio_readhdr (fp, &argname, &argval, &byteswap) < 0)
	E_FATAL("bio_readhdr(%s) failed\n", file_name);
    
    /* Parse argument-value list */
    chksum_present = 0;
    for (i = 0; argname[i]; i++) {
	if (strcmp (argname[i], "version") == 0) {
	    if (strcmp(argval[i], MIXW_PARAM_VERSION) != 0)
		E_WARN("Version mismatch(%s): %s, expecting %s\n",
			file_name, argval[i], MIXW_PARAM_VERSION);
	} else if (strcmp (argname[i], "chksum0") == 0) {
	    chksum_present = 1;	/* Ignore the associated value */
	}
    }
    bio_hdrarg_free (argname, argval);
    argname = argval = NULL;

    chksum = 0;

    /* Read #senones, #features, #codewords, arraysize */
    n_sen = s->n_sen;
    if ((bio_fread (&(s->n_sen),  sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&(s->n_feat), sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&(n_cw),   sizeof(int32), 1, fp, byteswap, &chksum) != 1) ||
	(bio_fread (&nval,   sizeof(int32), 1, fp, byteswap, &chksum) != 1)) {
	E_FATAL("bio_fread(%s) (arraysize) failed\n", file_name);
    }
    if ((n_sen != 0) && (s->n_sen != n_sen))
	E_FATAL("#senones(%d) conflict with mapping file(%d)\n", s->n_sen, n_sen);
    if (s->n_sen >= MAX_SENID)
	E_FATAL("%s: #senones (%d) exceeds limit (%d)\n", file_name, s->n_sen, MAX_SENID);
    if (s->n_feat <= 0)
	E_FATAL("Bad #features: %d\n", s->n_feat);
    if (n_cw <= 0)
	E_FATAL("Bad #mixing-wts/senone: %d\n", n_cw);
    
    /* Allocate sen2mgau map if not yet done so (i.e. no explicit mapping file given */
    if (! s->sen2mgau) {
	assert ((s->n_mgau == 0) || (s->n_mgau == 1));
	
	s->sen2mgau = (int32 *) ckd_calloc (s->n_sen, sizeof(int32));
	
	if (s->n_mgau == 1) {
	    /* Semicontinuous mode; all senones map to single, shared gaussian: 0 */
	    for (i = 0; i < s->n_sen; i++)
		s->sen2mgau[i] = 0;
	} else {
	    /* Fully continuous mode; each senone maps to own parent gaussian */
	    s->n_mgau = s->n_sen;
	    for (i = 0; i < s->n_sen; i++)
		s->sen2mgau[i] = i;
	}
    } else
	assert (s->n_mgau != 0);
    if (s->n_mgau >= MAX_MGAUID)
	E_FATAL("%s: #gauden (%d) exceeds limit (%d)\n", file_name, s->n_mgau, MAX_MGAUID);
    
    if (nval != s->n_sen * s->n_feat * n_cw) {
	E_FATAL("%s: #float32 values(%d) doesn't match dimensions: %d x %d x %d\n",
		file_name, nval, s->n_sen, s->n_feat, n_cw);
    }
    
    /*
     * Compute #LSB bits to be dropped to represent mixwfloor with 8 bits.
     * All PDF values will be truncated (in the LSB positions) by these many bits.
     */
    if ((mixwfloor <= 0.0) || (mixwfloor >= 1.0))
	E_FATAL("mixwfloor (%e) not in range (0, 1)\n", mixwfloor);
    p = logs3(mixwfloor);
    for (s->shift = 0, p = -p; p >= 256; s->shift++, p >>= 1);
    if (pocketsphinx) /* PocketSphinx uses a fixed 10-bit shift. */
	s->shift = 10;
    E_INFO("Truncating senone logs3(wt) values by %d bits, to 8 bits\n", s->shift);

    /* Allocate memory for s->mgau2sen and senone PDF data */
    build_mgau2sen (s, n_cw);
    
    /* Temporary structure to read in floats */
    pdf = (float32 *) ckd_calloc (n_cw, sizeof(float32));

    /* Read senone probs data, normalize, floor, convert to logs3, truncate to 8 bits */
    n_err = 0;
    for (i = 0; i < s->n_sen; i++) {
	m = s->sen2mgau[i];	/* Parent mgau */
	j = s->mgau2sen_idx[i];	/* Index of senone i within list of senones for mgau m */
	fw = s->mgau2sen[m].feat_mixw;

	for (f = 0; f < s->n_feat; f++) {
	    if (bio_fread((void *)pdf, sizeof(float32), n_cw, fp, byteswap, &chksum)
		!= n_cw) {
		E_FATAL("bio_fread(%s) (arraydata) failed\n", file_name);
	    }
	    
	    /* Normalize and floor */
	    if (vector_sum_norm (pdf, n_cw) == 0.0)
		n_err++;
	    vector_floor (pdf, n_cw, mixwfloor);
	    vector_sum_norm (pdf, n_cw);

	    /* Convert to logs3, truncate to 8 bits, and store in s->pdf */
	    for (c = 0; c < n_cw; c++) {
		p = -(logs3(pdf[c]));
		p += (1 << (s->shift-1)) - 1;	/* Rounding before truncation */
		if (pocketsphinx)
			p = (p < (MAX_NEG_MIXW << s->shift))
				? (p >> s->shift) : MAX_NEG_MIXW; /* Trunc/shift */
		else
			p = (p < (255 << s->shift)) ? (p >> s->shift) : 255;	/* Trunc/shift */
		fw[f].prob[j][c] = p;
	    }
	}
    }
    if (n_err > 0)
	E_WARN("Weight normalization failed for %d senones\n", n_err);

    ckd_free (pdf);

    if (chksum_present)
	bio_verify_chksum (fp, byteswap, chksum);
    
    if (fread (&eofchk, 1, 1, fp) == 1)
	E_FATAL("More data than expected in %s\n", file_name);

    fclose(fp);

    E_INFO("Read mixture weights for %d senones: %d features x %d codewords\n",
	   s->n_sen, s->n_feat, n_cw);
    
    return 0;
}

/* In the old S3 files, all senones have the same "shape" (#codewords/senone/feat) */
senone_t *senone_init (char *mixwfile, char *sen2mgau_map, float64 mixwfloor)
{
    senone_t *s;
    
    s = (senone_t *) ckd_calloc (1, sizeof(senone_t));
    s->n_sen = 0;	/* As yet unknown */
    s->sen2mgau = NULL;

    assert (sen2mgau_map);

    if (strcmp (sen2mgau_map, ".semi.") == 0) {
	/* Not a file; map all senones to a single parent mgau */
	s->n_mgau = 1;	/* But we don't yet know the #senones */
    } else if (strcmp (sen2mgau_map, ".cont.") == 0) {
	/* Not a file; map each senone to its own distinct parent mgau */
	s->n_mgau = 0;	/* We don't yet know the #senones */
    } else {
	/* Read mapping file */
	senone_mgau_map_read (s, sen2mgau_map);	/* Fills in n_sen */
    }
    
    assert (mixwfile);
    senone_mixw_read (s, mixwfile, mixwfloor);
    
    return s;
}

#if 0
int32 senone_eval (senone_t *s, s3senid_t sid, int32 f, int32 *dist, int32 n_dist)
{
    int32 i, c, scr, fscr;
    s3mgauid_t m;
    mixw_t *fw;
    
    m = s->sen2mgau[sid];
    assert ((m >= 0) && (m < s->n_mgau));

    fw = &(s->mgau2sen[m].feat_mixw[f]);
    assert (fw->n_wt == n_dist);

    i = s->mgau2sen_idx[sid];
    
    /* Weight first codeword */
    scr = dist[0] - (fw->prob[i][0] << s->shift);
    
    /* Remaining codewords */
    for (c = 1; c < fw->n_wt; c++) {
	fscr = dist[c] - (fw->prob[i][c] << s->shift);
	scr = logs3_add (scr, fscr);
    }
    
    return scr;
}


void senone_eval_all (senone_t *s, s3mgauid_t m, int32 f, int32 *dist, int32 n_dist,
		      int32 *senscr)
{
    int32 i, c, scr, fscr;
    s3senid_t sid;
    mixw_t *fw;
    
    fw = &(s->mgau2sen[m].feat_mixw[f]);
    assert (fw->n_wt == n_dist);
    
    for (i = 0; i < s->mgau2sen[m].n_sen; i++) {
	sid = s->mgau2sen[m].sen[i];
#if 1
	/* Weight first codeword */
	scr = dist[0] - (fw->prob[i][0] << s->shift);

	/* Remaining codewords */
	for (c = 1; c < fw->n_wt; c++) {
	    fscr = dist[c] - (fw->prob[i][c] << s->shift);
	    scr = logs3_add (scr, fscr);
	}

	senscr[sid] += scr;
#else
	senscr[sid] += senone_eval (s, sid, f, dist, n_dist);
#endif
    }
}
#endif

int main (int32 argc, char **argv)
{
    model_def_t *m;
    float64 wtflr;
    char *mdeffile, *senfile, *mgaumap, *feattype, *outfile;
    senone_t *s;
    
    parse_cmd_ln(argc, argv);

    feattype = "s2_4x";

    mdeffile = (char *)cmd_ln_access("-moddeffn");
    mgaumap = ".semi.";
    senfile = (char *)cmd_ln_access("-mixwfn");
    wtflr = (float64)(*(float32 *)cmd_ln_access("-mwfloor"));
    outfile = (char *)cmd_ln_access("-sendumpfn");
    
    logs3_init ((float64) 1.0001);
    feat_init (feattype);

    model_def_read(&m, mdeffile);
    s = senone_init (senfile, mgaumap, wtflr);
    printf("%p\n",s);
    if (m->n_tied_state != s->n_sen)
	E_FATAL("#senones different in mdef(%d) and mixw(%d) files\n", m->n_tied_state, s->n_sen);
    
    if (cmd_ln_boolean("-pocketsphinx")) {
	pocketsphinx_senone_dump(m, s, outfile);
    }
    else {
	senone_dump(m, s, outfile);
    }

    return 0;
}

