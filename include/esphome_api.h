#ifndef ESPHOME_API_H
#define ESPHOME_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration of session struct (opaque)
typedef struct esph_session esph_session_t;

/**
 * Establish a full ESPHome encrypted API session.
 *
 * Steps performed:
 *   1. TCP connect
 *   2. Noise NNpsk0 handshake
 *   3. Session object returned on success
 *
 * @param host  Hostname or IP of the ESPHome device
 * @param port  TCP port (default 6053)
 * @param psk   Pre-shared key (string form)
 * @return session pointer on success, NULL on failure
 */
esph_session_t *esph_connect(const char *host, uint16_t port, const char *psk);

/**
 * Close an ESPHome session and free resources.
 *
 * @param s  Session pointer
 * @return 0 on success, <0 on failure
 */
int esph_disconnect(esph_session_t *s);

/**
 * Subscribe to state updates from the ESPHome node.
 *
 * Placeholder implementation until protobuf is added.
 *
 * @param s  Active session
 * @return 0 on success, <0 on failure
 */
int esph_subscribe_states(esph_session_t *s);

/**
 * Send a switch command to an entity.
 *
 * Placeholder implementation until protobuf is added.
 *
 * @param s          Active session
 * @param entity_id  ESPHome entity ID (e.g. "switch.lamp")
 * @param state      1 = ON, 0 = OFF
 * @return 0 on success, <0 on failure
 */
int esph_set_switch(esph_session_t *s, const char *entity_id, int state);

#ifdef __cplusplus
}
#endif

#endif // ESPHOME_API_H
