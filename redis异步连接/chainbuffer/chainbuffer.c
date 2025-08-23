#include "chainbuffer/chainbuffer.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#ifndef BUF_DEFAULT_NODE
#define BUF_DEFAULT_NODE 4096
#endif

/* ---------- 内部工具 ---------- */
static cb_node_t* node_new(size_t cap){
    cb_node_t* n = (cb_node_t*)malloc(sizeof(cb_node_t) + cap);
    if (!n) return NULL;
    n->next = NULL; 
    n->off = 0; 
    n->len = 0; 
    n->cap = cap;
    return n;
}
static void node_free(cb_node_t* n){ 
    free(n); 
}

static void consume_front(buffer_t *b, size_t n){
    /* 消费并回收前缀 n 字节（n<=b->size） */
    size_t need = n;
    while (b->head && need){
        cb_node_t *h = b->head;
        size_t take = (h->len < need) ? h->len : need;
        h->off += take; h->len -= take;
        b->size -= take; need -= take;
        if (h->len == 0){
            b->head = h->next;
            if (b->tail == h) b->tail = NULL;
            node_free(h);
        }
    }
}

/* ---------- 生命周期 ---------- */
void buffer_init(buffer_t *b, size_t default_cap){
    if (!b) return;
    b->head = b->tail = NULL;
    b->size = 0;
    b->default_cap = default_cap ? default_cap : BUF_DEFAULT_NODE;
    b->fd = -1;
}
void buffer_free(buffer_t *b){
    if (!b) return;
    cb_node_t *n = b->head;
    while (n){ cb_node_t *nx = n->next; node_free(n); n = nx; }
    b->head = b->tail = NULL; b->size = 0;
}

/* ---------- 统计/基本操作 ---------- */
size_t buffer_len(const buffer_t *b){ 
    return b ? b->size : 0; 
}

static size_t tail_space(const buffer_t *b){
    if (!b || !b->tail) return 0;
    const cb_node_t *t = b->tail;
    size_t used_end = t->off + t->len;
    return used_end < t->cap ? (t->cap - used_end) : 0;
}
static uint8_t* tail_ptr_make_room(buffer_t *b, size_t min_need, size_t *space_out){
    if (!b) return NULL;
    size_t space = tail_space(b);
    if (space >= min_need){
        cb_node_t *t = b->tail;
        if (space_out) *space_out = space;
        return t->data + t->off + t->len;
    }
    size_t need = (min_need > b->default_cap) ? min_need : b->default_cap;
    cb_node_t *n = node_new(need);
    if (!n) return NULL;
    if (!b->head) b->head = b->tail = n;
    else { b->tail->next = n; b->tail = n; }
    if (space_out) *space_out = n->cap;
    return n->data;
}

size_t buffer_add(buffer_t *b, const void *src, size_t len){
    if (!b || !src || !len) return 0;
    const uint8_t *s = (const uint8_t*)src;
    size_t left = len;
    while (left){
        size_t space; uint8_t *p = tail_ptr_make_room(b, left, &space);
        if (!p) break;
        size_t n = (space < left) ? space : left;
        memcpy(p, s, n);
        b->tail->len += n; b->size += n;
        s += n; left -= n;
    }
    return len - left;
}

size_t buffer_peek(const buffer_t *b, void *dst, size_t len){
    if (!b || !dst || !len) return 0;
    size_t need = (len < b->size) ? len : b->size;
    uint8_t *d = (uint8_t*)dst;
    for (cb_node_t *p = b->head; p && need; p = p->next){
        if (!p->len) continue;
        size_t take = (p->len < need) ? p->len : need;
        memcpy(d, p->data + p->off, take);
        d += take; need -= take;
    }
    return (len < b->size) ? len : b->size;
}

size_t buffer_read(buffer_t *b, void *dst, size_t len){
    if (!b || !dst || !len) return 0;
    size_t want = (len < b->size) ? len : b->size;
    size_t copied = 0;
    while (copied < want){
        cb_node_t *h = b->head;
        if (!h) break;
        size_t take = h->len;
        if (take > (want - copied)) take = (want - copied);
        memcpy((uint8_t*)dst + copied, h->data + h->off, take);
        copied += take;
        consume_front(b, take);
    }
    return copied;
}

