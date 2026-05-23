#ifndef ESPHOME_TRANSPORT_H
#define ESPHOME_TRANSPORT_H

#include <stdint.h>

// Open a TCP connection to an ESPHome device
int esph_transport_connect(const char *host, uint16_t port);

// Send raw bytes over the socket
int esph_transport_send(int sock, const uint8_t *buf, int len);

// Receive raw bytes from the socket
int esph_transport_recv(int sock, uint8_t *buf, int maxlen);

#endif
