#ifndef DEBOUNCE_H
#define DEBOUNCE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    DEBOUNCE_NONE = 0,    /* no change in the stable state */
    DEBOUNCE_PRESSED,     /* stable state just became pressed */
    DEBOUNCE_RELEASED     /* stable state just became released */
} debounce_event_t;

typedef struct {
    uint32_t debounce_ms;   /* how long a new state must hold still */
    uint32_t since_ms;      /* when the candidate state last changed */
    bool     stable;        /* the debounced state */
    bool     candidate;     /* the latest raw state seen */
} debounce_t;

bool             debounce_init(debounce_t *d, uint32_t debounce_ms, bool initially_pressed);
debounce_event_t debounce_update(debounce_t *d, bool raw_pressed, uint32_t now_ms);
bool             debounce_is_pressed(const debounce_t *d);

#endif