void buffer_consume(buffer_t *b, size_t len){
    if (!b || !len) return;
    if (len > b->size) len = b->size;
    consume_front(b, len);
}

/* ---------- iovec 视图 ---------- */
int buffer_iovec(const buffer_t *b, struct iovec *vec, int max){
    if (!b || !vec || max <= 0) return 0;
    int n = 0;
    for (cb_node_t *p = b->head; p && n < max; p = p->next){
        if (!p->len) continue;
        vec[n].iov_base = (void*)(p->data + p->off);
        vec[n].iov_len  = p->len;
        ++n;
    }
    return n;
}

/* ---------- pullup：把前 n 字节拉直在 head ---------- */
int buffer_pullup(buffer_t *b, size_t n){
    if (!b) return -1;
    if (n == 0) return 0;
    cb_node_t *h = b->head;
    if (!h) return -1;
    if (h->len >= n) return 0;

    /* 头结点容量足够 → 原地合并 */
    if (h->cap >= n){
        if (h->off && h->len) memmove(h->data, h->data + h->off, h->len);
        h->off = 0;
        size_t need = n - h->len;
        cb_node_t *p = h->next;
        while (p && need){
            size_t take = (p->len < need) ? p->len : need;
            memcpy(h->data + h->len, p->data + p->off, take);
            h->len += take; need -= take;
            p->off += take; p->len -= take; /* 仅搬运，不改 b->size */
            if (p->len == 0){
                h->next = p->next;
                if (b->tail == p) b->tail = h;
                cb_node_t *dead = p; p = p->next; node_free(dead);
            }else{
                break;
            }
        }
        return (h->len >= n) ? 0 : -1;
    }

    /* 否则新建一个能容纳 n 的节点，复制前缀 */
    size_t cap = (n > b->default_cap) ? n : b->default_cap;
    cb_node_t *newh = node_new(cap);
    if (!newh) return -1;

    size_t copied = 0;
    while (b->head && copied < n){
        cb_node_t *x = b->head;
        size_t take = x->len;
        if (take > (n - copied)) take = (n - copied);
        memcpy(newh->data + copied, x->data + x->off, take);
        copied += take;
        x->off += take; x->len -= take; /* 不改 b->size */
        if (x->len == 0){
            b->head = x->next;
            if (b->tail == x) b->tail = NULL;
            node_free(x);
        }
    }
    newh->off = 0; newh->len = copied;
    newh->next = b->head;
    b->head = newh;
    if (!b->tail) b->tail = newh;
    return (copied >= n) ? 0 : -1;
}

uint8_t* buffer_write_atmost(buffer_t *b){
    if (!b || !b->head || b->head->len == 0) return NULL;
    return b->head->data + b->head->off;
}

size_t buffer_chunk_len(const buffer_t *b){
    if (!b || !b->head) return 0;
    return b->head->len;
}

ssize_t buffer_drain_to_fd(buffer_t *b, int fd, size_t max_bytes){
    if (!b) { errno = EINVAL; return -1; }
    if (fd < 0) { errno = EBADF; return -1; }
    if (b->size == 0) return 0;

    struct iovec vec[32];
    int nvec = 0; size_t total = 0;
    for (cb_node_t *p = b->head; p && nvec < (int)(sizeof(vec)/sizeof(vec[0])); p = p->next){
        if (!p->len) continue;
        size_t left = (max_bytes == (size_t)-1) ? SIZE_MAX : (max_bytes - total);
        if (!left) break;
        size_t take = p->len < left ? p->len : left;
        vec[nvec].iov_base = (void*)(p->data + p->off);
        vec[nvec].iov_len  = take;
        ++nvec; total += take;
    }
    if (!nvec) return 0;

    ssize_t n = writev(fd, vec, nvec);
    if (n <= 0) return -1;              /* EAGAIN/INTR 交给上层 */

    /* 消费已写字节 */
    consume_front(b, (size_t)n);
    return n;
}
