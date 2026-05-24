#include "pb_common.h"

bool pb_field_iter_begin(pb_field_iter_t *iter, const pb_field_t *fields, void *dest_struct)
{
    iter->start = fields;
    iter->pos = fields;
    iter->dest_struct = dest_struct;
    iter->pData = (char*)dest_struct + iter->pos->data_offset;
    iter->pSize = (char*)iter->pData + iter->pos->size_offset;
    return true;
}

bool pb_field_iter_next(pb_field_iter_t *iter)
{
    const pb_field_t *prev_field = iter->pos;

    if (prev_field->tag == 0)
    {
        return false;
    }
    
    iter->pos++;
    if (iter->pos->tag == 0)
    {
        pb_field_iter_begin(iter, iter->start, iter->dest_struct);
        return false;
    }
    
    iter->pData = (char*)iter->pData + prev_field->data_offset + iter->pos->data_offset;
    iter->pSize = (char*)iter->pData + iter->pos->size_offset;
    return true;
}

bool pb_field_iter_find(pb_field_iter_t *iter, uint32_t tag)
{
    const pb_field_t *start = iter->pos;
    
    do {
        if (iter->pos->tag == tag && PB_LTYPE(iter->pos->type) != PB_LTYPE_EXTENSION)
        {
            return true;
        }
        pb_field_iter_next(iter);
    } while (iter->pos != start);
    
    return false;
}