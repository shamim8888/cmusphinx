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
 * File: vector.c
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#include <s3/vector.h>
#include <s3/ckd_alloc.h>
#include <s3/s3.h>

#include <stdio.h>
#include <assert.h>

vector_t
vector_alloc(int32 n_dim)
{
    return (vector_t)ckd_calloc(n_dim, sizeof(float32));
}

int
vector_free(vector_t v)
{
    ckd_free(v);

    return S3_SUCCESS;
}

void
vector_floor(vector_t v, uint32 dim, float32 f)
{
    unsigned int i;

    for (i = 0; i < dim; i++)
	if (v[i] < f) v[i] = f;
}

void
vector_nz_floor(vector_t v, uint32 dim, float32 f)
{
    unsigned int i;

    for (i = 0; i < dim; i++)
	if ((v[i] != 0) && (v[i] < f)) v[i] = f;
}

int
vector_normalize(vector_t v,
		 uint32 dim)
{
    uint32 i;
    float64 s;
    int ret_val = S3_SUCCESS;

    for (i = 0, s = 0; i < dim; i++)
	s += v[i];

    if (s != 0) {
	for (i = 0; i < dim; i++)
	    v[i] /= s;
    }
    else {
	ret_val = S3_WARNING;
    }

    return ret_val;
}

void
vector_print(vector_t v,
	     uint32 dim)
{
    uint32 i;
    float64 s;
    
    for (i = 0, s = 0; i < dim; i++)
	printf("%e ", v[i]);
}

void
vector_add(vector_t a, const vector_t b, uint32 dim)
{
	uint32 i;
	for (i = 0; i < dim; ++i)
		a[i] += b[i];
}

void
vector_sub(vector_t a, const vector_t b, uint32 dim)
{
	uint32 i;
	for (i = 0; i < dim; ++i)
		a[i] -= b[i];
}

void
vector_scale(vector_t a, float32 b, uint32 dim)
{
	uint32 i;
	for (i = 0; i < dim; ++i)
		a[i] *= b;
}

void
vector_cross(vector_t a, const vector_t b, uint32 dim)
{
	uint32 i;
	for (i = 0; i < dim; ++i)
		a[i] *= b[i];
}

float64
vector_dot(const vector_t a, const vector_t b, uint32 dim)
{
	uint32 i;
	float64 p = 0.0;

	for (i = 0; i < dim; ++i)
		p += a[i] * b[i];
	return p;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  18:05:40  egouvea
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
 * Revision 1.6  96/10/30  07:43:33  eht
 * Removed warning message that was just cluttering up the output
 * 
 * Revision 1.5  95/10/10  17:37:33  eht
 * Fixed some bogus return values
 * 
 * Revision 1.4  1995/10/09  20:56:36  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.3  1995/10/09  15:02:03  eht
 * Changed ckd_alloc interface to get rid of __FILE__, __LINE__ arguments
 *
 * Revision 1.2  1995/06/02  14:52:54  eht
 * Use PWP's error package
 *
 * Revision 1.1  1995/02/13  15:48:25  eht
 * Initial revision
 *
 *
 */
