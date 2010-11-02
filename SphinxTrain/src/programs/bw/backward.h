/* ====================================================================
 * Copyright (c) 1995-2000 Carnegie Mellon University.  All rights 
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
 * File: backward.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef BACKWARD_H
#define BACKWARD_H

#include <sphinxbase/prim_type.h>
#include <s3/vector.h>
#include <s3/state.h>
#include <s3/model_inventory.h>

int32
backward_update(float64 **active_alpha,
		uint32 **active_astate,
		uint32 *n_active_astate,
		float64 *scale,
		float64 **dscale,
		vector_t **feature,
		uint32 n_obs,
		state_t *state_seq,
		uint32 n_state,
		model_inventory_t *inv,
		float64 beam,
		float32 spthresh,
		int32 mixw_reest,
		int32 tmat_reest,
		int32 mean_reest,
		int32 var_reest,
		int32 pass2var,
		int32 var_is_full,
		FILE *pdumpfn,
		float32 ***lda);

void
partial_op(float64 *p_op,
	   float64 op,

	   float64 **den,
	   uint32  **den_idx,
	   float32 **mixw,

	   uint32 n_feat,
	   uint32 n_top);

void
partial_ci_op(float64 *p_op,

	      float64 **den,
	      uint32  **den_idx,
	      float32 **mixw,

	      uint32 n_feat,
	      uint32 n_top);

void
den_terms_ci(float64 **d_term,

	     float64 post_j,
	     float64 *p_ci_op,
	     float64 **den,
	     uint32  **den_idx,
	     float32 **mixw,
	     uint32 n_feat,
	     uint32 n_top);

void
den_terms(float64 **d_term,
	  float64 p_reest_term,
	  float64 *p_op,
	  float64 **den,
	  uint32  **den_idx,
	  float32 **mixw,
	  uint32 n_feat,
	  uint32 n_top);

#endif /* BACKWARD_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  18:30:33  egouvea
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
 * Revision 1.7  97/07/16  11:38:16  eht
 * *** empty log message ***
 * 
 * Revision 1.6  1996/07/29  16:21:12  eht
 * float32 -> float64
 *
 * Revision 1.5  1996/03/26  15:17:51  eht
 * Fix beam definition bug
 *
 * Revision 1.4  1995/11/30  20:52:00  eht
 * Allow tmat_reest to be passed as an argument
 *
 * Revision 1.3  1995/10/12  18:22:18  eht
 * Updated comments and changed <s3/state.h> to "state.h"
 *
 * Revision 1.2  1995/10/10  12:44:06  eht
 * Changed to use <sphinxbase/prim_type.h>
 *
 * Revision 1.1  1995/06/02  20:43:56  eht
 * Initial revision
 *
 *
 */
