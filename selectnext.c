#include <geanyplugin.h>

GeanyPlugin *geany_plugin;
GeanyData *geany_data;

enum {
    KB_ADD_NEXT,
    KB_SELECT_ALL,
    KB_SKIP_CURRENT,
    KB_UNDO_LAST,
    KB_CLEAR,
    KB_COUNT
};

static GeanyKeyGroup *key_group;

static sptr_t sci_msg(ScintillaObject *sci, guint msg, uptr_t wparam, sptr_t lparam) {
    return scintilla_send_message(sci, msg, wparam, lparam);
}

static void bell(void) {
    GdkDisplay *display = gdk_display_get_default();

    if (display != NULL) {
        gdk_display_beep(display);
    }
}

static ScintillaObject *current_sci(void) {
    GeanyDocument *doc = document_get_current();

    if (doc == NULL || doc->editor == NULL || doc->editor->sci == NULL) {
        return NULL;
    }

    return doc->editor->sci;
}

static void enable_multi_cursor(ScintillaObject *sci) {
    sci_msg(sci, SCI_SETMULTIPLESELECTION, TRUE, 0);
    sci_msg(sci, SCI_SETADDITIONALSELECTIONTYPING, TRUE, 0);
    sci_msg(sci, SCI_SETMULTIPASTE, SC_MULTIPASTE_EACH, 0);
    sci_msg(sci, SCI_SETADDITIONALCARETSVISIBLE, TRUE, 0);
    sci_msg(sci, SCI_SETADDITIONALCARETSBLINK, TRUE, 0);
}

static gint main_selection(ScintillaObject *sci) {
    return (gint)sci_msg(sci, SCI_GETMAINSELECTION, 0, 0);
}

static sptr_t sel_start(ScintillaObject *sci, gint n) {
    return sci_msg(sci, SCI_GETSELECTIONNSTART, (uptr_t)n, 0);
}

static sptr_t sel_end(ScintillaObject *sci, gint n) {
    return sci_msg(sci, SCI_GETSELECTIONNEND, (uptr_t)n, 0);
}

static gboolean main_selection_empty(ScintillaObject *sci) {
    gint main = main_selection(sci);
    return sel_start(sci, main) == sel_end(sci, main);
}

static gboolean select_word_at_caret(ScintillaObject *sci) {
    sptr_t pos = sci_msg(sci, SCI_GETCURRENTPOS, 0, 0);
    sptr_t start = sci_msg(sci, SCI_WORDSTARTPOSITION, (uptr_t)pos, TRUE);
    sptr_t end = sci_msg(sci, SCI_WORDENDPOSITION, (uptr_t)pos, TRUE);

    if (start == end) {
        return FALSE;
    }

    sci_msg(sci, SCI_SETSELECTION, (uptr_t)end, start);
    return TRUE;
}

static gboolean main_selection_is_whole_word(ScintillaObject *sci) {
    gint main = main_selection(sci);
    sptr_t start = sel_start(sci, main);
    sptr_t end = sel_end(sci, main);

    if (start == end) {
        return FALSE;
    }

    sptr_t word_start = sci_msg(sci, SCI_WORDSTARTPOSITION, (uptr_t)start, TRUE);
    sptr_t word_end = sci_msg(sci, SCI_WORDENDPOSITION, (uptr_t)start, TRUE);

    return start == word_start && end == word_end;
}

static gint search_flags_for_current_selection(ScintillaObject *sci) {
    gint flags = SCFIND_MATCHCASE;

    if (main_selection_is_whole_word(sci)) {
        flags |= SCFIND_WHOLEWORD;
    }

    return flags;
}

static gchar *get_range_text(ScintillaObject *sci, sptr_t start, sptr_t end, gsize *out_len) {
    if (end <= start) {
        return NULL;
    }

    gsize len = (gsize)(end - start);
    gchar *text = g_malloc(len + 1);

    struct Sci_TextRange tr;
    tr.chrg.cpMin = start;
    tr.chrg.cpMax = end;
    tr.lpstrText = text;

    sci_msg(sci, SCI_GETTEXTRANGE, 0, (sptr_t)&tr);

    text[len] = '\0';

    if (out_len != NULL) {
        *out_len = len;
    }

    return text;
}

static gchar *get_main_selection_text(ScintillaObject *sci, gsize *out_len) {
    gint main = main_selection(sci);
    return get_range_text(sci, sel_start(sci, main), sel_end(sci, main), out_len);
}

static gboolean range_is_selected(ScintillaObject *sci, sptr_t start, sptr_t end) {
    gint count = (gint)sci_msg(sci, SCI_GETSELECTIONS, 0, 0);

    for (gint i = 0; i < count; i++) {
        if (sel_start(sci, i) == start && sel_end(sci, i) == end) {
            return TRUE;
        }
    }

    return FALSE;
}

