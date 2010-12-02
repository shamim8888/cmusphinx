/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 2010 Carnegie Mellon University.  All rights
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

/**
 * @file arc_buffer.h Queue passing hypotheses (arcs) between search passes
 */

#ifndef __ARC_BUFFER_H__
#define __ARC_BUFFER_H__

/* SphinxBase headers. */
#include <sphinxbase/garray.h>
#include <sphinxbase/sbthread.h>
#include <sphinxbase/bitvec.h>

/* Local headers. */
#include "bptbl.h"

typedef struct arc_s {
    int32 wid;
    int32 src;
    int32 dest;
} arc_t;

typedef struct sarc_s {
    arc_t arc;
    int32 score;
    int32 rc_idx;
    bitvec_t rc_bits[0];
} sarc_t;

typedef struct arc_buffer_s {
    int refcount;
    char *name;
    sbmtx_t *mtx;
    sbevent_t *evt;
    garray_t *arcs;
    garray_t *sf_idx;
    garray_t *rc_deltas;
    bptbl_t *input_bptbl;
    int final;
    int scores;
    int arc_size;
    int active_sf; /**< First frame of incoming arcs. */
    int next_sf;   /**< First frame not containing arcs (last frame + 1). */
    bpidx_t next_idx;  /**< Next bptbl index to scan from. */
    int active_arc; /**< First incoming arc. */
} arc_buffer_t;

/**
 * Create a new arc buffer.
 */
arc_buffer_t *arc_buffer_init(char const *name,
                              bptbl_t *input_bptbl, int keep_scores);

/**
 * Retain a pointer to an arc buffer.
 */
arc_buffer_t *arc_buffer_retain(arc_buffer_t *fab);

/**
 * Release a pointer to an arc buffer.
 */
int arc_buffer_free(arc_buffer_t *fab);

/**
 * Clear the contents of an arc buffer and mark it non-final.
 */
void arc_buffer_reset(arc_buffer_t *fab);

/**
 * Dump contents of arc buffer for debugging.
 */
void arc_buffer_dump(arc_buffer_t *fab, dict_t *dict);

/**
 * Extend the arc buffer up to the given frame index.
 *
 * @param next_sf Index of last start frame to be added to the buffer,
 * plus one.
 */
int arc_buffer_extend(arc_buffer_t *fab, int next_sf);

/**
 * Sweep newly available arcs from the bptbl into the arc buffer and
 * commit them.
 *
 * @param release If true, elease arcs from input_bptbl.
 * @return The first backpointer index between start and end which
 *         starts after the next active frame, i.e. the first
 *         backpointer index which must be preserved for the next pass
 *         of arc addition.
 */
bpidx_t arc_buffer_sweep(arc_buffer_t *fab, int release);

/**
 * Sweep all remaining arcs from the bptbl into the arc buffer and
 * mark it as final.

 * @param release If true, elease arcs from input_bptbl.
 * @return 0 or <0 for failure.
 */
int arc_buffer_finalize(arc_buffer_t *fab, int release);

/**
 * Mostly internal function for adding bps.
 */
bpidx_t arc_buffer_add_bps(arc_buffer_t *fab,
                           bptbl_t *bptbl, bpidx_t start,
                           bpidx_t end);

/**
 * Commit extended arcs to the arc buffer.
 *
 * This freezes in place the start frames added since the last call to
 * arc_buffer_extend().  No more arcs with these start frames may be
 * added to the arc buffer.
 *
 * It also signals the consumer thread.  IMPORTANT: It is assumed that
 * there is only one consumer thread.
 */
int arc_buffer_commit(arc_buffer_t *fab);

/**
 * Iterate over arcs in the arc buffer starting at given frame.
 *
 * @param sf Frame to iterate over.
 * @return First arc in frame, or NULL if frame not available.
 */
arc_t *arc_buffer_iter(arc_buffer_t *fab, int sf);

/**
 * Move the arc pointer forward.
 */
arc_t *arc_next(arc_buffer_t *fab, arc_t *ab);

/**
 * Lock the arc buffer.
 *
 * The pointers returned by arc_buffer_iter() and arc_next() can
 * become invalid if the buffer is resized while they are held.  To
 * avoid this, lock the arc buffer before iterating over it and unlock
 * it afterwards.
 */
void arc_buffer_lock(arc_buffer_t *fab);

/**
 * Unlock the arc buffer.
 */
void arc_buffer_unlock(arc_buffer_t *fab);

/**
 * Wait until new arcs are committed (or the buffer is finalized)
 *
 * IMPORTANT: It is currently assumed that only one thread will ever
 * wait on an arc buffer.  If multiple threads wait on the same arc
 * buffer, signals may be lost.
 *
 * @return Next start frame which will be available (i.e. currently
 * available frame plus one).
 */
int arc_buffer_wait(arc_buffer_t *fab, int timeout);

/**
 * Release old arcs from the arc buffer.
 *
 * This releases all arcs starting in frames before first_sf.  It
 * should be called from the consumer thread only.
 */
int arc_buffer_release(arc_buffer_t *fab, int first_sf);

#endif /* __ARC_BUFFER_H__ */
