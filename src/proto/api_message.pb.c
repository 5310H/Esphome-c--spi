#include "api_message.pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

const pb_field_t HelloRequest_fields[4] = {
    PB_FIELD(  1, STRING  , SINGULAR, CALLBACK, FIRST, HelloRequest, client_info, client_info, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, HelloRequest, api_version_major, client_info, 0),
    PB_FIELD(  3, UINT32  , SINGULAR, STATIC  , OTHER, HelloRequest, api_version_minor, api_version_major, 0),
    PB_LAST_FIELD
};

const pb_field_t HelloResponse_fields[5] = {
    PB_FIELD(  1, UINT32  , SINGULAR, STATIC  , FIRST, HelloResponse, api_version_major, api_version_major, 0),
    PB_FIELD(  2, UINT32  , SINGULAR, STATIC  , OTHER, HelloResponse, api_version_minor, api_version_major, 0),
    PB_FIELD(  3, STRING  , SINGULAR, CALLBACK, OTHER, HelloResponse, server_info, api_version_minor, 0),
    PB_FIELD(  4, STRING  , SINGULAR, CALLBACK, OTHER, HelloResponse, name, server_info, 0),
    PB_LAST_FIELD
};

const pb_field_t ConnectRequest_fields[2] = {
    PB_FIELD(  1, STRING  , SINGULAR, CALLBACK, FIRST, ConnectRequest, password, password, 0),
    PB_LAST_FIELD
};

const pb_field_t ConnectResponse_fields[2] = {
    PB_FIELD(  1, BOOL    , SINGULAR, STATIC  , FIRST, ConnectResponse, invalid_password, invalid_password, 0),
    PB_LAST_FIELD
};

const pb_field_t DisconnectRequest_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t DisconnectResponse_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t PingRequest_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t PingResponse_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t DeviceInfoRequest_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t DeviceInfoResponse_fields[10] = {
    PB_FIELD(  1, BOOL    , SINGULAR, STATIC  , FIRST, DeviceInfoResponse, uses_password, uses_password, 0),
    PB_FIELD(  2, STRING  , SINGULAR, CALLBACK, OTHER, DeviceInfoResponse, name, uses_password, 0),
    PB_FIELD(  3, STRING  , SINGULAR, CALLBACK, OTHER, DeviceInfoResponse, model, name, 0),
    PB_FIELD(  4, STRING  , SINGULAR, CALLBACK, OTHER, DeviceInfoResponse, mac_address, model, 0),
    PB_FIELD(  5, STRING  , SINGULAR, CALLBACK, OTHER, DeviceInfoResponse, esphome_version, mac_address, 0),
    PB_FIELD(  6, STRING  , SINGULAR, CALLBACK, OTHER, DeviceInfoResponse, compilation_time, esphome_version, 0),
    PB_FIELD(  7, BOOL    , SINGULAR, STATIC  , OTHER, DeviceInfoResponse, has_deep_sleep, compilation_time, 0),
    PB_FIELD(  8, STRING  , SINGULAR, CALLBACK, OTHER, DeviceInfoResponse, project_name, has_deep_sleep, 0),
    PB_FIELD(  9, STRING  , SINGULAR, CALLBACK, OTHER, DeviceInfoResponse, project_version, project_name, 0),
    PB_LAST_FIELD
};

const pb_field_t ListEntitiesRequest_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t ListEntitiesDoneResponse_fields[1] = {
    PB_LAST_FIELD
};

const pb_field_t SubscribeStatesRequest_fields[1] = {
    PB_LAST_FIELD
};