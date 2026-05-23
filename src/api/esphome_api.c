#include "esphome_api.h"
#include "esphome_noise.h"
#include "esphome_transport.h"
#include "esphome_messages.h"

#include <stdlib.h>
#include <string.h>

struct esph_session {
    int sock;
    esph_noise_ctx_t noise;
};

esph_session_t *esph_connect(const char *host, uint16_t port, const char *psk) {
    esph_session_t *s = calloc(1, sizeof(*s));
    if (!s)
        return NULL;

    s->sock = esph_transport_connect(host, port);
    if (s->sock < 0) {
        free(s);
        return NULL;
    }

    esph_noise_init(&s->noise, psk);

    if (esph_noise_handshake(&s->noise, s->sock) != 0) {
        free(s);
        return NULL;
    }

    return s;
}

int esph_disconnect(esph_session_t *s) {
    if (!s)
        return -1;

    // TODO: close socket
    free(s);
    return 0;
}

int esph_subscribe_states(esph_session_t *s) {
    if (!s)
        return -1;

    uint8_t frame[32];
    int len = esph_encode_message(ESPH_MSG_SUBSCRIBE_STATES, NULL, frame, sizeof(frame));
    if (len < 0)
        return -1;

    return esph_transport_send(s->sock, frame, len);
}

int esph_set_switch(esph_session_t *s, const char *entity_id, int state) {
    if (!s)
        return -1;

    uint8_t frame[64];
    // TODO: real protobuf payload
    (void)entity_id;
    (void)state;

    int len = esph_encode_message(ESPH_MSG_SWITCH_COMMAND, NULL, frame, sizeof(frame));
    if (len < 0)
        return -1;

    return esph_transport_send(s->sock, frame, len);
}
