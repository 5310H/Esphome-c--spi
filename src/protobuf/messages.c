#include "esphome_messages.h"
#include <string.h>

// Placeholder protobuf framing.
// Real nanopb-based encoding/decoding will be added later.

int esph_encode_message(esph_msg_type_t type,
                        const void *payload,
                        uint8_t *out, int maxlen) {
    (void)payload;

    // TODO: replace with real protobuf encoding
    if (maxlen < 3)
        return -1;

    out[0] = (uint8_t)type;
    out[1] = 0x00; // placeholder length
    out[2] = 0x00;

    return 3; // placeholder frame size
}

int esph_decode_message(const uint8_t *buf, int len) {
    (void)buf;
    (void)len;

    // TODO: replace with real protobuf decoding
    return 0;
}
