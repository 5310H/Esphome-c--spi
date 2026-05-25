#ifndef PB_API_ENCRYPTION_PB_H_INCLUDED
#define PB_API_ENCRYPTION_PB_H_INCLUDED
#include "pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _EncryptionRequest {
    char dummy_field;
} EncryptionRequest;

typedef struct _EncryptionResponse {
    char dummy_field;
} EncryptionResponse;

typedef struct _EncryptedMessage {
    uint32_t type;
    pb_callback_t data;
} EncryptedMessage;

/* Default values for struct fields */

/* Initializer values for message structs */
#define EncryptionRequest_init_default           {0}
#define EncryptionResponse_init_default          {0}
#define EncryptedMessage_init_default            {0, {{NULL}, NULL}}
#define EncryptionRequest_init_zero              {0}
#define EncryptionResponse_init_zero             {0}
#define EncryptedMessage_init_zero               {0, {{NULL}, NULL}}

/* Field tags (for use in manual encoding/decoding) */
#define EncryptedMessage_type_tag                1
#define EncryptedMessage_data_tag                2

/* Struct field encoding specification for nanopb */
extern const pb_field_t EncryptionRequest_fields[1];
extern const pb_field_t EncryptionResponse_fields[1];
extern const pb_field_t EncryptedMessage_fields[3];

/* Maximum encoded size of messages (where known) */
#define EncryptionRequest_size                   0
#define EncryptionResponse_size                  0
/* EncryptedMessage_size depends on runtime data length */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif