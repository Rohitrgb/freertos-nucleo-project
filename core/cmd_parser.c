#include "cmd_parser.h"
#include <string.h>

bool cmd_parser_init(cmd_parser_t *p, const cmd_entry_t *table, size_t table_len)
{
    if (p == NULL || table == NULL || table_len == 0U) {
        return false;
    }
    p->table     = table;
    p->table_len = table_len;
    p->len       = 0U;
    p->error     = CMD_NONE;
    return true;
}

/* Called with a complete, NUL-terminated line in p->line. */
static cmd_result_t dispatch(cmd_parser_t *p)
{
    char  *name = p->line;
    char  *arg;
    size_t n;

    while (*name == ' ') { name++; }          /* skip leading spaces */
    if (*name == '\0') {
        return CMD_NONE;                      /* line was only spaces */
    }

    arg = name;
    while (*arg != '\0' && *arg != ' ') { arg++; }
    if (*arg != '\0') {
        *arg = '\0';                          /* end the command word here */
        arg++;
        while (*arg == ' ') { arg++; }        /* skip spaces before the argument */
    }

    n = strlen(arg);
    while (n > 0U && arg[n - 1U] == ' ') {    /* trim trailing spaces */
        n--;
        arg[n] = '\0';
    }

    for (size_t i = 0U; i < p->table_len; i++) {
        if (strcmp(name, p->table[i].name) == 0) {
            p->table[i].handler(arg);
            return CMD_OK;
        }
    }
    return CMD_ERR_UNKNOWN;
}

cmd_result_t cmd_parser_feed(cmd_parser_t *p, uint8_t byte)
{
    cmd_result_t result = CMD_NONE;

    if (byte == '\r' || byte == '\n') {       /* end of line */
        if (p->error != CMD_NONE) {
            result = p->error;                /* report the problem... */
        } else if (p->len > 0U) {
            p->line[p->len] = '\0';
            result = dispatch(p);
        }
        p->len   = 0U;                        /* ...then start a fresh line */
        p->error = CMD_NONE;
        return result;
    }

    if (byte < 0x20U || byte > 0x7EU) {       /* not printable ASCII */
        if (p->error == CMD_NONE) { p->error = CMD_ERR_BAD_CHAR; }
        return CMD_NONE;
    }

    if (p->len >= (CMD_LINE_MAX - 1U)) {      /* no room: remember, don't store */
        if (p->error == CMD_NONE) { p->error = CMD_ERR_TOO_LONG; }
        return CMD_NONE;
    }

    p->line[p->len] = (char)byte;
    p->len++;
    return CMD_NONE;
}
