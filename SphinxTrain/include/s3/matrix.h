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
 * File: matrix.h
 * 
 * Description: Matrix and linear algebra functions
 * 
 * Author: 
 * 
 *********************************************************************/

#ifndef MATRIX_H
#define MATRIX_H

#include <s3/prim_type.h>
#include <s3/vector.h>

void
norm_3d(float32 ***arr,
	uint32 d1,
	uint32 d2,
	uint32 d3);

void
accum_3d(float32 ***out,
	 float32 ***in,
	 uint32 d1,
	 uint32 d2,
	 uint32 d3);

void
floor_3d(float32 ***m,
	 uint32 d1,
	 uint32 d2,
	 uint32 d3,
	 float32 floor);

void
floor_nz_3d(float32 ***m,
	    uint32 d1,
	    uint32 d2,
	    uint32 d3,
	    float32 floor);

void
floor_nz_1d(float32 *v,
	    uint32 d1,
	    float32 floor);

void
band_nz_1d(float32 *v,
	   uint32 d1,
	   float32 band);

/* These can be vanishingly small hence the float64 */
float64
determinant(float32 **a, int32 len);

/* Returns S3_ERROR for a singular matrix. */
int32
invert(vector_t *ainv, vector_t *a, int32 len);

void
outerproduct(vector_t *a, vector_t x, vector_t y, int32 len);

int32
eigenvectors(float32 **a,
	     float32 *out_ur, float32 *out_ui,
	     float32 **out_vr, float32 **out_vi,
	     int32 len);

void
matrixmultiply(vector_t *c, /* = */
	       vector_t *a, /* * */ vector_t *b,
	       int32 m, /**< #rows of a. */
	       int32 n, /**< #cols of b. */
	       int32 k  /**< #cols of a, #rows of b. */
	);

void
scalarmultiply(vector_t *a, float32 x, int32 m, int32 n);

void
matrixadd(vector_t *a, vector_t *b, int32 m, int32 n);

#endif /* MATRIX_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  17:46:09  egouvea
 * Changed the license terms to make it the same as sphinx2 and sphinx3.
 * 
 * Revision 1.3  2001/04/05 20:02:30  awb
 * *** empty log message ***
 *
 * Revision 1.2  2000/09/29 22:35:12  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:30  awb
 * *** empty log message ***
 *
 * Revision 1.1  97/07/16  11:39:10  eht
 * Initial revision
 * 
 *
 */
