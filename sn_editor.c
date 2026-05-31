#include "sn_editor.h"

ScintillaObject *sn_editor_current_sci(void) {
    GeanyDocument *doc = document_get_current();

    if (doc == NULL || doc->editor == NULL || doc->editor->sci == NULL) {
        return NULL;
    }

    return doc->editor->sci;
}

sptr_t sn_editor_msg(
    ScintillaObject *sci,
    guint msg,
    uptr_t wparam,
    sptr_t lparam
) {
    return scintilla_send_message(sci, msg, wparam, lparam);
}

void sn_editor_bell(void) {
    GdkDisplay *display = gdk_display_get_default();

    if (display != NULL) {
        gdk_display_beep(display);
    }
}

void sn_editor_enable_multicursor(ScintillaObject *sci) {
    sn_editor_msg(sci, SCI_SETMULTIPLESELECTION, TRUE, 0);
    sn_editor_msg(sci, SCI_SETADDITIONALSELECTIONTYPING, TRUE, 0);
    sn_editor_msg(sci, SCI_SETMULTIPASTE, SC_MULTIPASTE_EACH, 0);
    sn_editor_msg(sci, SCI_SETADDITIONALCARETSVISIBLE, TRUE, 0);
    sn_editor_msg(sci, SCI_SETADDITIONALCARETSBLINK, TRUE, 0);
}

gint sn_editor_main_selection(ScintillaObject *sci) {
    return (gint)sn_editor_msg(sci, SCI_GETMAINSELECTION, 0, 0);
}

sptr_t sn_editor_sel_start(ScintillaObject *sci, gint n) {
    return sn_editor_msg(sci, SCI_GETSELECTIONNSTART, (uptr_t)n, 0);
}

sptr_t sn_editor_sel_end(ScintillaObject *sci, gint n) {
    return sn_editor_msg(sci, SCI_GETSELECTIONNEND, (uptr_t)n, 0);
}

gboolean sn_editor_main_selection_empty(ScintillaObject *sci) {
    gint main = sn_editor_main_selection(sci);

    return sn_editor_sel_start(sci, main) == sn_editor_sel_end(sci, main);
}

gboolean sn_editor_select_word_at_caret(ScintillaObject *sci) {
    sptr_t pos = sn_editor_msg(sci, SCI_GETCURRENTPOS, 0, 0);

    sptr_t start = sn_editor_msg(sci, SCI_WORDSTARTPOSITION, (uptr_t)pos, TRUE);
    sptr_t end = sn_editor_msg(sci, SCI_WORDENDPOSITION, (uptr_t)pos, TRUE);

    if (start == end) {
        return FALSE;
    }

    sn_editor_msg(sci, SCI_SETSELECTION, (uptr_t)end, start);
    return TRUE;
}

gboolean sn_editor_main_selection_is_whole_word(ScintillaObject *sci) {
    gint main = sn_editor_main_selection(sci);

    sptr_t start = sn_editor_sel_start(sci, main);
    sptr_t end = sn_editor_sel_end(sci, main);

    if (start == end) {
        return FALSE;
    }

    sptr_t word_start = sn_editor_msg(
        sci,
        SCI_WORDSTARTPOSITION,
        (uptr_t)start,
        TRUE
    );

    sptr_t word_end = sn_editor_msg(
        sci,
        SCI_WORDENDPOSITION,
        (uptr_t)start,
        TRUE
    );

    return start == word_start && end == word_end;
}

gchar *sn_editor_get_range_text(
    ScintillaObject *sci,
    sptr_t start,
    sptr_t end,
    gsize *out_len
) {
    if (end <= start) {
        return NULL;
    }

    gsize len = (gsize)(end - start);
    gchar *text = g_malloc(len + 1);

    struct Sci_TextRange tr;
    tr.chrg.cpMin = start;
    tr.chrg.cpMax = end;
    tr.lpstrText = text;

    sn_editor_msg(sci, SCI_GETTEXTRANGE, 0, (sptr_t)&tr);

    text[len] = '\0';

    if (out_len != NULL) {
        *out_len = len;
    }

    return text;
}

gchar *sn_editor_get_main_selection_text(
    ScintillaObject *sci,
    gsize *out_len
) {
    gint main = sn_editor_main_selection(sci);

    return sn_editor_get_range_text(
        sci,
        sn_editor_sel_start(sci, main),
        sn_editor_sel_end(sci, main),
        out_len
    );
}

gchar *sn_editor_get_document_text(
    ScintillaObject *sci,
    gsize *out_len
) {
    sptr_t doc_len_sp = sn_editor_msg(sci, SCI_GETLENGTH, 0, 0);

    if (doc_len_sp < 0) {
        return NULL;
    }

    gsize doc_len = (gsize)doc_len_sp;
    gchar *text = g_malloc(doc_len + 1);

    sn_editor_msg(sci, SCI_GETTEXT, (uptr_t)(doc_len + 1), (sptr_t)text);
    text[doc_len] = '\0';

    if (out_len != NULL) {
        *out_len = doc_len;
    }

    return text;
}

SnRange *sn_editor_get_selected_ranges(
    ScintillaObject *sci,
    size_t *out_count
) {
    gint count_g = (gint)sn_editor_msg(sci, SCI_GETSELECTIONS, 0, 0);

    if (count_g <= 0) {
        if (out_count != NULL) {
            *out_count = 0;
        }

        return NULL;
    }

    size_t count = (size_t)count_g;
    SnRange *ranges = g_malloc(sizeof *ranges * count);

    for (size_t i = 0; i < count; i++) {
        gint idx = (gint)i;
        ranges[i].start = (SnPos)sn_editor_sel_start(sci, idx);
        ranges[i].end = (SnPos)sn_editor_sel_end(sci, idx);
    }

    if (out_count != NULL) {
        *out_count = count;
    }

    return ranges;
}

void sn_editor_add_selection(
    ScintillaObject *sci,
    sptr_t start,
    sptr_t end
) {
    sn_editor_msg(sci, SCI_ADDSELECTION, (uptr_t)end, start);
    sn_editor_msg(sci, SCI_SCROLLCARET, 0, 0);
}

void sn_editor_replace_single_selection(
    ScintillaObject *sci,
    sptr_t start,
    sptr_t end
) {
    sn_editor_msg(sci, SCI_SETSELECTION, (uptr_t)end, start);
    sn_editor_msg(sci, SCI_SCROLLCARET, 0, 0);
}
