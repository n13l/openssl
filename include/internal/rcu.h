/*
 * Copyright 2023-2026 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OPENSSL_RCU_H
#define OPENSSL_RCU_H
#pragma once

#include "crypto/context.h"
#include "internal/threads_common.h"

typedef void (*rcu_cb_fn)(void *data);

typedef struct rcu_lock_st CRYPTO_RCU_LOCK;

typedef struct rcu_cb_item CRYPTO_RCU_CB_ITEM;

CRYPTO_RCU_LOCK *ossl_rcu_lock_new(int num_writers, OSSL_LIB_CTX *ctx);
void ossl_rcu_lock_free(CRYPTO_RCU_LOCK *lock);
int ossl_rcu_read_lock(CRYPTO_RCU_LOCK *lock);
void ossl_rcu_write_lock(CRYPTO_RCU_LOCK *lock);
void ossl_rcu_write_unlock(CRYPTO_RCU_LOCK *lock);
void ossl_rcu_read_unlock(CRYPTO_RCU_LOCK *lock);
void ossl_synchronize_rcu(CRYPTO_RCU_LOCK *lock);
CRYPTO_RCU_CB_ITEM *ossl_rcu_cb_item_new(void);
void ossl_rcu_cb_item_free(CRYPTO_RCU_CB_ITEM *item);
void ossl_rcu_call(CRYPTO_RCU_LOCK *lock, CRYPTO_RCU_CB_ITEM *item,
    rcu_cb_fn cb, void *data);
void *ossl_rcu_uptr_deref(void **p);
void ossl_rcu_assign_uptr(void **p, void **v);
#define ossl_rcu_deref(p) ossl_rcu_uptr_deref((void **)p)
#define ossl_rcu_assign_ptr(p, v) ossl_rcu_assign_uptr((void **)p, (void **)v)

/*
 * ossl_rcu_deref() for a hot path. ossl_rcu_deref() is a function call around
 * one load of a published pointer; where that load is the compiler's own
 * atomic (the same condition under which crypto/threads_pthread.c uses it),
 * this does it in place, with the same acquire ordering. |p| points at the
 * pointer to load. The result has that pointer's type where the load is done
 * in place and is a void * otherwise.
 */
#if defined(OSSL_USE_GCC_ATOMICS)
#define ossl_rcu_deref_inline(p) __atomic_load_n((p), __ATOMIC_ACQUIRE)
#else
#define ossl_rcu_deref_inline(p) ossl_rcu_deref(p)
#endif

#endif
