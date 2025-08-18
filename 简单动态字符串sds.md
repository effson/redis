# 简单动态字符串sds
SET / GET / APPEND → String
## 源码
### 1.sds定义
> src/sds.c   src/sds.h
```c
// Redis 在内部用 typedef 封装了 sds：
typedef char *sds;

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

### 2.操作函数
```c
#define SDS_HDR(T,s) ((struct sdshdr##T *)((s)-(sizeof(struct sdshdr##T))))

static inline size_t sdslen(const sds s) {
    switch (sdsType(s)) {
        case SDS_TYPE_5: return SDS_TYPE_5_LEN(s);
        case SDS_TYPE_8:
            return SDS_HDR(8,s)->len;
        case SDS_TYPE_16:
            return SDS_HDR(16,s)->len;
        case SDS_TYPE_32:
            return SDS_HDR(32,s)->len;
        case SDS_TYPE_64:
            return SDS_HDR(64,s)->len;
    }
    return 0;
}
```
### 3.SDS与C字符串
#### C 字符串的特点
- 长度获取：必须从头扫描，直到遇到 \0 → strlen() 时间复杂度 O(n)
- 存储内容：不能安全存储二进制数据（中间如果有 \0 会被截断）
- 扩容问题：如果拼接字符串，需要手动 malloc/realloc，并拷贝数据，容易出错（内存泄漏、缓冲区溢出）
- 元信息缺失：只知道指针，无法知道已分配的容量（capacity）

#### SDS 的特点

```css
[ header (len/alloc/flags) ][ buf ... ][ '\0' ]
                        ↑
                        sds 指针指向这里
```

- O(1) 获取长度：直接读取 len 字段，不需要像 C 字符串那样遍历
- 二进制安全：不依赖 \0 作为结束符，可以存储任意字节流（包括 \0）
- 自动扩容：拼接时若空间不足，会自动 realloc，并按策略预留冗余空间（减少频繁分配）
- 惰性缩容：删除内容后不会立刻 free，而是保留多余空间，以便下次追加时复用，避免频繁分配
- 兼容 C API：buf 的末尾依然放一个 \0，保证可以直接传给 printf、strcmp 等 C 函数使用

#### Redis 使用 SDS 的好处
- 性能提升:sdslen() O(1) 获取长度，strlen() 只能 O(n)。大量 key/value 操作时效率显著提升
- 安全性提高,避免 strcat、strcpy 那类 C 字符串常见的缓冲区溢出问题。SDS 内部自己维护容量，确保拼接和追加不会写越界
- 支持二进制安全,Redis 的 key、value 不仅是文本，也可能是图片、压缩包等任意二进制数据。SDS 依赖 len 管理字符串，不会被 \0 干扰
- 减少内存分配次数,空间预分配：扩容时会多分配一部分空间，下次追加直接用。惰性释放：删除数据时不立刻归还内存，后续复用，减少 malloc/free 调用
