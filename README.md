# Select Next Occurrence for Geany

A small Geany plugin that adds VS Code/Sublime-style occurrence selection and basic multi-cursor editing to Geany.

This plugin is intentionally lightweight. It keeps Geany's fast editor feel while adding the editing command many people miss most: `Ctrl+D` to select the next occurrence.

## Features

- `Ctrl+D`: select current word, then add the next occurrence each time you press it
- `Ctrl+Shift+D`: select all matching occurrences
- `Alt+D`: skip the current occurrence and move to the next one
- `Ctrl+Alt+D`: undo/drop the current added occurrence
- Optional command for clearing multiple selections
- Real Scintilla multiple selections/carets
- Case-sensitive matching
- Whole-word matching when the original selection is a whole word
- Build output isolated under `build/`

## Requirements

You need Geany development headers and `pkg-config`.

### Debian / Ubuntu / Linux Mint

```sh
sudo apt update
sudo apt install build-essential pkg-config geany libgtk-3-dev libglib2.0-dev
```

If you compiled Geany from source into `/usr/local`, make sure this works:

```sh
pkg-config --cflags geany
pkg-config --libs geany
```

## Project layout

```text
.
├── Makefile
├── README.md
├── selectnext.c        # Geany plugin entry point and keybinding registration
├── sn_editor.c         # Thin Scintilla/Geany editor wrapper
├── sn_editor.h
├── sn_occurrence.c     # Plugin command behavior
├── sn_occurrence.h
├── sn_search.c         # Pure occurrence-search logic
├── sn_search.h
```

Generated files go here:

```text
build/
├── obj/
└── selectnext.so
```

## Build

```sh
make
```

The plugin will be created at:

```text
build/selectnext.so
```

## Install

```sh
make install
```

By default, this copies the plugin to:

```text
~/.config/geany/plugins/selectnext.so
```

Then restart Geany and enable it:

```text
Tools → Plugin Manager → Select Next Occurrence
```

## Uninstall

```sh
make uninstall
```


## Keybindings

After enabling the plugin, open:

```text
Edit → Preferences → Keybindings → Select Next Occurrence
```

Default bindings:

| Action | Default binding |
|---|---|
| Add Next Occurrence | `Ctrl+D` |
| Select All Occurrences | `Ctrl+Shift+D` |
| Skip Current Occurrence | `Alt+D` |
| Undo Last Occurrence | `Ctrl+Alt+D` |
| Clear Multiple Selections | unbound |

Geany may already bind `Ctrl+D` to another command such as duplicate line. If so, clear the old binding and assign `Ctrl+D` to **Add Next Occurrence**.

## Behavior

### Add Next Occurrence

If there is no selection, the plugin selects the word under the caret.

If there is already a selection, it searches for the next unselected matching occurrence and adds it as another selection/caret.

When the end of the file is reached, search wraps to the top.

### Select All Occurrences

If there is no selection, the plugin first selects the word under the caret. Then it selects every occurrence in the document.

### Skip Current Occurrence

When you selected one occurrence but do not want to edit it, press `Alt+D`. The current occurrence is skipped and the next occurrence is selected instead.

### Undo Last Occurrence

Drops the current/main selection if multiple selections exist.

## Matching rules

Current behavior is deliberately simple:

- matching is case-sensitive
- if the original selection is a whole word, only whole-word matches are selected
- word characters are ASCII letters, digits, and `_`

This is good enough for C identifiers and most source-code editing.

## Development

Useful commands:

```sh
make clean
make
make install
```

For stricter local development, you can override `CFLAGS`:

```sh
make clean
make CFLAGS='-std=c17 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror -g'
```

## Troubleshooting

### `Package geany was not found in the pkg-config search path`

Install Geany development files or make sure your source-built Geany installed `geany.pc` somewhere visible to `pkg-config`.

Check:

```sh
pkg-config --modversion geany
```

If Geany is installed under `/usr/local`, you may need:

```sh
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
```

### Plugin does not appear in Plugin Manager

Check that the file exists:

```sh
ls ~/.config/geany/plugins/selectnext.so
```

Then launch Geany from a terminal and look for plugin loading errors:

```sh
geany -v
```

### `Ctrl+D` does not work

Go to:

```text
Edit → Preferences → Keybindings
```

Find conflicting `Ctrl+D` bindings and clear them. Then assign `Ctrl+D` to:

```text
Select Next Occurrence → Add Next Occurrence
```

## Status

This is a small personal plugin. It is meant to be understandable C code, not a giant editor framework.


## License

MIT License is recommended, but no license file is included yet. Add one before publishing publicly if you want others to reuse the code.
