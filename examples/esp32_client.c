#include "esphome_api.h"
#include "esp_log.h"

static const char *TAG = "ESPHomeClient";

void app_main() {
    ESP_LOGI(TAG, "ESPHome C API - ESP32 Example");

    const char *host = "192.168.1.50";
    uint16_t port = 6053;
    const char *psk = "my_psk";

    esph_session_t *s = esph_connect(host, port, psk);

    if (!s) {
        ESP_LOGE(TAG, "Failed to connect to ESPHome device");
        return;
    }

    ESP_LOGI(TAG, "Connected. Subscribing to states...");
    esph_subscribe_states(s);

    ESP_LOGI(TAG, "Turning on switch.relay_1...");
    esph_set_switch(s, "switch.relay_1", 1);

    ESP_LOGI(TAG, "Disconnecting...");
    esph_disconnect(s);
}
