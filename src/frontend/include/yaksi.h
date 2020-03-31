/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#ifndef YAKSI_H_INCLUDED
#define YAKSI_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>
#include <sys/uio.h>
#include "yaksa_config.h"
#include "yaksa.h"
#include "yaksu.h"

#if !defined ATTRIBUTE
#if defined HAVE_GCC_ATTRIBUTE
#define ATTRIBUTE(a_) __attribute__(a_)
#else /* MPL_HAVE_GCC_ATTRIBUTE */
#define ATTRIBUTE(a_)
#endif /* MPL_HAVE_GCC_ATTRIBUTE */
#endif /* ATTRIBUTE */

#define YAKSI_ENV_DEFAULT_NESTING_LEVEL  (3)

typedef enum {
    YAKSI_TYPE_KIND__BUILTIN,
    YAKSI_TYPE_KIND__CONTIG,
    YAKSI_TYPE_KIND__DUP,
    YAKSI_TYPE_KIND__RESIZED,
    YAKSI_TYPE_KIND__HVECTOR,
    YAKSI_TYPE_KIND__BLKHINDX,
    YAKSI_TYPE_KIND__HINDEXED,
    YAKSI_TYPE_KIND__STRUCT,
    YAKSI_TYPE_KIND__SUBARRAY,
} yaksi_type_kind_e;

struct yaksi_type_s;
struct yaksi_request_s;

/* global variables */
typedef struct {
    yaksu_pool_s type_pool;
    yaksu_pool_s request_pool;
    int is_initialized;
} yaksi_global_s;
extern yaksi_global_s yaksi_global;

typedef struct yaksi_type_s {
    /* yaksa type associated with this structure */
    yaksa_type_t id;
    yaksu_atomic_int refcount;

    yaksi_type_kind_e kind;
    int tree_depth;

    uintptr_t size;
    uintptr_t extent;
    intptr_t lb;
    intptr_t ub;
    intptr_t true_lb;
    intptr_t true_ub;

    /* "is_contig" is set to true only when an array of this type is
     * contiguous, not just one element */
    bool is_contig;
    uintptr_t num_contig;

    union {
        struct {
            int count;
            int blocklength;
            intptr_t stride;
            struct yaksi_type_s *child;
        } hvector;
        struct {
            int count;
            int blocklength;
            intptr_t *array_of_displs;
            struct yaksi_type_s *child;
        } blkhindx;
        struct {
            int count;
            int *array_of_blocklengths;
            intptr_t *array_of_displs;
            struct yaksi_type_s *child;
        } hindexed;
        struct {
            int count;
            int *array_of_blocklengths;
            intptr_t *array_of_displs;
            struct yaksi_type_s **array_of_types;
        } str;
        struct {
            struct yaksi_type_s *child;
        } resized;
        struct {
            int count;
            struct yaksi_type_s *child;
        } contig;
        struct {
            int ndims;
            struct yaksi_type_s *primary;
        } subarray;
        struct {
            struct yaksi_type_s *child;
        } dup;
    } u;

    /* give some space for the backend to store content */
    void *backend;
} yaksi_type_s;

typedef struct yaksi_request_s {
    /* yaksa request associated with this structure */
    yaksa_request_t request;
} yaksi_request_s;


/* pair types */
typedef struct {
    float x;
    int y;
} yaksi_float_int_s;

typedef struct {
    double x;
    int y;
} yaksi_double_int_s;

typedef struct {
    long x;
    int y;
} yaksi_long_int_s;

typedef struct {
    int x;
    int y;
} yaksi_2int_s;

typedef struct {
    short x;
    int y;
} yaksi_short_int_s;

typedef struct {
    long double x;
    int y;
} yaksi_long_double_int_s;

int yaksi_create_hvector(int count, int blocklength, intptr_t stride, yaksi_type_s * intype,
                         yaksi_type_s ** outtype);
int yaksi_create_contig(int count, yaksi_type_s * intype, yaksi_type_s ** outtype);
int yaksi_create_dup(yaksi_type_s * intype, yaksi_type_s ** outtype);
int yaksi_create_hindexed(int count, const int *array_of_blocklengths,
                          const intptr_t * array_of_displacements, yaksi_type_s * intype,
                          yaksi_type_s ** outtype);
int yaksi_create_hindexed_block(int count, int blocklength, const intptr_t * array_of_displacements,
                                yaksi_type_s * intype, yaksi_type_s ** outtype);
int yaksi_create_resized(yaksi_type_s * intype, intptr_t lb, uintptr_t extent,
                         yaksi_type_s ** outtype);
int yaksi_create_struct(int count, const int *array_of_blocklengths,
                        const intptr_t * array_of_displacements, yaksi_type_s ** array_of_intypes,
                        yaksi_type_s ** outtype);
int yaksi_create_subarray(int ndims, const int *array_of_sizes, const int *array_of_subsizes,
                          const int *array_of_starts, yaksa_subarray_order_e order,
                          yaksi_type_s * intype, yaksi_type_s ** outtype);
int yaksi_free(yaksi_type_s * type);

int yaksi_ipack(const void *inbuf, uintptr_t incount, yaksi_type_s * type, uintptr_t inoffset,
                void *outbuf, uintptr_t max_pack_bytes, uintptr_t * actual_pack_bytes,
                yaksi_request_s ** request);
int yaksi_ipack_backend(const void *inbuf, void *outbuf, uintptr_t count, yaksi_type_s * type,
                        yaksi_request_s ** request);
int yaksi_ipack_element(const void *inbuf, yaksi_type_s * type, uintptr_t inoffset, void *outbuf,
                        uintptr_t max_pack_bytes, uintptr_t * actual_pack_bytes,
                        yaksi_request_s ** request);

int yaksi_iunpack(const void *inbuf, uintptr_t insize, void *outbuf, uintptr_t outcount,
                  yaksi_type_s * type, uintptr_t outoffset, yaksi_request_s ** request);
int yaksi_iunpack_backend(const void *inbuf, void *outbuf, uintptr_t outcount, yaksi_type_s * type,
                          yaksi_request_s ** request);
int yaksi_iunpack_element(const void *inbuf, uintptr_t insize, void *outbuf, yaksi_type_s * type,
                          uintptr_t outoffset, yaksi_request_s ** request);

int yaksi_iov_len(uintptr_t count, yaksi_type_s * type, uintptr_t * iov_len);

int yaksi_flatten_size(yaksi_type_s * type, uintptr_t * flattened_type_size);

/* type pool */
int yaksi_type_alloc(yaksi_type_s ** type);
int yaksi_type_free(yaksi_type_s * type);
int yaksi_type_get(yaksa_type_t type, yaksi_type_s ** yaksi_type);

/* request pool */
int yaksi_request_alloc(yaksi_request_s ** request);
int yaksi_request_free(yaksi_request_s * request);
int yaksi_request_get(yaksa_request_t request, yaksi_request_s ** yaksi_request);

#endif /* YAKSI_H_INCLUDED */
