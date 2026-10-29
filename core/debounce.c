#include "debounce.h"
#include <stddef.h>

bool debounce_init(debounce_t *d, uint32_t debounce_ms, bool initially_pressed)
{
    if (d == NULL) {
        return false;
    }
    d->debounce_ms = debounce_ms;
    d->since_ms    = 0U;
    d->stable      = initially_pressed;
    d->candidate   = initially_pressed;
    return true;
}

bool debounce_is_pressed(const debounce_t *d)
{
    return d->stable;
}

debounce_event_t debounce_update(debounce_t *d, bool raw_pressed, uint32_t now_ms)
{
    if (raw_pressed != d->candidate) {      /* something changed: restart the clock */
        d->candidate = raw_pressed;
        d->since_ms  = now_ms;
    }

    if (d->candidate != d->stable &&
        (uint32_t)(now_ms - d->since_ms) >= d->debounce_ms) {
        d->stable = d->candidate;           /* it held still long enough */
        return d->stable ? DEBOUNCE_PRESSED : DEBOUNCE_RELEASED;
    }
    return DEBOUNCE_NONE;
}
