#include "esphome_transport.h"
#include <string.h>

// Placeholder transport layer.
// Real TCP sockets will be added for Linux + ESP32.

int esph_transport_connect(const char *host, uint16_t port) {
    (void)host;
    (void)port;
    // TODO: implement TCP connect
    return -1; // placeholder: no real socket yet
}

int esph_transport_send(int sock, const uint8_t *buf, int len) {
    (void)sock;
    (void)buf;
    (void)len;
    // TODO: implement send()
    return -1;
}

int esph_transport_recv(int sock, uint8_t *buf, int maxlen) {
    (void)sock;
    (void)buf;
    (void)maxlen;
    // TODO: implement recv()
    return -1;
}
