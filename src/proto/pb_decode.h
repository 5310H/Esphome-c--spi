#ifndef PB_DECODE_H_INCLUDED
#define PB_DECODE_H_INCLUDED
#include "pb.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Structure for abstracting the reading of data. */
typedef struct pb_istream_s pb_istream_t;
struct pb_istream_s
{
    bool (*callback)(pb_istream_t *stream, uint8_t *buf, size_t count);
    void *state;
    size_t bytes_left;
};

/* Creation functions */
pb_istream_t pb_istream_from_buffer(const uint8_t *buf, size_t bufsize);

/* Low-level read function */
bool pb_read(pb_istream_t *stream, uint8_t *buf, size_t count);

/* Main entry point for decoding a message */
bool pb_decode(pb_istream_t *stream, const pb_field_t fields[], void *dest_struct);

/* Helper functions for decoding different types */
bool pb_decode_tag(pb_istream_t *stream, pb_wire_type_t *wire_type, uint32_t *tag, bool *eof);
bool pb_decode_varint(pb_istream_t *stream, uint64_t *value);
bool pb_skip_field(pb_istream_t *stream, pb_wire_type_t wire_type);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif