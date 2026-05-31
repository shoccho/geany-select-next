#include "sn_search.h"

#include <ctype.h>
#include <string.h>

bool sn_range_is_valid(SnRange range) {
    return range.start >= 0 && range.end >= range.start;
}

bool sn_range_equals(SnRange a, SnRange b) {
    return a.start == b.start && a.end == b.end;
}

bool sn_range_list_contains(
    const SnRange *ranges,
    size_t count,
    SnRange needle
) {
    if (ranges == NULL && count != 0) {
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        if (sn_range_equals(ranges[i], needle)) {
            return true;
        }
    }

    return false;
}

static bool is_word_char(unsigned char c) {
    return isalnum(c) || c == '_';
}

static bool range_has_word_boundaries(
    const char *text,
    size_t text_len,
    size_t start,
    size_t end
) {
    bool left_ok = start == 0 || !is_word_char((unsigned char)text[start - 1]);
    bool right_ok = end >= text_len || !is_word_char((unsigned char)text[end]);

    return left_ok && right_ok;
}

static int lower_byte(unsigned char c) {
    return tolower(c);
}

static bool bytes_match(
    const char *a,
    const char *b,
    size_t len,
    bool match_case
) {
    if (match_case) {
        return memcmp(a, b, len) == 0;
    }

    for (size_t i = 0; i < len; i++) {
        if (lower_byte((unsigned char)a[i]) != lower_byte((unsigned char)b[i])) {
            return false;
        }
    }

    return true;
}

static bool find_in_range(
    const char *text,
    size_t text_len,
    const char *needle,
    size_t needle_len,
    size_t from,
    size_t to,
    bool whole_word,
    bool match_case,
    SnRange *out
) {
    if (text == NULL || needle == NULL || out == NULL) {
        return false;
    }

    if (needle_len == 0 || text_len < needle_len) {
        return false;
    }

    if (from > text_len) {
        from = text_len;
    }

    if (to > text_len) {
        to = text_len;
    }

    if (from > to || to - from < needle_len) {
        return false;
    }

    size_t last = to - needle_len;

    for (size_t i = from; i <= last; i++) {
        size_t end = i + needle_len;

        if (!bytes_match(text + i, needle, needle_len, match_case)) {
            continue;
        }

        if (whole_word && !range_has_word_boundaries(text, text_len, i, end)) {
            continue;
        }

        out->start = (SnPos)i;
        out->end = (SnPos)end;
        return true;
    }

    return false;
}

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
) {
    if (out == NULL) {
        return false;
    }

    if (from < 0) {
        from = 0;
    }

    if (stop_before < 0) {
        stop_before = 0;
    }

    if ((size_t)from > text_len) {
        from = (SnPos)text_len;
    }

    if ((size_t)stop_before > text_len) {
        stop_before = (SnPos)text_len;
    }

    size_t start = (size_t)from;

    while (start < text_len) {
        SnRange found = {0, 0};

        if (!find_in_range(
                text,
                text_len,
                needle,
                needle_len,
                start,
                text_len,
                whole_word,
                match_case,
                &found
            )) {
            break;
        }

        if (!sn_range_list_contains(selected, selected_count, found)) {
            *out = found;
            return true;
        }

        start = (size_t)found.end;
    }

    start = 0;
    size_t stop = (size_t)stop_before;

    while (start < stop) {
        SnRange found = {0, 0};

        if (!find_in_range(
                text,
                text_len,
                needle,
                needle_len,
                start,
                stop,
                whole_word,
                match_case,
                &found
            )) {
            break;
        }

        if (!sn_range_list_contains(selected, selected_count, found)) {
            *out = found;
            return true;
        }

        start = (size_t)found.end;
    }

    return false;
}

size_t sn_collect_occurrences_in_text(
    const char *text,
    size_t text_len,
    const char *needle,
    size_t needle_len,
    bool whole_word,
    bool match_case,
    SnRange *out,
    size_t out_cap
) {
    if (text == NULL || needle == NULL || needle_len == 0) {
        return 0;
    }

    size_t count = 0;
    size_t start = 0;

    while (start < text_len) {
        SnRange found = {0, 0};

        if (!find_in_range(
                text,
                text_len,
                needle,
                needle_len,
                start,
                text_len,
                whole_word,
                match_case,
                &found
            )) {
            break;
        }

        if (out != NULL && count < out_cap) {
            out[count] = found;
        }

        count++;
        start = (size_t)found.end;
    }

    return count;
}
