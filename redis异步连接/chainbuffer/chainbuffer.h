#ifndef CHAINBUFFER_BUFFER_H
#define CHAINBUFFER_BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <sys/uio.h>   /* struct iovec */
#include <sys/types.h> /* ssize_t */

#ifdef __cplusplus
extern "C" {
#endif

/* 单个节点（柔性数组承载数据） */
typedef struct cb_node_s {
    struct cb_node_s *next;
    size_t off;     /* 已消费偏移 */
    size_t len;     /* 有效数据长度（从 off 开始） */
    size_t cap;     /* data[] 总容量 */
    uint8_t data[]; /* 柔性数组 */
} cb_node_t;

/* 外部使用的缓冲对象（注意：按你的用法是“栈上实例+init/free”） */
typedef struct buffer_s {
    cb_node_t *head;
    cb_node_t *tail;
    size_t size;         /* 总可读字节数 */
    size_t default_cap;  /* 新节点默认容量（如 4096、16K 等） */
    int fd;              /* 可选：绑定的 fd，buffer_write_atmost() 用 */
} buffer_t;

/* 生命周期（与你的调用保持一致） */
void   buffer_init (buffer_t *b, size_t default_cap);
void   buffer_free (buffer_t *b);

/* 统计/基本操作 */
size_t buffer_len  (const buffer_t *b);                 /* = b->size */
size_t buffer_add  (buffer_t *b, const void *src, size_t len); /* 追加写入 */
size_t buffer_peek (const buffer_t *b, void *dst, size_t len); /* 复制但不消费 */
size_t buffer_read (buffer_t *b, void *dst, size_t len);       /* 复制并消费 */
void   buffer_consume(buffer_t *b, size_t len);                /* 仅消费 */

uint8_t* buffer_write_atmost(buffer_t *b);
size_t   buffer_chunk_len(const buffer_t *b);
ssize_t  buffer_drain_to_fd(buffer_t *b, int fd, size_t max_bytes);

int buffer_iovec(const buffer_t *b, struct iovec *vec, int max);

int buffer_pullup(buffer_t *b, size_t n);

#ifdef __cplusplus
}
#endif
#endif
