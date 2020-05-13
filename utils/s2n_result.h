/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#pragma once

#include <s2n.h>
#include <stdbool.h>

/* A value which indicates the outcome of a function */
typedef struct {
    int __error_signal;
} s2n_result;

/* used to signal a successful function return */
extern const s2n_result S2N_RESULT_OK;
/* used to signal an error while executing a function */
extern const s2n_result S2N_RESULT_ERROR;

#if defined(__clang__) || defined(__GNUC__)
#define S2N_RESULT_MUST_USE __attribute__((warn_unused_result))
#else
#define S2N_RESULT_MUST_USE
#endif

/* returns true when the result is S2N_RESULT_OK */
S2N_RESULT_MUST_USE bool s2n_result_is_ok(s2n_result result);

/* returns true when the result is S2N_RESULT_ERROR */
S2N_RESULT_MUST_USE bool s2n_result_is_error(s2n_result result);

/* used in function declarations to signal function fallibility */
#define S2N_RESULT S2N_RESULT_MUST_USE s2n_result

/* s2n_result GUARD helpers */
#define GUARD_RESULT( x )               do {if ( s2n_result_is_error(x) ) return S2N_RESULT_ERROR;} while (0)
#define GUARD_AS_RESULT( x )            do {if ( (x) < 0 ) return S2N_RESULT_ERROR;} while (0)
#define GUARD_AS_POSIX( x )             do {if ( s2n_result_is_error(x) ) return S2N_FAILURE;} while (0)
#define GUARD_RESULT_GOTO( x, label )   do {if ( s2n_result_is_error(x) ) goto label;} while (0)
#define GUARD_RESULT_PTR( x )           do {if ( s2n_result_is_error(x) ) return NULL;} while (0)
#define GUARD_RESULT_NONNULL( x )       do {if ( (x) == NULL ) return S2N_RESULT_ERROR;} while (0)

#define GUARD_POSIX( x )                do {if ( (x) < 0 ) return S2N_FAILURE;} while (0)
#define GUARD_POSIX_STRICT( x )         do {if ( (x) != 0 ) return S2N_FAILURE;} while (0)
#define GUARD_POSIX_GOTO( x , label )   do {if ( (x) < 0 ) goto label;} while (0)
#define GUARD_POSIX_PTR( x )            do {if ( (x) < 0 ) return NULL;} while (0)

/* s2n_result ERROR helpers */
/* note: eventually this will just alias S2N_ERROR and be deprecated once everything is using s2n_result */
#define S2N_ERROR_RESULT( x )              do { _S2N_ERROR( ( x ) ); return S2N_RESULT_ERROR; } while (0)
#define S2N_ERROR_RESULT_PTR( x )          do { _S2N_ERROR( ( x ) ); return NULL; } while (0)
#define S2N_ERROR_RESULT_IF( cond , x )    do { if ( cond ) { S2N_ERROR_RESULT( x ); }} while (0)
#define S2N_ERROR_IF_NULL( x )             S2N_ERROR_RESULT_IF(x == NULL, S2N_ERR_NULL)

#define CHECKED_MEMCPY( d, s, n )                                           \
  do {                                                                      \
    __typeof( n ) __tmp_n = ( n );                                          \
    if ( __tmp_n ) {                                                        \
      void *r = trace_memcpy_check( (d), (s) , (__tmp_n), _S2N_DEBUG_LINE); \
      GUARD_RESULT_NONNULL(r);                                              \
    }                                                                       \
  } while(0)

#define CHECKED_MEMSET( d, c, n )                                            \
  do {                                                                      \
    __typeof( n ) __tmp_n = ( n );                                          \
    if ( __tmp_n ) {                                                        \
      __typeof( d ) __tmp_d = ( d );                                        \
      S2N_ERROR_IF_NULL( __tmp_d );                                         \
      memset( __tmp_d, (c), __tmp_n);                                       \
    }                                                                       \
  } while(0)

#define char_to_digit_result(c, d)  do { if(!isdigit(c)) { S2N_ERROR_RESULT(S2N_ERR_SAFETY); } d = c - '0'; } while(0)

/* Range check a number */
#define gte_check_result(n, min)  do { if ( (n) < min ) { S2N_ERROR_RESULT(S2N_ERR_SAFETY); } } while(0)
#define lte_check_result(n, max)  do { if ( (n) > max ) { S2N_ERROR_RESULT(S2N_ERR_SAFETY); } } while(0)
#define gt_check_result(n, min)  do { if ( (n) <= min ) { S2N_ERROR_RESULT(S2N_ERR_SAFETY); } } while(0)
#define lt_check_result(n, max)  do { if ( (n) >= max ) { S2N_ERROR_RESULT(S2N_ERR_SAFETY); } } while(0)
#define eq_check_result(a, b)  do { if ( (a) != (b) ) { S2N_ERROR_RESULT(S2N_ERR_SAFETY); } } while(0)
#define ne_check_result(a, b)  do { if ( (a) == (b) ) { S2N_ERROR_RESULT(S2N_ERR_SAFETY); } } while(0)
#define inclusive_range_check_result( low, n, high )   \
  do  {                                         \
    __typeof( n ) __tmp_n = ( n );              \
    gte_check_result(__tmp_n, low);             \
    lte_check_result(__tmp_n, high);            \
  } while (0)
#define exclusive_range_check_result( low, n, high )   \
  do {                                          \
    __typeof( n ) __tmp_n = ( n );              \
    gt_check_result(__tmp_n, low);              \
    lt_check_result(__tmp_n, high);             \
  } while (0)

/* Similar to GUARD, but preserves the blocking error code. S2N_CALLBACK_BLOCKED is left to preserve backwards compatibility .*/
#define GUARD_NONBLOCKING_RESULT( x )          \
  do {                                  \
    int __tmp_r = (x);                  \
    if (ERR_IS_BLOCKING( __tmp_r )) {   \
      S2N_ERROR_RESULT( __tmp_r );      \
    }                                   \
    GUARD( __tmp_r );                   \
  } while(0)

/* TODO: use the OSSL error code in error reporting https://github.com/awslabs/s2n/issues/705 */
#define GUARD_OSSL_RESULT( x , errcode )        \
  do {                                          \
  if (( x ) != 1) {                             \
    S2N_ERROR_RESULT( errcode );                \
  }                                             \
  } while (0)