static gboolean search_range(
    ScintillaObject *sci,
    const gchar *needle,
    gsize needle_len,
    sptr_t from,
    sptr_t to,
    sptr_t *out_start,
    sptr_t *out_end
) {
    sci_msg(sci, SCI_SETTARGETSTART, (uptr_t)from, 0);
    sci_msg(sci, SCI_SETTARGETEND, (uptr_t)to, 0);

    sptr_t found = sci_msg(
        sci,
        SCI_SEARCHINTARGET,
        (uptr_t)needle_len,
        (sptr_t)needle
    );

    if (found < 0) {
        return FALSE;
    }

    *out_start = sci_msg(sci, SCI_GETTARGETSTART, 0, 0);
    *out_end = sci_msg(sci, SCI_GETTARGETEND, 0, 0);

    return TRUE;
}

static gboolean find_next_unselected(
    ScintillaObject *sci,
    const gchar *needle,
    gsize needle_len,
    sptr_t from,
    sptr_t stop_before,
    sptr_t *out_start,
    sptr_t *out_end
) {
    sptr_t doc_len = sci_msg(sci, SCI_GETLENGTH, 0, 0);
    sptr_t start = from;

    while (start < doc_len) {
        sptr_t match_start = 0;
        sptr_t match_end = 0;

        if (!search_range(sci, needle, needle_len, start, doc_len, &match_start, &match_end)) {
            break;
        }

        if (!range_is_selected(sci, match_start, match_end)) {
            *out_start = match_start;
            *out_end = match_end;
            return TRUE;
        }

        start = match_end;
    }

    start = 0;

    while (start < stop_before) {
        sptr_t match_start = 0;
        sptr_t match_end = 0;

        if (!search_range(sci, needle, needle_len, start, stop_before, &match_start, &match_end)) {
            break;
        }

        if (!range_is_selected(sci, match_start, match_end)) {
            *out_start = match_start;
            *out_end = match_end;
            return TRUE;
        }

        start = match_end;
    }

    return FALSE;
}

static void add_selection(ScintillaObject *sci, sptr_t start, sptr_t end) {
    sci_msg(sci, SCI_ADDSELECTION, (uptr_t)end, start);
    sci_msg(sci, SCI_SCROLLCARET, 0, 0);
}

static void replace_single_selection(ScintillaObject *sci, sptr_t start, sptr_t end) {
    sci_msg(sci, SCI_SETSELECTION, (uptr_t)end, start);
    sci_msg(sci, SCI_SCROLLCARET, 0, 0);
}

static void kb_add_next(guint key_id) {
    (void)key_id;

    ScintillaObject *sci = current_sci();

    if (sci == NULL) {
        return;
    }

    enable_multi_cursor(sci);

    if (main_selection_empty(sci)) {
        if (!select_word_at_caret(sci)) {
            bell();
        }
        return;
    }

    gint flags = search_flags_for_current_selection(sci);
    sci_msg(sci, SCI_SETSEARCHFLAGS, (uptr_t)flags, 0);

    gint main = main_selection(sci);
    sptr_t current_start = sel_start(sci, main);
    sptr_t current_end = sel_end(sci, main);

    gsize needle_len = 0;
    gchar *needle = get_main_selection_text(sci, &needle_len);

    if (needle == NULL || needle_len == 0) {
        g_free(needle);
        bell();
        return;
    }

    sptr_t match_start = 0;
    sptr_t match_end = 0;

    if (find_next_unselected(
            sci,
            needle,
            needle_len,
            current_end,
            current_start,
            &match_start,
            &match_end
        )) {
        add_selection(sci, match_start, match_end);
    } else {
        bell();
    }

    g_free(needle);
}

static void kb_select_all(guint key_id) {
    (void)key_id;

    ScintillaObject *sci = current_sci();

    if (sci == NULL) {
        return;
    }

    enable_multi_cursor(sci);

    if (main_selection_empty(sci)) {
        if (!select_word_at_caret(sci)) {
            bell();
            return;
        }
    }

    gint flags = search_flags_for_current_selection(sci);
    sci_msg(sci, SCI_SETSEARCHFLAGS, (uptr_t)flags, 0);

    gsize needle_len = 0;
    gchar *needle = get_main_selection_text(sci, &needle_len);

    if (needle == NULL || needle_len == 0) {
        g_free(needle);
        bell();
        return;
    }

    sptr_t doc_len = sci_msg(sci, SCI_GETLENGTH, 0, 0);
    sptr_t search_from = 0;
    gboolean found_any = FALSE;

    while (search_from < doc_len) {
        sptr_t match_start = 0;
        sptr_t match_end = 0;

        if (!search_range(
                sci,
                needle,
                needle_len,
                search_from,
                doc_len,
                &match_start,
                &match_end
            )) {
            break;
        }

        if (!found_any) {
            replace_single_selection(sci, match_start, match_end);
            found_any = TRUE;
        } else {
            add_selection(sci, match_start, match_end);
        }

        search_from = match_end;
    }

    if (!found_any) {
        bell();
    }

    g_free(needle);
}

