# 简单动态字符串sds

## 源码
> src/sds.c   src/sds.h
```c
/* Note: sdshdr5 is never used, we just access the flags byte directly.
 * However is here to document the layout of type 5 SDS strings. */
struct __attribute__ ((__packed__)) sdshdr5 {
    unsigned char flags; /* 3 lsb of type, and 5 msb of string length */
    char buf[];
};
struct __attribute__ ((__packed__)) sdshdr8 {
    uint8_t len; /* 字符串长度 */
    uint8_t alloc; /* 分配的总空间 */
    unsigned char flags; /* 类型标识（低 3 位） */
    char buf[]; /*字符数组*/
};

```
sdshdr16、sdshdr32、sdshdr64一样的定义
