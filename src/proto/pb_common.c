#include "pb_common.h"

bool pb_field_iter_begin(pb_field_iter_t *iter, const pb_field_t *fields, void *dest_struct)
{
    if (fields == NULL || fields->tag == 0) return false;
    iter->start = fields;
    iter->pos = fields;
    iter->dest_struct = dest_struct;
    iter->pData = (uint8_t*)dest_struct + iter->pos->data_offset;
    iter->pSize = (uint8_t*)iter->pData + iter->pos->size_offset;
    return true;
}

bool pb_field_iter_next(pb_field_iter_t *iter)
{
    const pb_field_t *next_pos = iter->pos + 1;
    if (next_pos->tag == 0) return false;

    iter->pos = next_pos;
    /* Re-calculate absolute pointers from struct base */
    iter->pData = (uint8_t*)iter->dest_struct + iter->pos->data_offset;
    iter->pSize = (uint8_t*)iter->pData + iter->pos->size_offset;
    return true;
}

bool pb_field_iter_find(pb_field_iter_t *iter, uint32_t tag)
{
    if (iter->pos->tag == tag) return true;

    const pb_field_t *start_pos = iter->pos;
    while (pb_field_iter_next(iter)) {
        if (iter->pos->tag == tag) return true;
    }

    /* Wrap around to start */
    pb_field_iter_begin(iter, iter->start, iter->dest_struct);
    if (iter->pos->tag == tag) return true;
    while (pb_field_iter_next(iter) && iter->pos != start_pos) {
        if (iter->pos->tag == tag) return true;
    }

    return false;
}