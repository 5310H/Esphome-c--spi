#include "api_service.pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

const pb_field_t ListEntitiesSensorResponse_fields[12] = {
    PB_FIELD(  1, STRING  , SINGULAR, CALLBACK, FIRST, ListEntitiesSensorResponse, object_id, object_id, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, ListEntitiesSensorResponse, key, object_id, 0),
    PB_FIELD(  3, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesSensorResponse, name, key, 0),
    PB_FIELD(  4, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesSensorResponse, unique_id, name, 0),
    PB_FIELD(  5, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesSensorResponse, icon, unique_id, 0),
    PB_FIELD(  6, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesSensorResponse, unit_of_measurement, icon, 0),
    PB_FIELD(  7, INT32   , SINGULAR, STATIC  , OTHER, ListEntitiesSensorResponse, accuracy_decimals, unit_of_measurement, 0),
    PB_FIELD(  8, BOOL    , SINGULAR, STATIC  , OTHER, ListEntitiesSensorResponse, force_update, accuracy_decimals, 0),
    PB_FIELD(  9, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesSensorResponse, device_class, force_update, 0),
    PB_FIELD( 10, INT32   , SINGULAR, STATIC  , OTHER, ListEntitiesSensorResponse, state_class, device_class, 0),
    PB_FIELD( 11, INT32   , SINGULAR, STATIC  , OTHER, ListEntitiesSensorResponse, entity_category, state_class, 0),
    PB_LAST_FIELD
};

const pb_field_t SensorStateResponse_fields[4] = {
    PB_FIELD(  1, UINT32  , SINGULAR, STATIC  , FIRST, SensorStateResponse, key, key, 0),
    PB_FIELD(  2, FLOAT   , SINGULAR, STATIC  , OTHER, SensorStateResponse, state, key, 0),
    PB_FIELD(  3, BOOL    , SINGULAR, STATIC  , OTHER, SensorStateResponse, missing_state, state, 0),
    PB_LAST_FIELD
};

const pb_field_t ListEntitiesBinarySensorResponse_fields[9] = {
    PB_FIELD(  1, STRING  , SINGULAR, CALLBACK, FIRST, ListEntitiesBinarySensorResponse, object_id, object_id, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, ListEntitiesBinarySensorResponse, key, object_id, 0),
    PB_FIELD(  3, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesBinarySensorResponse, name, key, 0),
    PB_FIELD(  4, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesBinarySensorResponse, unique_id, name, 0),
    PB_FIELD(  5, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesBinarySensorResponse, icon, unique_id, 0),
    PB_FIELD(  6, BOOL    , SINGULAR, STATIC  , OTHER, ListEntitiesBinarySensorResponse, is_status_binary_sensor, icon, 0),
    PB_FIELD(  7, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesBinarySensorResponse, device_class, is_status_binary_sensor, 0),
    PB_FIELD(  8, INT32   , SINGULAR, STATIC  , OTHER, ListEntitiesBinarySensorResponse, entity_category, device_class, 0),
    PB_LAST_FIELD
};

const pb_field_t BinarySensorStateResponse_fields[4] = {
    PB_FIELD(  1, UINT32  , SINGULAR, STATIC  , FIRST, BinarySensorStateResponse, key, key, 0),
    PB_FIELD(  2, BOOL    , SINGULAR, STATIC  , OTHER, BinarySensorStateResponse, state, key, 0),
    PB_FIELD(  3, BOOL    , SINGULAR, STATIC  , OTHER, BinarySensorStateResponse, missing_state, state, 0),
    PB_LAST_FIELD
};

const pb_field_t ListEntitiesSwitchResponse_fields[9] = {
    PB_FIELD(  1, STRING  , SINGULAR, CALLBACK, FIRST, ListEntitiesSwitchResponse, object_id, object_id, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, ListEntitiesSwitchResponse, key, object_id, 0),
    PB_FIELD(  3, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesSwitchResponse, name, key, 0),
    PB_FIELD(  4, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesSwitchResponse, unique_id, name, 0),
    PB_FIELD(  5, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesSwitchResponse, icon, unique_id, 0),
    PB_FIELD(  6, BOOL    , SINGULAR, STATIC  , OTHER, ListEntitiesSwitchResponse, assumed_state, icon, 0),
    PB_FIELD(  7, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesSwitchResponse, device_class, assumed_state, 0),
    PB_FIELD(  8, INT32   , SINGULAR, STATIC  , OTHER, ListEntitiesSwitchResponse, entity_category, device_class, 0),
    PB_LAST_FIELD
};

