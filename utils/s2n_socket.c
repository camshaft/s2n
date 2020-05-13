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

#include <tls/s2n_connection.h>

#include "utils/s2n_result.h"
#include <utils/s2n_socket.h>
#include <utils/s2n_safety.h>

#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#if TCP_CORK
    #define S2N_CORK        TCP_CORK
    #define S2N_CORK_ON     1
    #define S2N_CORK_OFF    0
#elif TCP_NOPUSH
    #define S2N_CORK        TCP_NOPUSH
    #define S2N_CORK_ON     1
    #define S2N_CORK_OFF    0
#elif TCP_NODELAY
    #define S2N_CORK        TCP_NODELAY
    #define S2N_CORK_ON     0
    #define S2N_CORK_OFF    1
#endif

S2N_RESULT s2n_socket_quickack(struct s2n_connection *conn)
{
#ifdef TCP_QUICKACK
    if (!conn->managed_io) {
        return S2N_RESULT_OK;
    }

    struct s2n_socket_read_io_context *r_io_ctx = (struct s2n_socket_read_io_context *) conn->recv_io_context;
    if (r_io_ctx->tcp_quickack_set) {
        return S2N_RESULT_OK;
    }

    /* Ignore the return value, if it fails it fails */
    int optval = 1;
    if (setsockopt(r_io_ctx->fd, IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof(optval)) == 0) {
        r_io_ctx->tcp_quickack_set = 1;
    }
#endif

    return S2N_RESULT_OK;
}

S2N_RESULT s2n_socket_write_snapshot(struct s2n_connection *conn)
{
#ifdef S2N_CORK
    socklen_t corklen = sizeof(int);

    struct s2n_socket_write_io_context *w_io_ctx = (struct s2n_socket_write_io_context *) conn->send_io_context;
    S2N_ERROR_IF_NULL(w_io_ctx);

    getsockopt(w_io_ctx->fd, IPPROTO_TCP, S2N_CORK, &w_io_ctx->original_cork_val, &corklen);
    eq_check_result(corklen, sizeof(int));
    w_io_ctx->original_cork_is_set = 1;
#endif

    return S2N_RESULT_OK;
}


S2N_RESULT s2n_socket_read_snapshot(struct s2n_connection *conn)
{
#ifdef SO_RCVLOWAT
    socklen_t watlen = sizeof(int);

    struct s2n_socket_read_io_context *r_io_ctx = (struct s2n_socket_read_io_context *) conn->recv_io_context;
    S2N_ERROR_IF_NULL(r_io_ctx);

    getsockopt(r_io_ctx->fd, SOL_SOCKET, SO_RCVLOWAT, &r_io_ctx->original_rcvlowat_val, &watlen);
    eq_check_result(watlen, sizeof(int));
    r_io_ctx->original_rcvlowat_is_set = 1;
#endif

    return S2N_RESULT_OK;
}

S2N_RESULT s2n_socket_write_restore(struct s2n_connection *conn)
{
#ifdef S2N_CORK
    struct s2n_socket_write_io_context *w_io_ctx = (struct s2n_socket_write_io_context *) conn->send_io_context;
    S2N_ERROR_IF_NULL(w_io_ctx);

    if (!w_io_ctx->original_cork_is_set) {
        return S2N_RESULT_OK;
    }
    setsockopt(w_io_ctx->fd, IPPROTO_TCP, S2N_CORK, &w_io_ctx->original_cork_val, sizeof(w_io_ctx->original_cork_val));
    w_io_ctx->original_cork_is_set = 0;
#endif

    return S2N_RESULT_OK;
}

S2N_RESULT s2n_socket_read_restore(struct s2n_connection *conn)
{
#ifdef SO_RCVLOWAT
    struct s2n_socket_read_io_context *r_io_ctx = (struct s2n_socket_read_io_context *) conn->recv_io_context;
    S2N_ERROR_IF_NULL(r_io_ctx);

    if (!r_io_ctx->original_rcvlowat_is_set) {
        return S2N_RESULT_OK;
    }
    setsockopt(r_io_ctx->fd, SOL_SOCKET, SO_RCVLOWAT, &r_io_ctx->original_rcvlowat_val, sizeof(r_io_ctx->original_rcvlowat_val));
    r_io_ctx->original_rcvlowat_is_set = 0;
#endif

    return S2N_RESULT_OK;
}

