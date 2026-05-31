#include "sn_occurrence.h"

#include "sn_editor.h"
#include "sn_search.h"

static bool current_selection_is_whole_word(ScintillaObject *sci) {
    return sn_editor_main_selection_is_whole_word(sci) ? true : false;
}

static bool prepare_selection(ScintillaObject *sci) {
    sn_editor_enable_multicursor(sci);

    if (sn_editor_main_selection_empty(sci)) {
        return sn_editor_select_word_at_caret(sci) ? true : false;
    }

    return true;
}

static bool get_search_context(
    ScintillaObject *sci,
    gchar **out_doc_text,
    gsize *out_doc_len,
    gchar **out_needle,
    gsize *out_needle_len,
    SnRange **out_selected,
    size_t *out_selected_count
) {
    *out_doc_text = sn_editor_get_document_text(sci, out_doc_len);
    *out_needle = sn_editor_get_main_selection_text(sci, out_needle_len);
    *out_selected = sn_editor_get_selected_ranges(sci, out_selected_count);

    if (
        *out_doc_text == NULL ||
        *out_needle == NULL ||
        *out_needle_len == 0 ||
        (*out_selected == NULL && *out_selected_count != 0)
    ) {
        g_free(*out_doc_text);
        g_free(*out_needle);
        g_free(*out_selected);
        return false;
    }

    return true;
}

static void free_search_context(
    gchar *doc_text,
    gchar *needle,
    SnRange *selected
) {
    g_free(doc_text);
    g_free(needle);
    g_free(selected);
}

void sn_occurrence_add_next(guint key_id) {
    (void)key_id;

    ScintillaObject *sci = sn_editor_current_sci();

    if (sci == NULL) {
        return;
    }

    if (!prepare_selection(sci)) {
        sn_editor_bell();
        return;
    }

    gint main = sn_editor_main_selection(sci);
    SnPos current_start = (SnPos)sn_editor_sel_start(sci, main);
    SnPos current_end = (SnPos)sn_editor_sel_end(sci, main);

    gchar *doc_text = NULL;
    gchar *needle = NULL;
    SnRange *selected = NULL;
    gsize doc_len = 0;
    gsize needle_len = 0;
    size_t selected_count = 0;

    if (!get_search_context(
            sci,
            &doc_text,
            &doc_len,
            &needle,
            &needle_len,
            &selected,
            &selected_count
        )) {
        sn_editor_bell();
        return;
    }

    SnRange found = {0, 0};

    bool ok = sn_find_next_unselected_in_text(
        doc_text,
        doc_len,
        needle,
        needle_len,
        selected,
        selected_count,
        current_end,
        current_start,
        current_selection_is_whole_word(sci),
        true,
        &found
    );

    if (ok) {
        sn_editor_add_selection(sci, (sptr_t)found.start, (sptr_t)found.end);
    } else {
        sn_editor_bell();
    }

    free_search_context(doc_text, needle, selected);
}

void sn_occurrence_select_all(guint key_id) {
    (void)key_id;

    ScintillaObject *sci = sn_editor_current_sci();

    if (sci == NULL) {
        return;
    }

    if (!prepare_selection(sci)) {
        sn_editor_bell();
        return;
    }

    gchar *doc_text = NULL;
    gchar *needle = NULL;
    SnRange *selected = NULL;
    gsize doc_len = 0;
    gsize needle_len = 0;
    size_t selected_count = 0;

    if (!get_search_context(
            sci,
            &doc_text,
            &doc_len,
            &needle,
            &needle_len,
            &selected,
            &selected_count
        )) {
        sn_editor_bell();
        return;
    }

    size_t count = sn_collect_occurrences_in_text(
        doc_text,
        doc_len,
        needle,
        needle_len,
        current_selection_is_whole_word(sci),
        true,
        NULL,
        0
    );

    if (count == 0) {
        sn_editor_bell();
        free_search_context(doc_text, needle, selected);
        return;
    }

    SnRange *ranges = g_malloc(sizeof *ranges * count);

    size_t written = sn_collect_occurrences_in_text(
        doc_text,
        doc_len,
        needle,
        needle_len,
        current_selection_is_whole_word(sci),
        true,
        ranges,
        count
    );

    for (size_t i = 0; i < written; i++) {
        if (i == 0) {
            sn_editor_replace_single_selection(
                sci,
                (sptr_t)ranges[i].start,
                (sptr_t)ranges[i].end
            );
        } else {
            sn_editor_add_selection(
                sci,
                (sptr_t)ranges[i].start,
                (sptr_t)ranges[i].end
            );
        }
    }

    g_free(ranges);
    free_search_context(doc_text, needle, selected);
}

void sn_occurrence_skip_current(guint key_id) {
    (void)key_id;

    ScintillaObject *sci = sn_editor_current_sci();

    if (sci == NULL) {
        return;
    }

    if (!prepare_selection(sci)) {
        sn_editor_bell();
        return;
    }

    gint main = sn_editor_main_selection(sci);
    SnPos current_start = (SnPos)sn_editor_sel_start(sci, main);
    SnPos current_end = (SnPos)sn_editor_sel_end(sci, main);

    gchar *doc_text = NULL;
    gchar *needle = NULL;
    SnRange *selected = NULL;
    gsize doc_len = 0;
    gsize needle_len = 0;
    size_t selected_count = 0;

    if (!get_search_context(
            sci,
            &doc_text,
            &doc_len,
            &needle,
            &needle_len,
            &selected,
            &selected_count
        )) {
        sn_editor_bell();
        return;
    }

    SnRange found = {0, 0};

    bool ok = sn_find_next_unselected_in_text(
        doc_text,
        doc_len,
        needle,
        needle_len,
        selected,
        selected_count,
        current_end,
        current_start,
        current_selection_is_whole_word(sci),
        true,
        &found
    );

    if (!ok) {
        sn_editor_bell();
        free_search_context(doc_text, needle, selected);
        return;
    }

    gint selections = (gint)sn_editor_msg(sci, SCI_GETSELECTIONS, 0, 0);

    if (selections > 1) {
        sn_editor_msg(sci, SCI_DROPSELECTIONN, (uptr_t)main, 0);
        sn_editor_add_selection(sci, (sptr_t)found.start, (sptr_t)found.end);
    } else {
        sn_editor_replace_single_selection(sci, (sptr_t)found.start, (sptr_t)found.end);
    }

    free_search_context(doc_text, needle, selected);
}

void sn_occurrence_undo_last(guint key_id) {
    (void)key_id;

    ScintillaObject *sci = sn_editor_current_sci();

    if (sci == NULL) {
        return;
    }

    gint selections = (gint)sn_editor_msg(sci, SCI_GETSELECTIONS, 0, 0);

    if (selections > 1) {
        gint main = sn_editor_main_selection(sci);
        sn_editor_msg(sci, SCI_DROPSELECTIONN, (uptr_t)main, 0);
        sn_editor_msg(sci, SCI_SCROLLCARET, 0, 0);
    } else {
        sn_editor_bell();
    }
}

void sn_occurrence_clear(guint key_id) {
    (void)key_id;

    ScintillaObject *sci = sn_editor_current_sci();

    if (sci == NULL) {
        return;
    }

    sn_editor_msg(sci, SCI_CANCEL, 0, 0);
}
