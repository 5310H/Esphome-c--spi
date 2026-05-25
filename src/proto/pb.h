#ifndef PB_H_INCLUDED
#define PB_H_INCLUDED

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define PB_PROTO_HEADER_VERSION 30

typedef uint8_t pb_type_t;

/* Wire types */
typedef enum {
    PB_WT_VARINT = 0,
    PB_WT_64BIT  = 1,
    PB_WT_STRING = 2,
    PB_WT_32BIT  = 5
} pb_wire_type_t;

/* Field allocation types */
#define PB_HTYPE_REQUIRED 0x00
#define PB_HTYPE_OPTIONAL 0x10
#define PB_HTYPE_REPEATED 0x20
#define PB_HTYPE_CALLBACK 0x30
#define PB_HTYPE_STATIC   0x00
#define PB_HTYPE(x) ((x) & 0xF0)

/* Field data types */
#define PB_LTYPE_VARINT  0x00
#define PB_LTYPE_UVARINT 0x01
#define PB_LTYPE_SVARINT 0x02
#define PB_LTYPE_FIXED32 0x03
#define PB_LTYPE_FIXED64 0x04
#define PB_LTYPE_BYTES   0x05
#define PB_LTYPE_STRING  0x06
#define PB_LTYPE_SUBMESSAGE 0x07
#define PB_LTYPE_EXTENSION 0x08
#define PB_LTYPE(x) ((x) & 0x0F)

/* Type mappings for generated code */
#define PB_LTYPE_UINT32 PB_LTYPE_VARINT
#define PB_LTYPE_INT32  PB_LTYPE_VARINT
#define PB_LTYPE_BOOL   PB_LTYPE_VARINT
#define PB_LTYPE_FLOAT  PB_LTYPE_FIXED32

/* Macros for defining fields */
#define SINGULAR STATIC
#define OTHER    0
#define FIRST    0

typedef struct pb_field_s pb_field_t;
struct pb_field_s {
    uint32_t tag;
    pb_type_t type;
    uint32_t data_offset;
    int32_t size_offset;
    uint8_t data_size;
    uint8_t array_size;
    const void *ptr;
};

typedef struct {
    const pb_field_t *start;
    const pb_field_t *pos;
    void *dest_struct;
    void *pData;
    void *pSize;
} pb_field_iter_t;

typedef struct {
    size_t len;
    uint8_t *data;
} pb_bytes_array_t;

typedef struct pb_callback_s pb_callback_t;
struct pb_callback_s {
    union {
        bool (*decode)(void *stream, const pb_field_t *field, void **arg);
        bool (*encode)(void *stream, const pb_field_t *field, void * const *arg);
    } funcs;
    void *arg;
};

typedef struct {
    const pb_field_t *type;
    void *dest;
} pb_extension_type_t;

#define PB_FIELD(tag, type, rules, allocation, placement, message, field, prevfield, ptr) \
    {tag, (pb_type_t)PB_LTYPE_##type | (pb_type_t)PB_HTYPE_##allocation, \
    offsetof(message, field), 0, sizeof(((message*)0)->field), 0, ptr}

#define PB_LAST_FIELD {0, (pb_type_t) 0, 0, 0, 0, 0, 0}

typedef struct pb_ostream_s pb_ostream_t;
typedef struct pb_istream_s pb_istream_t;

#endif