#include "pb_decode.h"
#include "pb_common.h"
#include <string.h>

/* --- Helper functions for reading --- */

static bool buf_read(pb_istream_t *stream, uint8_t *buf, size_t count)
{
    const uint8_t *src = (const uint8_t *)stream->state;
    if (buf != NULL)
        memcpy(buf, src, count);
    
    stream->state = (void *)(src + count);
    return true;
}

pb_istream_t pb_istream_from_buffer(const uint8_t *buf, size_t bufsize)
{
    pb_istream_t stream;
    stream.callback = &buf_read;
    stream.state = (void *)buf;
    stream.bytes_left = bufsize;
    return stream;
}

bool pb_read(pb_istream_t *stream, uint8_t *buf, size_t count)
{
    if (count > stream->bytes_left)
        return false;
    
    if (!stream->callback(stream, buf, count))
        return false;
    
    stream->bytes_left -= count;
    return true;
}

/* --- Basic type decoders --- */

bool pb_decode_varint(pb_istream_t *stream, uint64_t *value)
{
    uint8_t byte;
    int bitpos = 0;
    *value = 0;
    
    while (bitpos < 64 && pb_read(stream, &byte, 1))
    {
        *value |= (uint64_t)(byte & 0x7F) << bitpos;
        if (!(byte & 0x80))
            return true;
        bitpos += 7;
    }
    
    return false;
}

bool pb_decode_fixed32(pb_istream_t *stream, void *dest)
{
    uint8_t bytes[4];
    if (!pb_read(stream, bytes, 4)) return false;
    
    uint32_t value = ((uint32_t)bytes[0] << 0) |
                     ((uint32_t)bytes[1] << 8) |
                     ((uint32_t)bytes[2] << 16) |
                     ((uint32_t)bytes[3] << 24);
    
    memcpy(dest, &value, 4);
    return true;
}

bool pb_decode_fixed64(pb_istream_t *stream, void *dest)
{
    uint8_t bytes[8];
    if (!pb_read(stream, bytes, 8)) return false;
    
    uint64_t value = 0;
    for (int i = 0; i < 8; i++) {
        value |= (uint64_t)bytes[i] << (i * 8);
    }
    
    memcpy(dest, &value, 8);
    return true;
}

bool pb_decode_tag(pb_istream_t *stream, pb_wire_type_t *wire_type, uint32_t *tag, bool *eof)
{
    uint64_t temp;
    *eof = false;
    
    if (stream->bytes_left == 0)
    {
        *eof = true;
        return false;
    }
    
    if (!pb_decode_varint(stream, &temp))
        return false;
    
    *tag = (uint32_t)(temp >> 3);
    *wire_type = (pb_wire_type_t)(temp & 7);
    return true;
}

bool pb_skip_field(pb_istream_t *stream, pb_wire_type_t wire_type)
{
    switch (wire_type)
    {
        case PB_WT_VARINT: { uint64_t temp; return pb_decode_varint(stream, &temp); }
        case PB_WT_64BIT:  return pb_read(stream, NULL, 8);
        case PB_WT_STRING: { uint64_t len; if (!pb_decode_varint(stream, &len)) return false; return pb_read(stream, NULL, (size_t)len); }
        case PB_WT_32BIT:  return pb_read(stream, NULL, 4);
        default: return false;
    }
}

bool pb_decode(pb_istream_t *stream, const pb_field_t fields[], void *dest_struct)
{
    pb_field_iter_t iter;
    if (fields == NULL || !pb_field_iter_begin(&iter, fields, dest_struct))
        return true;
    
    while (stream->bytes_left)
    {
        uint32_t tag;
        pb_wire_type_t wire_type;
        bool eof;
        
        if (!pb_decode_tag(stream, &wire_type, &tag, &eof))
            return eof;
        
        if (pb_field_iter_find(&iter, tag))
        {
            pb_type_t type = iter.pos->type;
            if (PB_HTYPE(type) == PB_HTYPE_CALLBACK)
            {
                pb_callback_t *cb = (pb_callback_t*)iter.pData;
                if (cb->funcs.decode)
                {
                    if (!cb->funcs.decode(stream, iter.pos, &cb->arg))
                        return false;
                }
                else
                {
                    if (!pb_skip_field(stream, wire_type)) return false;
                }
            }
            else
            {
                if (PB_LTYPE(type) == PB_LTYPE_VARINT)
                {
                    uint64_t val;
                    if (!pb_decode_varint(stream, &val)) return false;
                    if (iter.pos->data_size == 8) memcpy(iter.pData, &val, 8);
                    else if (iter.pos->data_size == 4) { uint32_t v = (uint32_t)val; memcpy(iter.pData, &v, 4); }
                    else if (iter.pos->data_size == 2) { uint16_t v = (uint16_t)val; memcpy(iter.pData, &v, 2); }
                    else if (iter.pos->data_size == 1) { uint8_t v = (uint8_t)val;  memcpy(iter.pData, &v, 1); }
                }
                else if (PB_LTYPE(type) == PB_LTYPE_FIXED32)
                {
                    if (!pb_decode_fixed32(stream, iter.pData)) return false;
                }
                else if (PB_LTYPE(type) == PB_LTYPE_FIXED64)
                {
                    if (!pb_decode_fixed64(stream, iter.pData)) return false;
                }
                else
                {
                    if (!pb_skip_field(stream, wire_type)) return false;
                }
            }
        }
        else
        {
            if (!pb_skip_field(stream, wire_type)) return false;
        }
    }
    
    return true;
}