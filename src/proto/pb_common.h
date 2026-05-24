#ifndef PB_COMMON_H_INCLUDED
#define PB_COMMON_H_INCLUDED
#include "pb.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Internal function for iterate_fields.
 * These are used by both the encoder and decoder to traverse the pb_field_t arrays. */
bool pb_field_iter_begin(pb_field_iter_t *iter, const pb_field_t *fields, void *dest_struct);
bool pb_field_iter_next(pb_field_iter_t *iter);
bool pb_field_iter_find(pb_field_iter_t *iter, uint32_t tag);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif