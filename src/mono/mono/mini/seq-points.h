/**
 * \file
 * Copyright 2014 Xamarin Inc
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */

#ifndef __MONO_SEQ_POINTS_H__
#define __MONO_SEQ_POINTS_H__

#include <mono/metadata/seq-points-data.h>

void
mono_save_seq_point_info (MonoCompile *cfg, MonoJitInfo *jinfo);

MONO_COMPONENT_API MonoSeqPointInfo*
mono_get_seq_points (MonoMethod *method);

MONO_COMPONENT_API gboolean
mono_find_next_seq_point_for_native_offset (MonoMethod *method, gint32 native_offset, MonoSeqPointInfo **info, SeqPoint* seq_point);

MONO_COMPONENT_API gboolean
mono_find_prev_seq_point_for_native_offset (MonoMethod *method, gint32 native_offset, MonoSeqPointInfo **info, SeqPoint* seq_point);

MONO_COMPONENT_API gboolean
mono_find_seq_point (MonoMethod *method, gint32 il_offset, MonoSeqPointInfo **info, SeqPoint *seq_point);

void
mono_bb_deduplicate_op_il_seq_points (MonoCompile *cfg, MonoBasicBlock *bb);

#endif /* __MONO_SEQ_POINTS_H__ */
