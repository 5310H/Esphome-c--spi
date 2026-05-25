#include "pb_encode.h"
#include "pb_common.h"
#include <string.h>

/* --- Helper functions for writing --- */

static bool buf_write(pb_ostream_t *stream, const uint8_t *buf, size_t count)
{
    uint8_t *dest = (uint8_t*)stream->state;
    if (stream->bytes_written + count > stream->max_size)
        return false;

    memcpy(dest, buf, count);
    stream->state = dest + count;
    return true;
}

pb_ostream_t pb_ostream_from_buffer(uint8_t *buf, size_t bufsize)
{
    pb_ostream_t stream;
    stream.callback = &buf_write;
    stream.state = buf;
    stream.max_size = bufsize;
    stream.bytes_written = 0;
    return stream;
}

bool pb_write(pb_ostream_t *stream, const uint8_t *buf, size_t count)
{
    if (count == 0) return true;
    if (!stream->callback(stream, buf, count)) return false;
    stream->bytes_written += count;
    return true;
}

/* --- Basic type encoders --- */

bool pb_encode_varint(pb_ostream_t *stream, uint64_t value)
{
    uint8_t buffer[10];
    size_t i = 0;

    if (value == 0)
    {
        buffer[i++] = 0;
    }
    else
    {
        while (value)
        {
            buffer[i] = (uint8_t)((value & 0x7F) | 0x80);
            value >>= 7;
            i++;
        }
        buffer[i - 1] &= 0x7F;
    }

    return pb_write(stream, buffer, i);
}

bool pb_encode_svarint(pb_ostream_t *stream, int64_t value)
{
    uint64_t zigzag;
    if (value < 0)
        zigzag = (uint64_t)(~(value << 1));
    else
        zigzag = (uint64_t)(value << 1);

    return pb_encode_varint(stream, zigzag);
}

bool pb_encode_fixed32(pb_ostream_t *stream, const void *value)
{
    uint32_t val; memcpy(&val, value, 4);
    uint8_t bytes[4];
    bytes[0] = (uint8_t)(val & 0xFF);
    bytes[1] = (uint8_t)((val >> 8) & 0xFF);
    bytes[2] = (uint8_t)((val >> 16) & 0xFF);
    bytes[3] = (uint8_t)((val >> 24) & 0xFF);
    return pb_write(stream, bytes, 4);
}

bool pb_encode_fixed64(pb_ostream_t *stream, const void *value)
{
    uint64_t val; memcpy(&val, value, 8);
    uint8_t bytes[8];
    for (int i = 0; i < 8; i++)
        bytes[i] = (uint8_t)((val >> (i * 8)) & 0xFF);
    return pb_write(stream, bytes, 8);
}

bool pb_encode_tag(pb_ostream_t *stream, pb_wire_type_t wiretype, uint32_t field_number)
{
    uint64_t tag = ((uint64_t)field_number << 3) | wiretype;
    return pb_encode_varint(stream, tag);
}

bool pb_encode_string(pb_ostream_t *stream, const uint8_t *buffer, size_t size)
{
    if (!pb_encode_varint(stream, (uint64_t)size))
        return false;

    return pb_write(stream, buffer, size);
}

/* --- Field encoding logic --- */

static bool encode_basic_field(pb_ostream_t *stream, const pb_field_iter_t *iter)
{
    pb_wire_type_t wiretype;
    pb_type_t type = iter->pos->type;

    switch (PB_LTYPE(type))
    {
        case PB_LTYPE_VARINT:
        case PB_LTYPE_UVARINT:
        case PB_LTYPE_SVARINT:
            wiretype = PB_WT_VARINT;
            break;
        case PB_LTYPE_FIXED32:
            wiretype = PB_WT_32BIT;
            break;
        case PB_LTYPE_FIXED64:
            wiretype = PB_WT_64BIT;
            break;
        case PB_LTYPE_BYTES:
        case PB_LTYPE_STRING:
        case PB_LTYPE_SUBMESSAGE:
            wiretype = PB_WT_STRING;
            break;
        default:
            return false;
    }

    /* HTYPE check must come before tag encoding to prevent double tagging */
    if (PB_HTYPE(type) == PB_HTYPE_CALLBACK) {
        pb_callback_t *cb = (pb_callback_t*)iter->pData;
        if (cb && cb->funcs.encode != NULL) {
            return cb->funcs.encode(stream, iter->pos, &cb->arg);
        }
        return true;
    }

    if (!pb_encode_tag(stream, wiretype, iter->pos->tag))
        return false;

    switch (PB_LTYPE(type))
    {
        case PB_LTYPE_VARINT:
            {
                uint64_t val = 0;
                if (iter->pos->data_size == 8) memcpy(&val, iter->pData, 8);
                else if (iter->pos->data_size == 4) { uint32_t v; memcpy(&v, iter->pData, 4); val = v; }
                else if (iter->pos->data_size == 2) { uint16_t v; memcpy(&v, iter->pData, 2); val = v; }
                else if (iter->pos->data_size == 1) { uint8_t v;  memcpy(&v, iter->pData, 1); val = v; }
                return pb_encode_varint(stream, val);
            }
        case PB_LTYPE_SVARINT:
            {
                int64_t val = 0;
                if (iter->pos->data_size == 8) memcpy(&val, iter->pData, 8);
                else if (iter->pos->data_size == 4) { int32_t v; memcpy(&v, iter->pData, 4); val = v; }
                return pb_encode_svarint(stream, val);
            }
        case PB_LTYPE_FIXED32:
            return pb_encode_fixed32(stream, iter->pData);
        case PB_LTYPE_FIXED64:
            return pb_encode_fixed64(stream, iter->pData);
        case PB_LTYPE_BYTES:
        case PB_LTYPE_STRING:
            return false; /* Static strings not implemented */
        default:
            return false;
    }
}

bool pb_encode(pb_ostream_t *stream, const pb_field_t fields[], const void *src_struct)
{
    pb_field_iter_t iter;
    if (!pb_field_iter_begin(&iter, fields, (void*)src_struct))
        return true; /* Empty message */

    do
    {
        /* Extension fields are handled separately */
        if (PB_LTYPE(iter.pos->type) == PB_LTYPE_EXTENSION)
            continue;

        /* For SINGULAR STATIC fields, we encode them directly if present */
        if (!encode_basic_field(stream, &iter))
            return false;

    } while (pb_field_iter_next(&iter));

    return true;
}