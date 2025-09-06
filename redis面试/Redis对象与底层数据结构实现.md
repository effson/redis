```c
typedef struct redisObject {
    unsigned type:4;      // 对象类型（string、list、hash、set、zset）
    unsigned encoding:4;  // 编码方式（底层数据结构）
    unsigned lru:LRU_BITS; // LRU/LFU 信息，用于淘汰策略
    int refcount;          // 引用计数
    void *ptr;             // 指向底层数据结构的指针
} robj;
```

# 1.String
- int：如果内容是整数且能用 long 表示，直接存整数。
- embstr：长度 ≤ 44 字节的短字符串，使用一块连续内存存储 redisObject + SDS，分配效率高。
- raw：长字符串，robj 与 SDS 分离

# 2. List
- ziplist（旧版本 ≤5.0）或 listpack（新版本）——连续内存紧凑存储，适合短小元素
- quicklist（默认）——双向链表 + ziplist 结合，既省内存又支持快速插入/删除
## 2.1 listpack
### 2.1.1 介绍
- 用一块 连续内存 存放若干元素（字符串或整数），追求 极小内存占用
- 作为底层“小容器”被多个上层对象复用（小 hash / 小 zset / quicklist 的节点内元素）

### 2.1.2 内存布局（整体）
```css
+----------------+-------------------+--------------------+---------+
| total_bytes(4) | num_elements( ? ) | entries ...        | 0xFF    |
+----------------+-------------------+--------------------+---------+
```
- total_bytes：整个 listpack 所占字节数（uint32）。
- num_elements：元素个数（新版可为 32 位计数，早期实现可能更小，达到极值会用 “未知数” 标记）。
- entries...：一条条紧挨着的变长 entry。
- 终止符 0xFF：结束标记。
