#ifndef SN_EDITOR_H
#define SN_EDITOR_H

#include <geanyplugin.h>

#include "sn_search.h"

ScintillaObject *sn_editor_current_sci(void);

sptr_t sn_editor_msg(
    ScintillaObject *sci,
    guint msg,
    uptr_t wparam,
    sptr_t lparam
);

void sn_editor_bell(void);

void sn_editor_enable_multicursor(ScintillaObject *sci);

gint sn_editor_main_selection(ScintillaObject *sci);

sptr_t sn_editor_sel_start(ScintillaObject *sci, gint n);
sptr_t sn_editor_sel_end(ScintillaObject *sci, gint n);

gboolean sn_editor_main_selection_empty(ScintillaObject *sci);
gboolean sn_editor_select_word_at_caret(ScintillaObject *sci);
gboolean sn_editor_main_selection_is_whole_word(ScintillaObject *sci);

gchar *sn_editor_get_range_text(
    ScintillaObject *sci,
    sptr_t start,
    sptr_t end,
    gsize *out_len
);

gchar *sn_editor_get_main_selection_text(
    ScintillaObject *sci,
    gsize *out_len
);

gchar *sn_editor_get_document_text(
    ScintillaObject *sci,
    gsize *out_len
);

SnRange *sn_editor_get_selected_ranges(
    ScintillaObject *sci,
    size_t *out_count
);

void sn_editor_add_selection(
    ScintillaObject *sci,
    sptr_t start,
    sptr_t end
);

void sn_editor_replace_single_selection(
    ScintillaObject *sci,
    sptr_t start,
    sptr_t end
);

#endif
