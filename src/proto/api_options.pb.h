#ifndef PB_API_OPTIONS_PB_H_INCLUDED
#define PB_API_OPTIONS_PB_H_INCLUDED
#include "pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _AppEntityState {
    AppEntityState_APP_ENTITY_STATE_NONE = 0,
    AppEntityState_APP_ENTITY_STATE_STOPPED = 1,
    AppEntityState_APP_ENTITY_STATE_STARTING = 2,
    AppEntityState_APP_ENTITY_STATE_RUNNING = 3
} AppEntityState;

/* Struct definitions */
typedef struct _Void {
    char dummy_field;
} Void;

/* Initializer values for message structs */
#define Void_init_default                        {0}
#define Void_init_zero                           {0}

/* Struct field encoding specification for nanopb */
extern const pb_field_t Void_fields[1];

/* Extensions */
#define esphome_api_id_tag                       1000
extern const pb_extension_type_t esphome_api_id;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif