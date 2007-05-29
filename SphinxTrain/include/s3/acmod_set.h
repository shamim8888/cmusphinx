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
/**********************************************************************
 *
 * File: acmod_set.h
 * 
 * Description: 
 *	This header defines the interface to the acmod_set (acoustic
 *	model set) module.  See acmod_set.c for the detailed
 *	descriptions of each function below.
 *
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef ACMOD_SET_H
#define ACMOD_SET_H


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include <s3/acmod_set_ds.h>	/* defines the data structures used by this
				 * module */
#include <s3/prim_type.h>

acmod_set_t *
acmod_set_new(void);

int32
acmod_set_set_n_ci_hint(acmod_set_t *acmod_set,
			uint32 n_ci_hint);

int32
acmod_set_set_n_tri_hint(acmod_set_t *acmod_set,
			 uint32 n_tri_hint);

acmod_id_t
acmod_set_add_ci(acmod_set_t *acmod_set,
		 const char *name,
		 const char **attrib);

acmod_id_t
acmod_set_add_tri(acmod_set_t *acmod_set,
		  acmod_id_t base,
		  acmod_id_t left_context,
		  acmod_id_t right_context,
		  word_posn_t posn,
		  const char **attrib);

acmod_id_t
acmod_set_name2id(acmod_set_t *acmod_set,
		  const char *name);

const char *
acmod_set_id2name(acmod_set_t *acmod_set,
		  acmod_id_t id);

const char *
acmod_set_id2fullname(acmod_set_t *acmod_set,
		      acmod_id_t id);

const char *
acmod_set_id2s2name(acmod_set_t *acmod_set,
		    acmod_id_t id);

acmod_id_t
acmod_set_enum_init(acmod_set_t *acmod_set,
		    acmod_id_t base);

acmod_id_t
acmod_set_enum(void);

acmod_id_t
acmod_set_tri2id(acmod_set_t *acmod_set,
		 acmod_id_t base,
		 acmod_id_t left_context,
		 acmod_id_t right_context,
		 word_posn_t posn);

int32
acmod_set_id2tri(acmod_set_t *acmod_set,
		 acmod_id_t *base,
		 acmod_id_t *left_context,
		 acmod_id_t *right_context,
		 word_posn_t *posn,
		 acmod_id_t id);

const char *
acmod_set_s2_id2name(acmod_set_t *acmod_set,
		     acmod_id_t id);

uint32
acmod_set_n_acmod(acmod_set_t *acmod_set);

uint32
acmod_set_n_base_acmod(acmod_set_t *acmod_set);

uint32
acmod_set_n_multi(acmod_set_t *acmod_set);

uint32
acmod_set_n_ci(acmod_set_t *acmod_set);

acmod_id_t
acmod_set_base_phone(acmod_set_t *acmod_set,
		     acmod_id_t id);

uint32
acmod_set_has_attrib(acmod_set_t *acmod_set,
		     acmod_id_t id,
		     const char *attrib);

const char **
acmod_set_attrib(acmod_set_t *acmod_set,
		 acmod_id_t id);

/* some SPHINX-II compatibility routines */
int
acmod_set_s2_parse_triphone(acmod_set_t *acmod_set,
			    acmod_id_t *base,
			    acmod_id_t *left,
			    acmod_id_t *right,
			    word_posn_t *posn,
			    char *str);
#ifdef __cplusplus
}
#endif

#endif /* ACMOD_SET_H */ 

/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.4  2004/07/21  17:46:08  egouvea
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
 * Revision 1.7  97/07/16  11:39:10  eht
 * *** empty log message ***
 * 
 * Revision 1.6  96/03/25  15:46:57  eht
 * Added acmod_set_id2s2name() to output SPHINX-II triphone names
 * 
 * Revision 1.5  1996/03/04  15:55:43  eht
 * Added ability to walk the triphone index trees
 *
 * Revision 1.4  1996/01/26  18:29:54  eht
 * Interface definition was incomplete.
 *
 * Revision 1.3  1995/10/09  20:55:35  eht
 * Changes needed for prim_type.h
 *
 * Revision 1.2  1995/10/09  20:29:29  eht
 * Changed "char *" to "const char *" for name2id call.
 *
 * Revision 1.1  1995/09/08  19:13:52  eht
 * Initial revision
 *
 * Revision 1.1  95/08/15  13:44:14  13:44:14  eht (Eric Thayer)
 * Initial revision
 * 
 *
 */
