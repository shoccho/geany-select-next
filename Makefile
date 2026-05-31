CC := cc
PLUGIN := selectnext.so
SRC := selectnext.c

CFLAGS := -std=c17 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -fPIC -g
GEANY_CFLAGS := $(shell pkg-config --cflags geany)
GEANY_LIBS := $(shell pkg-config --libs geany)

PLUGIN_DIR := $(HOME)/.config/geany/plugins

all: $(PLUGIN)

$(PLUGIN): $(SRC)
	$(CC) $(CFLAGS) $(GEANY_CFLAGS) -shared -o $@ $< $(GEANY_LIBS)

install: $(PLUGIN)
	mkdir -p $(PLUGIN_DIR)
	cp $(PLUGIN) $(PLUGIN_DIR)/$(PLUGIN)

clean:
	rm -f $(PLUGIN)
