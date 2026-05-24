#include "esphome_transport.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

int esph_transport_connect(const char *host, uint16_t port) {
    int sock = -1;
    struct addrinfo hints;
    struct addrinfo *res = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%u", port);

    int err = getaddrinfo(host, port_str, &hints, &res);
    if (err != 0) {
        fprintf(stderr, "[TRANSPORT] getaddrinfo failed: %s\n", gai_strerror(err));
        return -1;
    }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        perror("[TRANSPORT] socket");
        freeaddrinfo(res);
        return -1;
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        perror("[TRANSPORT] connect");
        close(sock);
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    return sock;
}

int esph_transport_send(int sock, const uint8_t *buf, int len) {
    int sent = send(sock, buf, len, 0);
    if (sent < 0) {
        perror("[TRANSPORT] send");
    }
    return sent;
}

int esph_transport_recv(int sock, uint8_t *buf, int maxlen) {
    int r = recv(sock, buf, maxlen, 0);
    if (r < 0) {
        perror("[TRANSPORT] recv");
    }
    return r;
}
