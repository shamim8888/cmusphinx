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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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
 * File: next_utt_states.h
 * 
 * Description: 
 * 
 * Author: 
 * 	Eric H. Thayer (eht@cs.cmu.edu)
 *********************************************************************/

#ifndef NEXT_UTT_STATES_H
#define NEXT_UTT_STATES_H

#include <s3/state.h>
#include <s3/prim_type.h>
#include <s3/lexicon.h>
#include <s3/model_inventory.h>
#include <s3/model_def.h>

state_t *next_utt_states(uint32 *n_state,
			 lexicon_t *lex,
			 model_inventory_t *inv,
			 model_def_t *mdef,
			 char *transcript);

#endif /* NEXT_UTT_STATES_H */ 


/*
 * Log record.  Maintained by RCS.
 *
 * $Log$
 * Revision 1.3  2001/04/05  20:02:31  awb
 * *** empty log message ***
 * 
 * Revision 1.2  2000/09/29 22:35:13  awb
 * *** empty log message ***
 *
 * Revision 1.1  2000/09/24 21:38:31  awb
 * *** empty log message ***
 *
 * Revision 1.5  1996/07/29  16:22:02  eht
 * Mixxing includes
 *
 * Revision 1.4  1995/10/12  18:22:18  eht
 * Updated comments and changed <s3/state.h> to "state.h"
 *
 * Revision 1.3  1995/10/10  12:44:06  eht
 * Changed to use <s3/prim_type.h>
 *
 * Revision 1.2  1995/08/09  20:20:10  eht
 * Use mdef structure for tying info rather than DAG thing
 *
 * Revision 1.1  1995/06/02  20:43:56  eht
 * Initial revision
 *
 *
 */