const pb_field_t SwitchStateResponse_fields[3] = {
    PB_FIELD(  1, UINT32  , SINGULAR, STATIC  , FIRST, SwitchStateResponse, key, key, 0),
    PB_FIELD(  2, BOOL    , SINGULAR, STATIC  , OTHER, SwitchStateResponse, state, key, 0),
    PB_LAST_FIELD
};

const pb_field_t SwitchCommandRequest_fields[3] = {
    PB_FIELD(  1, UINT32  , SINGULAR, STATIC  , FIRST, SwitchCommandRequest, key, key, 0),
    PB_FIELD(  2, BOOL    , SINGULAR, STATIC  , OTHER, SwitchCommandRequest, state, key, 0),
    PB_LAST_FIELD
};

const pb_field_t ListEntitiesLightResponse_fields[9] = {
    PB_FIELD(  1, STRING  , SINGULAR, CALLBACK, FIRST, ListEntitiesLightResponse, object_id, object_id, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, ListEntitiesLightResponse, key, object_id, 0),
    PB_FIELD(  3, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesLightResponse, name, key, 0),
    PB_FIELD(  4, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesLightResponse, unique_id, name, 0),
    PB_FIELD(  5, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesLightResponse, icon, unique_id, 0),
    PB_FIELD(  6, INT32   , SINGULAR, STATIC  , OTHER, ListEntitiesLightResponse, entity_category, icon, 0),
    PB_FIELD(  7, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesLightResponse, supported_color_modes, entity_category, 0),
    PB_FIELD(  8, STRING  , SINGULAR, CALLBACK, OTHER, ListEntitiesLightResponse, effects, supported_color_modes, 0),
    PB_LAST_FIELD
};

const pb_field_t LightCommandRequest_fields[18] = {
    PB_FIELD(  1, UINT32  , SINGULAR, STATIC  , FIRST, LightCommandRequest, key, key, 0),
    PB_FIELD(  2, BOOL    , SINGULAR, STATIC  , OTHER, LightCommandRequest, has_state, key, 0),
    PB_FIELD(  3, BOOL    , SINGULAR, STATIC  , OTHER, LightCommandRequest, state, has_state, 0),
    PB_FIELD(  4, BOOL    , SINGULAR, STATIC  , OTHER, LightCommandRequest, has_brightness, state, 0),
    PB_FIELD(  5, FLOAT   , SINGULAR, STATIC  , OTHER, LightCommandRequest, brightness, has_brightness, 0),
    PB_FIELD(  6, BOOL    , SINGULAR, STATIC  , OTHER, LightCommandRequest, has_color_mode, brightness, 0),
    PB_FIELD(  7, INT32   , SINGULAR, STATIC  , OTHER, LightCommandRequest, color_mode, has_color_mode, 0),
    PB_FIELD(  8, BOOL    , SINGULAR, STATIC  , OTHER, LightCommandRequest, has_color_temp, color_mode, 0),
    PB_FIELD(  9, FLOAT   , SINGULAR, STATIC  , OTHER, LightCommandRequest, color_temp, has_color_temp, 0),
    PB_FIELD( 10, BOOL    , SINGULAR, STATIC  , OTHER, LightCommandRequest, has_rgb, color_temp, 0),
    PB_FIELD( 11, FLOAT   , SINGULAR, STATIC  , OTHER, LightCommandRequest, red, has_rgb, 0),
    PB_FIELD( 12, FLOAT   , SINGULAR, STATIC  , OTHER, LightCommandRequest, green, red, 0),
    PB_FIELD( 13, FLOAT   , SINGULAR, STATIC  , OTHER, LightCommandRequest, blue, green, 0),
    PB_FIELD( 14, BOOL    , SINGULAR, STATIC  , OTHER, LightCommandRequest, has_white, blue, 0),
    PB_FIELD( 15, FLOAT   , SINGULAR, STATIC  , OTHER, LightCommandRequest, white, has_white, 0),
    PB_FIELD( 16, BOOL    , SINGULAR, STATIC  , OTHER, LightCommandRequest, has_effect, white, 0),
    PB_FIELD( 17, STRING  , SINGULAR, CALLBACK, OTHER, LightCommandRequest, effect, has_effect, 0),
    PB_LAST_FIELD
};