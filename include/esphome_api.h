#ifndef ESPHOME_API_H
#define ESPHOME_API_H

#include <stdint.h>
#include <stddef.h>

typedef struct esph_session esph_session_t;

// Connect to an ESPHome device using encrypted API
esph_session_t *esph_connect(const char *host, uint16_t port, const char *psk);

// Disconnect and free resources
int esph_disconnect(esph_session_t *s);

// Subscribe to state updates (sensors, switches, etc.)
int esph_subscribe_states(esph_session_t *s);

// Set a switch entity on/off
int esph_set_switch(esph_session_t *s, const char *entity_id, int state);

#endif