S2N_RESULT s2n_socket_was_corked(struct s2n_connection *conn, bool *was_corked)
{
    /* If we're not using custom I/O and a send fd has not been set yet, return false*/
    if (!conn->managed_io || !conn->send) {
        *was_corked = false;
        return S2N_RESULT_OK;
    }

    struct s2n_socket_write_io_context *io_ctx = (struct s2n_socket_write_io_context *) conn->send_io_context;
    S2N_ERROR_IF_NULL(io_ctx);

    *was_corked = io_ctx->original_cork_val;
    return S2N_RESULT_OK;
}

S2N_RESULT s2n_socket_write_cork(struct s2n_connection *conn)
{
#ifdef S2N_CORK
    int optval = S2N_CORK_ON;

    struct s2n_socket_write_io_context *w_io_ctx = (struct s2n_socket_write_io_context *) conn->send_io_context;
    S2N_ERROR_IF_NULL(w_io_ctx);

    /* Ignore the return value, if it fails it fails */
    setsockopt(w_io_ctx->fd, IPPROTO_TCP, S2N_CORK, &optval, sizeof(optval));
#endif

    return S2N_RESULT_OK;
}

S2N_RESULT s2n_socket_write_uncork(struct s2n_connection *conn)
{
#ifdef S2N_CORK
    int optval = S2N_CORK_OFF;

    struct s2n_socket_write_io_context *w_io_ctx = (struct s2n_socket_write_io_context *) conn->send_io_context;
    S2N_ERROR_IF_NULL(w_io_ctx);

    /* Ignore the return value, if it fails it fails */
    setsockopt(w_io_ctx->fd, IPPROTO_TCP, S2N_CORK, &optval, sizeof(optval));
#endif

    return S2N_RESULT_OK;
}

S2N_RESULT s2n_socket_set_read_size(struct s2n_connection *conn, int size)
{
#ifdef SO_RCVLOWAT
    struct s2n_socket_read_io_context *r_io_ctx = (struct s2n_socket_read_io_context *) conn->recv_io_context;
    S2N_ERROR_IF_NULL(r_io_ctx);

    setsockopt(r_io_ctx->fd, SOL_SOCKET, SO_RCVLOWAT, &size, sizeof(size));
#endif

    return S2N_RESULT_OK;
}

S2N_RESULT s2n_socket_read(void *io_context, uint8_t *buf, uint32_t len, uint32_t *bytes_read)
{
    int rfd = ((struct s2n_socket_read_io_context*) io_context)->fd;
    if (rfd < 0) {
        errno = EBADF;
        S2N_ERROR_RESULT(S2N_ERR_BAD_FD);
    }

    /* Clear the quickack flag so we know to reset it */
    ((struct s2n_socket_read_io_context*) io_context)->tcp_quickack_set = 0;

    /* On success, the number of bytes read is returned. On failure, -1 is
     * returned and errno is set appropriately. */
    ssize_t read_result = read(rfd, buf, len);
    if (read_result < 0) {
        return S2N_RESULT_ERROR;
    }

    *bytes_read = read_result;
    return S2N_RESULT_OK;
}

int s2n_socket_read_posix(void *io_context, uint8_t *buf, uint32_t len)
{
    uint32_t bytes_read = 0;
    GUARD_AS_POSIX(s2n_socket_read(io_context, buf, len, &bytes_read));
    return bytes_read;
}

S2N_RESULT s2n_socket_write(void *io_context, const uint8_t *buf, uint32_t len, uint32_t *bytes_written)
{
    int wfd = ((struct s2n_socket_write_io_context*) io_context)->fd;
    if (wfd < 0) {
        errno = EBADF;
        S2N_ERROR_RESULT(S2N_ERR_BAD_FD);
    }

    /* On success, the number of bytes written is returned. On failure, -1 is
     * returned and errno is set appropriately. */
    ssize_t write_result = write(wfd, buf, len);
    if (write_result < 0) {
        return S2N_RESULT_ERROR;
    }

    *bytes_written = write_result;
    return S2N_RESULT_OK;
}

int s2n_socket_write_posix(void *io_context, const uint8_t *buf, uint32_t len)
{
    uint32_t bytes_written = 0;
    GUARD_AS_POSIX(s2n_socket_write(io_context, buf, len, &bytes_written));
    return bytes_written;
}

S2N_RESULT s2n_socket_is_ipv6(int fd, uint8_t *ipv6)
{
    S2N_ERROR_IF_NULL(ipv6);

    socklen_t len;
    struct sockaddr_storage addr;
    len = sizeof (addr);
    GUARD_AS_RESULT(getpeername(fd, (struct sockaddr*)&addr, &len));

    *ipv6 = 0;
    if (AF_INET6 == addr.ss_family) {
       *ipv6 = 1;
    }

    return S2N_RESULT_OK;
}
