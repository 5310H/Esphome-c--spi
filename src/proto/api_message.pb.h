#ifndef PB_API_MESSAGE_PB_H_INCLUDED
#define PB_API_MESSAGE_PB_H_INCLUDED
#include "pb.h"

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _HelloRequest {
    pb_callback_t client_info;
    uint32_t api_version_major;
    uint32_t api_version_minor;
} HelloRequest;

typedef struct _HelloResponse {
    uint32_t api_version_major;
    uint32_t api_version_minor;
    pb_callback_t server_info;
    pb_callback_t name;
} HelloResponse;

typedef struct _ConnectRequest {
    pb_callback_t password;
} ConnectRequest;

typedef struct _ConnectResponse {
    bool invalid_password;
} ConnectResponse;

typedef struct _DisconnectRequest {
    char dummy_field;
} DisconnectRequest;

typedef struct _DisconnectResponse {
    char dummy_field;
} DisconnectResponse;

typedef struct _PingRequest {
    char dummy_field;
} PingRequest;

typedef struct _PingResponse {
    char dummy_field;
} PingResponse;

typedef struct _DeviceInfoRequest {
    char dummy_field;
} DeviceInfoRequest;

typedef struct _DeviceInfoResponse {
    bool uses_password;
    pb_callback_t name;
    pb_callback_t model;
    pb_callback_t mac_address;
    pb_callback_t esphome_version;
    pb_callback_t compilation_time;
    bool has_deep_sleep;
    pb_callback_t project_name;
    pb_callback_t project_version;
} DeviceInfoResponse;

typedef struct _ListEntitiesRequest {
    char dummy_field;
} ListEntitiesRequest;

typedef struct _ListEntitiesDoneResponse {
    char dummy_field;
} ListEntitiesDoneResponse;

typedef struct _SubscribeStatesRequest {
    char dummy_field;
} SubscribeStatesRequest;

/* Initializer values for message structs */
#define HelloRequest_init_default                {{{NULL}, NULL}, 0, 0}
#define HelloResponse_init_default               {0, 0, {{NULL}, NULL}, {{NULL}, NULL}}
#define ConnectRequest_init_default              {{{NULL}, NULL}}
#define ConnectResponse_init_default             {0}
#define DisconnectRequest_init_default           {0}
#define DisconnectResponse_init_default          {0}
#define PingRequest_init_default                 {0}
#define PingResponse_init_default                {0}
#define DeviceInfoRequest_init_default           {0}
#define DeviceInfoResponse_init_default          {0, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, {{NULL}, NULL}, 0, {{NULL}, NULL}, {{NULL}, NULL}}
#define ListEntitiesRequest_init_default         {0}
#define ListEntitiesDoneResponse_init_default    {0}
#define SubscribeStatesRequest_init_default      {0}

/* Field tags (for use in manual encoding/decoding) */
#define HelloRequest_client_info_tag             1
#define HelloRequest_api_version_major_tag       2
#define HelloRequest_api_version_minor_tag       3
#define HelloResponse_api_version_major_tag      1
#define HelloResponse_api_version_minor_tag      2
#define HelloResponse_server_info_tag            3
#define HelloResponse_name_tag                  4
#define ConnectRequest_password_tag              1
#define ConnectResponse_invalid_password_tag     1
#define DeviceInfoResponse_uses_password_tag     1
#define DeviceInfoResponse_name_tag              2
#define DeviceInfoResponse_model_tag             3
#define DeviceInfoResponse_mac_address_tag       4
#define DeviceInfoResponse_esphome_version_tag   5
#define DeviceInfoResponse_compilation_time_tag  6
#define DeviceInfoResponse_has_deep_sleep_tag    7
#define DeviceInfoResponse_project_name_tag      8
#define DeviceInfoResponse_project_version_tag   9

/* Struct field encoding specification for nanopb */
extern const pb_field_t HelloRequest_fields[4];
extern const pb_field_t HelloResponse_fields[5];
extern const pb_field_t ConnectRequest_fields[2];
extern const pb_field_t ConnectResponse_fields[2];
extern const pb_field_t DeviceInfoResponse_fields[10];
extern const pb_field_t PingRequest_fields[1];
extern const pb_field_t PingResponse_fields[1];
extern const pb_field_t ListEntitiesRequest_fields[1];
extern const pb_field_t ListEntitiesDoneResponse_fields[1];
extern const pb_field_t SubscribeStatesRequest_fields[1];

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif