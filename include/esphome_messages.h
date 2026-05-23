#ifndef ESPHOME_MESSAGES_H
#define ESPHOME_MESSAGES_H

#include <stdint.h>

// ESPHome message type IDs (subset for now)
typedef enum {
    ESPH_MSG_HELLO = 1,
    ESPH_MSG_CONNECT = 2,
    ESPH_MSG_LIST_ENTITIES = 3,
    ESPH_MSG_SUBSCRIBE_STATES = 4,
    ESPH_MSG_SWITCH_COMMAND = 10,
} esph_msg_type_t;

// Encode a protobuf message into a framed ESPHome packet
int esph_encode_message(esph_msg_type_t type,
                        const void *payload,
                        uint8_t *out, int maxlen);

// Decode a received ESPHome packet
int esph_decode_message(const uint8_t *buf, int len);

#endif
