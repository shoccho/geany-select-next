#include <geanyplugin.h>

#include "sn_occurrence.h"

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

static GeanyKeyGroup *key_group = NULL;

static gboolean selectnext_init(GeanyPlugin *plugin, gpointer pdata) {
    (void)pdata;

    geany_plugin = plugin;
    geany_data = plugin->geany_data;

    key_group = plugin_set_key_group(plugin, "selectnext", KB_COUNT, NULL);

    keybindings_set_item(
        key_group,
        KB_ADD_NEXT,
        sn_occurrence_add_next,
        GDK_KEY_d,
        GEANY_PRIMARY_MOD_MASK,
        "add_next_occurrence",
        "Add Next Occurrence",
        NULL
    );

    keybindings_set_item(
        key_group,
        KB_SELECT_ALL,
        sn_occurrence_select_all,
        GDK_KEY_d,
        GEANY_PRIMARY_MOD_MASK | GDK_SHIFT_MASK,
        "select_all_occurrences",
        "Select All Occurrences",
        NULL
    );

    keybindings_set_item(
        key_group,
        KB_SKIP_CURRENT,
        sn_occurrence_skip_current,
        GDK_KEY_d,
        GDK_MOD1_MASK,
        "skip_current_occurrence",
        "Skip Current Occurrence",
        NULL
    );

    keybindings_set_item(
        key_group,
        KB_UNDO_LAST,
        sn_occurrence_undo_last,
        GDK_KEY_d,
        GEANY_PRIMARY_MOD_MASK | GDK_MOD1_MASK,
        "undo_last_occurrence",
        "Undo Last Occurrence",
        NULL
    );

    keybindings_set_item(
        key_group,
        KB_CLEAR,
        sn_occurrence_clear,
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
    plugin->info->version = "0.3";
    plugin->info->author = "Eftakharul Islam";

    plugin->funcs->init = selectnext_init;
    plugin->funcs->cleanup = selectnext_cleanup;

    GEANY_PLUGIN_REGISTER(plugin, 226);
}
