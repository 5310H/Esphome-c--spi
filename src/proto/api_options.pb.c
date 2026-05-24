#include "api_options.pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Field definitions for the Void message.
 * As this message has no fields in the proto definition, it only contains 
 * the sentinel value. */
const pb_field_t Void_fields[1] = {
    PB_LAST_FIELD
};

/* Extension field for esphome_api_id. 
 * This allows the library to programmatically determine the message type ID
 * defined in the .proto files. */
const pb_extension_type_t esphome_api_id = {
    NULL,
    NULL
};