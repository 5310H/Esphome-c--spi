#ifndef ESPHOME_MESSAGES_H
#define ESPHOME_MESSAGES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
// Message types used by the ESPHome API.
// These correspond to the message tags in the ESPHome API protocol.
// ---------------------------------------------------------------------------
typedef enum {
    ESPH_MSG_HELLO_REQUEST           = 1,
    ESPH_MSG_HELLO_RESPONSE          = 2,
    ESPH_MSG_DEVICE_INFO_REQUEST     = 9,
    ESPH_MSG_DEVICE_INFO_RESPONSE    = 10,
    ESPH_MSG_SUBSCRIBE_STATES_REQUEST = 20,
    ESPH_MSG_SWITCH_COMMAND_REQUEST  = 26,
} esph_msg_type_t;

/**
 * Encode a message into a protobuf frame.
 *
 * @param type     Message type enum
 * @param payload  Pointer to the Nanopb-generated struct for this message type
 * @param out      Output buffer
 * @param maxlen   Maximum bytes available in output buffer
 * @return number of bytes written, <0 on failure
 */
int esph_encode_message(esph_msg_type_t type,
                        const void *payload,
                        uint8_t *out, int maxlen);

/**
 * Decode a protobuf frame.
 *
 * Placeholder implementation:
 *   - Currently does nothing
 *
 * @param buf   Input buffer
 * @param len   Length of input buffer
 * @return 0 on success, <0 on failure
 */
int esph_decode_message(const uint8_t *buf, int len);

#ifdef __cplusplus
}
#endif

#endif // ESPHOME_MESSAGES_H
