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

#include "crypto/s2n_drbg.h"

#include "utils/s2n_blob.h"
#include "utils/s2n_result.h"

extern S2N_RESULT s2n_rand_init(void);
extern S2N_RESULT s2n_rand_cleanup(void);
extern S2N_RESULT s2n_rand_cleanup_thread(void);
extern S2N_RESULT s2n_set_private_drbg_for_test(struct s2n_drbg drbg);
extern S2N_RESULT s2n_get_public_random_data(struct s2n_blob *blob);
extern S2N_RESULT s2n_get_public_random_bytes_used(void);
extern S2N_RESULT s2n_get_private_random_data(struct s2n_blob *blob);
extern S2N_RESULT s2n_get_private_random_bytes_used(void);
extern S2N_RESULT s2n_get_urandom_data(struct s2n_blob *blob);
extern S2N_RESULT s2n_public_random(int64_t max, int64_t *output);
extern S2N_RESULT s2n_cpu_supports_rdrand(void);
extern S2N_RESULT s2n_get_rdrand_data(struct s2n_blob *out);
