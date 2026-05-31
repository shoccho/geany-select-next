#ifndef SN_SEARCH_H
#define SN_SEARCH_H

#include <stdbool.h>
#include <stddef.h>

typedef long SnPos;

typedef struct {
    SnPos start;
    SnPos end;
} SnRange;

bool sn_range_is_valid(SnRange range);
bool sn_range_equals(SnRange a, SnRange b);

bool sn_range_list_contains(
    const SnRange *ranges,
    size_t count,
    SnRange needle
);

bool sn_find_next_unselected_in_text(
    const char *text,
    size_t text_len,
    const char *needle,
    size_t needle_len,
    const SnRange *selected,
    size_t selected_count,
    SnPos from,
    SnPos stop_before,
    bool whole_word,
    bool match_case,
    SnRange *out
);

size_t sn_collect_occurrences_in_text(
    const char *text,
    size_t text_len,
    const char *needle,
    size_t needle_len,
    bool whole_word,
    bool match_case,
    SnRange *out,
    size_t out_cap
);

#endif
