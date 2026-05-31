CC := cc
PLUGIN_NAME := selectnext.so
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
PLUGIN := $(BUILD_DIR)/$(PLUGIN_NAME)

PLUGIN_SRC := selectnext.c sn_editor.c sn_search.c sn_occurrence.c
PLUGIN_OBJ := $(patsubst %.c,$(OBJ_DIR)/%.o,$(PLUGIN_SRC))


WARN := -Wall -Wextra -Wpedantic -Wconversion -Wshadow
CSTD := -std=c17
CFLAGS ?= $(CSTD) $(WARN) -g
PLUGIN_CFLAGS := $(CFLAGS) -fPIC

GEANY_CFLAGS := $(shell pkg-config --cflags geany)
GEANY_LIBS := $(shell pkg-config --libs geany)

PLUGIN_DIR ?= $(HOME)/.config/geany/plugins

.PHONY: all install uninstall clean dirs

all: $(PLUGIN)

dirs:
	mkdir -p $(OBJ_DIR)

$(PLUGIN): $(PLUGIN_OBJ) | dirs
	$(CC) -shared -o $@ $(PLUGIN_OBJ) $(GEANY_LIBS)

$(OBJ_DIR)/%.o: %.c | dirs
	$(CC) $(PLUGIN_CFLAGS) $(GEANY_CFLAGS) -c $< -o $@

install: $(PLUGIN)
	mkdir -p $(PLUGIN_DIR)
	cp $(PLUGIN) $(PLUGIN_DIR)/$(PLUGIN_NAME)

uninstall:
	rm -f $(PLUGIN_DIR)/$(PLUGIN_NAME)

clean:
	rm -rf $(BUILD_DIR)
