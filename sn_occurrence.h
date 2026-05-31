#ifndef SN_OCCURRENCE_H
#define SN_OCCURRENCE_H

#include <geanyplugin.h>

void sn_occurrence_add_next(guint key_id);
void sn_occurrence_select_all(guint key_id);
void sn_occurrence_skip_current(guint key_id);
void sn_occurrence_undo_last(guint key_id);
void sn_occurrence_clear(guint key_id);

#endif
