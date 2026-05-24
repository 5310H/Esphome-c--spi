#include "esphome_api.h"
#include "esphome_api.pb.h"

#include <string.h>
#include <stdio.h>

#define MAX_ENTITIES 64

typedef struct {
    uint32_t key;
    char object_id[64];
    uint32_t legacy_type;
} entity_entry_t;

static entity_entry_t registry[MAX_ENTITIES];
static size_t registry_count = 0;

// ---------------------------------------------------------------------------
// Store entity info
// ---------------------------------------------------------------------------
void esph_registry_add(const esphome_api_EntityInfo *info)
{
    if (registry_count >= MAX_ENTITIES)
        return;

    registry[registry_count].key = info->key;
    strncpy(registry[registry_count].object_id,
            info->object_id,
            sizeof(registry[registry_count].object_id)-1);
    registry[registry_count].legacy_type = info->legacy_type;

    registry_count++;
}

// ---------------------------------------------------------------------------
// Lookup key by entity_id (e.g. "switch.lamp")
// ---------------------------------------------------------------------------
uint32_t esph_registry_lookup_key(const char *entity_id)
{
    // entity_id format: "switch.lamp"
    const char *dot = strchr(entity_id, '.');
    if (!dot) return 0;

    const char *object = dot + 1;

    for (size_t i = 0; i < registry_count; i++) {
        if (strcmp(registry[i].object_id, object) == 0)
            return registry[i].key;
    }

    return 0;
}
