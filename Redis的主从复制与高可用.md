## 1. Redis淘汰策略
### maxmemory
maxmemory 参数用于配置 Redis 实例可以使用的最大内存量。当 Redis 占用的内存超过这个限制时，就会触发内存淘汰机制或报错

### maxmemory-policy
maxmemory-policy 用于配置当 maxmemory 限制被触发时，Redis 应如何选择要淘汰的键（key）。不同的策略会影响数据淘汰的效率和准确性<br>

<mark>对过期KEY：</mark>
- volatile-lru：从设置了过期时间（EXPIRE）的键中，淘汰最近最少使用的KEY（Least Recently Used）
- volatile-lfu：从设置了过期时间的键中，淘汰最近使用频率最少的KEY（Least Frequently Used）
- volatile-ttl：只从设置了过期时间的键中，淘汰剩余生存时间（Time To Live, TTL）最短的KEY
- volatile-random：只从设置了过期时间的键中，随机淘汰

<mark>对全部KEY：</mark>
- allkeys-lru：从所有键中，淘汰最近最少使用的键
- allkeys-lfu：从所有键中，淘汰最近使用频率最少的键
- allkeys-random：从所有键中，随机淘汰

<mark>禁止淘汰：</mark>
- noeviction：不淘汰任何键。当达到内存上限时，所有写操作（如 SET, INCR）都会返回错误，但读操作（如 GET）仍然可用

### maxmemory-samples
maxmemory-samples 用于配置 LRU（最近最少使用）和 LFU（最不常用）算法的取样数量。因为要准确找出整个数据集中“最少使用”的键成本非常高，
所以 Redis 采用了一种近似算法。它会从数据集中随机抽取一定数量的键，然后从这些样本中选择最符合淘汰条件的键进行淘汰
- 作用：设置 LRU 或 LFU 算法的样本数
- 取值范围：通常在 3 到 10 之间。默认值为 5
- 样本数越大，淘汰算法的准确性就越高，越能淘汰掉真正的“最少使用”键

## 2. Redis的持久化
### AOF


### RDB
