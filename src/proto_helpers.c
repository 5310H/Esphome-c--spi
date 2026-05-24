#include <unistd.h>
#include "pb_encode.h"
#include "esphome_api.pb.h"

bool send_message(int sock, uint32_t type, const void *msg_struct, const pb_msgdesc_t *fields) {
    uint8_t buffer[1024];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer + 6, sizeof(buffer) - 6);

    if (!pb_encode(&stream, fields, msg_struct))
        return false;

    uint32_t len = stream.bytes_written;

    buffer[0] = (type >> 24) & 0xFF;
    buffer[1] = (type >> 16) & 0xFF;
    buffer[2] = (type >> 8) & 0xFF;
    buffer[3] = (type) & 0xFF;

    buffer[4] = (len >> 8) & 0xFF;
    buffer[5] = (len) & 0xFF;

    write(sock, buffer, len + 6);
    return true;
}
