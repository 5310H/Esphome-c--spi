#include "api_encryption.pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

const pb_field_t EncryptionRequest_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t EncryptionResponse_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t EncryptedMessage_fields[3] = {
    PB_FIELD(  1, UINT32  , SINGULAR, STATIC  , FIRST, EncryptedMessage, type, type, 0),
    PB_FIELD(  2, BYTES   , SINGULAR, CALLBACK, OTHER, EncryptedMessage, data, type, 0),
    PB_LAST_FIELD
};