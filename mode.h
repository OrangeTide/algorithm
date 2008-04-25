#ifndef MODE_H
#define MODE_H
struct mode_parser {
    const char *mode_curr;
    int plus_or_minus; /* 1 = plus, 0 = minus */
    const char *arg_curr;
};

void parse_mode_begin(struct mode_parser *st, const char *fulltext);
int parse_mode_next(struct mode_parser *st, char mode_flag [2 ], char *arg, size_t arglen);
#endif
