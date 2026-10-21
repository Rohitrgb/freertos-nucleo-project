#ifndef CMD_PARSER_H
#define CMD_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CMD_LINE_MAX 64U   /* includes the terminating NUL: 63 usable characters */

typedef void (*cmd_handler_t)(const char *arg);

typedef struct {
    const char    *name;
    cmd_handler_t  handler;
} cmd_entry_t;

typedef enum {
    CMD_NONE = 0,       /* nothing to report (line incomplete, or a blank line) */
    CMD_OK,             /* command found and its handler ran */
    CMD_ERR_UNKNOWN,    /* complete line, but no such command */
    CMD_ERR_TOO_LONG,   /* line longer than CMD_LINE_MAX - 1 characters */
    CMD_ERR_BAD_CHAR    /* line contained a non-printable byte */
} cmd_result_t;

typedef struct {
    const cmd_entry_t *table;
    size_t             table_len;
    char               line[CMD_LINE_MAX];
    size_t             len;
    cmd_result_t       error;   /* first problem seen on the current line */
} cmd_parser_t;

bool         cmd_parser_init(cmd_parser_t *p, const cmd_entry_t *table, size_t table_len);
cmd_result_t cmd_parser_feed(cmd_parser_t *p, uint8_t byte);

#endif