static void kb_skip_current(guint key_id) {
    (void)key_id;

    ScintillaObject *sci = current_sci();

    if (sci == NULL) {
        return;
    }

    enable_multi_cursor(sci);

    if (main_selection_empty(sci)) {
        if (!select_word_at_caret(sci)) {
            bell();
        }
        return;
    }

    gint flags = search_flags_for_current_selection(sci);
    sci_msg(sci, SCI_SETSEARCHFLAGS, (uptr_t)flags, 0);

    gint main = main_selection(sci);
    sptr_t current_start = sel_start(sci, main);
    sptr_t current_end = sel_end(sci, main);

    gsize needle_len = 0;
    gchar *needle = get_main_selection_text(sci, &needle_len);

    if (needle == NULL || needle_len == 0) {
        g_free(needle);
        bell();
        return;
    }

    sptr_t match_start = 0;
    sptr_t match_end = 0;

    if (!find_next_unselected(
            sci,
            needle,
            needle_len,
            current_end,
            current_start,
            &match_start,
            &match_end
        )) {
        g_free(needle);
        bell();
        return;
    }

    gint selections = (gint)sci_msg(sci, SCI_GETSELECTIONS, 0, 0);

    if (selections > 1) {
        sci_msg(sci, SCI_DROPSELECTIONN, (uptr_t)main, 0);
        add_selection(sci, match_start, match_end);
    } else {
        replace_single_selection(sci, match_start, match_end);
    }

    g_free(needle);
}

static void kb_undo_last(guint key_id) {
    (void)key_id;

    ScintillaObject *sci = current_sci();

    if (sci == NULL) {
        return;
    }

    gint selections = (gint)sci_msg(sci, SCI_GETSELECTIONS, 0, 0);

    if (selections > 1) {
        gint main = main_selection(sci);
        sci_msg(sci, SCI_DROPSELECTIONN, (uptr_t)main, 0);
        sci_msg(sci, SCI_SCROLLCARET, 0, 0);
    } else {
        bell();
    }
}

static void kb_clear(guint key_id) {
    (void)key_id;

    ScintillaObject *sci = current_sci();

    if (sci == NULL) {
        return;
    }

    sci_msg(sci, SCI_CANCEL, 0, 0);
}

static gboolean selectnext_init(GeanyPlugin *plugin, gpointer pdata) {
    (void)pdata;

    geany_plugin = plugin;
    geany_data = plugin->geany_data;

    key_group = plugin_set_key_group(plugin, "selectnext", KB_COUNT, NULL);

    keybindings_set_item(
        key_group,
        KB_ADD_NEXT,
        kb_add_next,
        GDK_KEY_d,
        GEANY_PRIMARY_MOD_MASK,
        "add_next_occurrence",
        "Add Next Occurrence",
        NULL
    );

    keybindings_set_item(
        key_group,
        KB_SELECT_ALL,
        kb_select_all,
        GDK_KEY_d,
        GEANY_PRIMARY_MOD_MASK | GDK_SHIFT_MASK,
        "select_all_occurrences",
        "Select All Occurrences",
        NULL
    );

    keybindings_set_item(
        key_group,
        KB_SKIP_CURRENT,
        kb_skip_current,
        GDK_KEY_d,
        GDK_MOD1_MASK,
        "skip_current_occurrence",
        "Skip Current Occurrence",
        NULL
    );

    keybindings_set_item(
        key_group,
        KB_UNDO_LAST,
        kb_undo_last,
        GDK_KEY_d,
        GEANY_PRIMARY_MOD_MASK | GDK_MOD1_MASK,
        "undo_last_occurrence",
        "Undo Last Occurrence",
        NULL
    );

    keybindings_set_item(
        key_group,
        KB_CLEAR,
        kb_clear,
        0,
        0,
        "clear_multiple_selections",
        "Clear Multiple Selections",
        NULL
    );

    return TRUE;
}

static void selectnext_cleanup(GeanyPlugin *plugin, gpointer pdata) {
    (void)plugin;
    (void)pdata;
}

G_MODULE_EXPORT
void geany_load_module(GeanyPlugin *plugin) {
    geany_plugin = plugin;
    geany_data = plugin->geany_data;

    plugin->info->name = "Select Next Occurrence";
    plugin->info->description = "VS Code/Sublime-style select next occurrence and multi-cursor editing.";
    plugin->info->version = "0.2";
    plugin->info->author = "Eftakharul Islam";

    plugin->funcs->init = selectnext_init;
    plugin->funcs->cleanup = selectnext_cleanup;

    GEANY_PLUGIN_REGISTER(plugin, 226);
}
