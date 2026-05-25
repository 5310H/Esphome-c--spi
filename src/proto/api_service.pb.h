#ifndef PB_API_SERVICE_PB_H_INCLUDED
#define PB_API_SERVICE_PB_H_INCLUDED
#include "pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _ListEntitiesSensorResponse {
    pb_callback_t object_id;
    uint32_t key;
    pb_callback_t name;
    pb_callback_t unique_id;
    pb_callback_t icon;
    pb_callback_t unit_of_measurement;
    int32_t accuracy_decimals;
    bool force_update;
    pb_callback_t device_class;
    int32_t state_class;
    int32_t entity_category;
} ListEntitiesSensorResponse;

typedef struct _SensorStateResponse {
    uint32_t key;
    float state;
    bool missing_state;
} SensorStateResponse;

typedef struct _ListEntitiesBinarySensorResponse {
    pb_callback_t object_id;
    uint32_t key;
    pb_callback_t name;
    pb_callback_t unique_id;
    pb_callback_t icon;
    bool is_status_binary_sensor;
    pb_callback_t device_class;
    int32_t entity_category;
} ListEntitiesBinarySensorResponse;

typedef struct _BinarySensorStateResponse {
    uint32_t key;
    bool state;
    bool missing_state;
} BinarySensorStateResponse;

typedef struct _ListEntitiesSwitchResponse {
    pb_callback_t object_id;
    uint32_t key;
    pb_callback_t name;
    pb_callback_t unique_id;
    pb_callback_t icon;
    bool assumed_state;
    pb_callback_t device_class;
    int32_t entity_category;
} ListEntitiesSwitchResponse;

typedef struct _SwitchStateResponse {
    uint32_t key;
    bool state;
} SwitchStateResponse;

typedef struct _SwitchCommandRequest {
    uint32_t key;
    bool state;
} SwitchCommandRequest;

typedef struct _ListEntitiesLightResponse {
    pb_callback_t object_id;
    uint32_t key;
    pb_callback_t name;
    pb_callback_t unique_id;
    pb_callback_t icon;
    int32_t entity_category;
    pb_callback_t supported_color_modes;
    pb_callback_t effects;
} ListEntitiesLightResponse;

typedef struct _LightStateResponse {
    uint32_t key;
    bool state;
    float brightness;
    float color_temp;
    float red;
    float green;
    float blue;
    float white;
    int32_t color_mode;
    pb_callback_t effect;
} LightStateResponse;

typedef struct _LightCommandRequest {
    uint32_t key;
    bool has_state;
    bool state;
    bool has_brightness;
    float brightness;
    bool has_color_mode;
    int32_t color_mode;
    bool has_color_temp;
    float color_temp;
    bool has_rgb;
    float red;
    float green;
    float blue;
    bool has_white;
    float white;
    bool has_effect;
    pb_callback_t effect;
} LightCommandRequest;

/* Initializer values for message structs */
#define ListEntitiesSensorResponse_init_default  {{{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, 0, 0, {{NULL}, NULL}, 0, 0}
#define SensorStateResponse_init_default         {0, 0, 0}
#define ListEntitiesBinarySensorResponse_init_default {{{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, 0, {{NULL}, NULL}, 0}
#define BinarySensorStateResponse_init_default   {0, 0, 0}
#define ListEntitiesSwitchResponse_init_default  {{{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, 0, {{NULL}, NULL}, 0}
#define SwitchStateResponse_init_default         {0, 0}
#define ListEntitiesLightResponse_init_default   {{{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}}
#define SwitchCommandRequest_init_default        {0, 0}
#define LightCommandRequest_init_default         {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {{NULL}, NULL}}

/* Field tags (for use in manual encoding/decoding) */
#define ListEntitiesSensorResponse_object_id_tag 1
#define ListEntitiesSensorResponse_key_tag       2
#define ListEntitiesSensorResponse_name_tag      3
#define ListEntitiesSensorResponse_unique_id_tag 4
#define ListEntitiesSensorResponse_icon_tag      5
#define ListEntitiesSensorResponse_unit_of_measurement_tag 6
#define ListEntitiesSensorResponse_accuracy_decimals_tag 7
#define ListEntitiesSensorResponse_force_update_tag 8
#define ListEntitiesSensorResponse_device_class_tag 9
#define ListEntitiesSensorResponse_state_class_tag 10
#define ListEntitiesSensorResponse_entity_category_tag 11
#define SensorStateResponse_key_tag              1
#define SensorStateResponse_state_tag            2
#define SensorStateResponse_missing_state_tag    3
#define ListEntitiesBinarySensorResponse_object_id_tag 1
#define ListEntitiesBinarySensorResponse_key_tag 2
#define ListEntitiesBinarySensorResponse_name_tag 3
#define ListEntitiesBinarySensorResponse_unique_id_tag 4
#define ListEntitiesBinarySensorResponse_icon_tag 5
#define ListEntitiesBinarySensorResponse_is_status_binary_sensor_tag 6
#define ListEntitiesBinarySensorResponse_device_class_tag 7
#define ListEntitiesBinarySensorResponse_entity_category_tag 8
#define BinarySensorStateResponse_key_tag        1
#define BinarySensorStateResponse_state_tag      2
#define BinarySensorStateResponse_missing_state_tag 3
#define SwitchCommandRequest_key_tag             1
#define SwitchCommandRequest_state_tag           2
#define SwitchStateResponse_key_tag              1
#define SwitchStateResponse_state_tag            2
#define LightCommandRequest_key_tag              1
#define LightCommandRequest_state_tag            3
#define LightCommandRequest_brightness_tag       5
#define LightCommandRequest_color_mode_tag       7
#define LightCommandRequest_color_temp_tag       9
#define LightCommandRequest_red_tag              11
#define LightCommandRequest_green_tag            12
#define LightCommandRequest_blue_tag             13
#define LightCommandRequest_white_tag            14
#define LightCommandRequest_effect_tag           16

/* Struct field encoding specification for nanopb */
extern const pb_field_t ListEntitiesSensorResponse_fields[12];
extern const pb_field_t SensorStateResponse_fields[4];
extern const pb_field_t ListEntitiesBinarySensorResponse_fields[9];
extern const pb_field_t BinarySensorStateResponse_fields[4];
extern const pb_field_t ListEntitiesSwitchResponse_fields[9];
extern const pb_field_t SwitchStateResponse_fields[3];
extern const pb_field_t ListEntitiesLightResponse_fields[9];
extern const pb_field_t SwitchCommandRequest_fields[3];
extern const pb_field_t LightCommandRequest_fields[18];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif