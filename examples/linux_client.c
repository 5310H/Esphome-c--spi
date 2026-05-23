#include "esphome_api.h"
#include <stdio.h>

int main() {
    printf("ESPHome C API - Linux Example\n");

    // Replace with your device IP and PSK
    const char *host = "192.168.1.50";
    uint16_t port = 6053;
    const char *psk = "my_psk";

    esph_session_t *s = esph_connect(host, port, psk);

    if (!s) {
        printf("Failed to connect to ESPHome device\n");
        return 1;
    }

    printf("Connected. Subscribing to states...\n");
    esph_subscribe_states(s);

    printf("Turning on switch.relay_1...\n");
    esph_set_switch(s, "switch.relay_1", 1);

    printf("Disconnecting...\n");
    esph_disconnect(s);

    return 0;
}
