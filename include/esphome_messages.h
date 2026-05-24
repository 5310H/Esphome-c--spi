#ifndef ESPHOME_MESSAGES_H
#define ESPHOME_MESSAGES_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
// Message types used by the ESPHome API.
// These match the placeholder values in messages.c.
// Later you can expand this enum to include real ESPHome protobuf types.
// ---------------------------------------------------------------------------
typedef enum {
    ESPH_MSG_SUBSCRIBE_STATES = 1,
    ESPH_MSG_SWITCH_COMMAND   = 2,
    // Add more message types here as needed
} esph_msg_type_t;

/**
 * Encode a message into a protobuf frame.
 *
 * Placeholder implementation:
 *   - Writes type byte
 *   - Writes 2-byte placeholder length
 *   - Returns 3
 *
 * @param type     Message type enum
 * @param payload  Pointer to message-specific struct (unused for now)
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
