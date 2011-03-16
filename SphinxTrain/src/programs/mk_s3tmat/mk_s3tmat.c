/* ====================================================================
 * Copyright (c) 1996-2000 Carnegie Mellon University.  All rights 
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
 * File: mk_s3tmat.c
 * 
 * Description: 
 * 	Make a SPHINX-III transition matrix file from SPHINX-II
 *	HMM files.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include "parse_cmd_ln.h"

#include <s3/model_def_io.h>
#include <s3/acmod_set.h>
#include <sphinxbase/ckd_alloc.h>
#include <sphinxbase/cmd_ln.h>
#include <s3/s2_read_tmat.h>
#include <s3/s3tmat_io.h>
#include <s3/s2_param.h>
#include <s3/s3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

int
main(int argc, char *argv[])
{
    float32 ***tmat;
    model_def_t *mdef;
    uint32 n_mdef;
    char comment[4192];
    time_t t;

    parse_cmd_ln(argc, argv);
    
    E_INFO("Reading %s\n", cmd_ln_str("-moddeffn"));

    if (model_def_read(&mdef, cmd_ln_str("-moddeffn")) !=
	S3_SUCCESS) {
	exit(1);
    }

    n_mdef = mdef->n_defn;

    E_INFO("%d models defined\n", n_mdef);

    tmat = s2_read_tmat(cmd_ln_str("-hmmdir"),
			mdef->acmod_set,
			cmd_ln_float32("-floor"));

    t = time(NULL);
    sprintf(comment,
	    "Generated on %s\n\tmoddeffn: %s\n\tfloor: %e\n\thmmdir: %s\n\n\n\n\n\n\n\n\n",
	    ctime(&t),
	    cmd_ln_str("-moddeffn"),
	    cmd_ln_float32("-floor"),
	    cmd_ln_str("-hmmdir"));

    E_INFO("Writing %s\n",  cmd_ln_str("-tmatfn"));

    if (s3tmat_write(cmd_ln_str("-tmatfn"),
		     tmat,
		     acmod_set_n_ci(mdef->acmod_set),	/* total # transition matrices */
		     S2_N_STATE) != S3_SUCCESS) {
	E_ERROR_SYSTEM("Couldn't write transition matrix file");
    }

    return 0;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.5  2004/07/21  19:17:25  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.4  2004/06/17 19:17:23  arthchan2003
 * Code Update for silence deletion and standardize the name for command -line arguments
 *
 * Revision 1.3  2001/04/05 20:02:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:14  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.2  97/03/07  08:56:06  eht
 * - use E_*() routines for error and info output
 * - deal w/ new i/o routines
 * 
 * Revision 1.1  97/01/21  13:04:17  eht
 * Initial revision
 * 
 *
 */
