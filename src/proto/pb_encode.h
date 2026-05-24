#ifndef PB_ENCODE_H_INCLUDED
#define PB_ENCODE_H_INCLUDED
#include "pb.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Structure for abstracting the writing of data.
 * User can provide a custom callback to write to a socket, file, or buffer.
 */
typedef struct pb_ostream_s pb_ostream_t;
struct pb_ostream_s
{
    bool (*callback)(pb_ostream_t *stream, const uint8_t *buf, size_t count);
    void *state;          /* Free-form state for the callback */
    size_t max_size;      /* Max bytes allowed to be written */
    size_t bytes_written; /* Number of bytes written so far */
};

/* Creation functions */
pb_ostream_t pb_ostream_from_buffer(uint8_t *buf, size_t bufsize);

/* Low-level write function */
bool pb_write(pb_ostream_t *stream, const uint8_t *buf, size_t count);

/* Main entry point for encoding a message */
bool pb_encode(pb_ostream_t *stream, const pb_field_t fields[], const void *src_struct);

/* Submessage encoding (handles size prefixing) */
bool pb_encode_submessage(pb_ostream_t *stream, const pb_field_t fields[], const void *src_struct);

/* Helper functions for encoding different types */
bool pb_encode_tag(pb_ostream_t *stream, pb_wire_type_t wiretype, uint32_t field_number);
bool pb_encode_tag_for_field(pb_ostream_t *stream, const pb_field_iter_t *field);
bool pb_encode_varint(pb_ostream_t *stream, uint64_t value);
bool pb_encode_svarint(pb_ostream_t *stream, int64_t value);
bool pb_encode_fixed32(pb_ostream_t *stream, const void *value);
bool pb_encode_fixed64(pb_ostream_t *stream, const void *value);
bool pb_encode_string(pb_ostream_t *stream, const uint8_t *buffer, size_t size);

/* Extension support */
bool pb_encode_extension(pb_ostream_t *stream, const pb_field_t *field, const void *pData);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif