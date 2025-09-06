# 1.什么是Redis
## 1.1Redis介绍
<mark>**远程字典服务：Remote Dictionary Server**</mark>,是一个开源的、<mark>基于内存的高性能数据库、数据结构数据库和KV数据库</mark>
### 1.1.1 内存数据库
- 存储介质: 所有数据常驻内存运⾏，区别于关系型数据库主要数据存储在磁盘的特点
- 持久化机制: ⽀持数据持久化到磁盘，但内存中始终保留完整数据副本
- 性能优势: 内存访问速度⽐磁盘快3-4个数量级，适合⾼性能场景
### 1.1.2 KV数据库
- 存储模型: 采⽤key-value键值对存储结构，通过唯⼀key定位数据
- 访问⽅式: 所有操作都基于key进⾏，包括数据的写⼊、读取和删除
- 设计优势: 简单直接的存储模型带来极⾼的读写效率
### 1.1.3 数据结构

- String: 最基本的键值类型，value可以是字符串或数字
- List: 链表结构，⽀持双向操作，可⽤于实现队列/栈
- Hash: 字段-值映射表，适合存储对象属性
- Set: ⽆序不重复集合，⽀持交并差运算
- ZSet: 带排序权重的集合，每个元素关联⼀个score⽤于排序

# 2. Redis的应用场景
## 2.1 string

### 2.1.1 操作
基本操作：
```bash
SET key val: 设置键值对
GET key: 获取键对应的值
```
原⼦操作:
```bash
INCR key: 原⼦加1
INCRBY key increment: 原⼦加指定整数
DECR key: 原⼦减1
DECRBY key decrement: 原⼦减指定整数
```
条件操作:
```bash
SETNX key value: 当key不存在时才设置（返回1表示成功，0表示失败）
DEL key: 删除键值对
```
位操作:
```bash
SETBIT key offset value: 设置/清空字符串在offset处的bit值
GETBIT key offset: 获取字符串在offset处的bit值
BITCOUNT key: 统计字符串中被设置为1的bit数
```
### 2.1.2 应用
#### <mark>对象存储： 将对象序列化为JSON字符串存储</mark>
```bash
SET role:10001 '{["name"]: "mark", ["sex"]: "male", ["age"]: 30}'
GET role:10001
# key设计:使⽤有意义字段：role:10001
```
适⽤场景: 对象属性字段很少修改的情况

#### <mark>累加器</mark>
```bash
# 统计阅读数
INCR reads 
INCRBY reads 100
```
#### <mark>分布式锁</mark>

```bash
# 非公平锁
SETNX lock 1  # 排他性: 通过SETNX实现（成功返回1，失败返回0）
SETNX lock uuid
SET lock uuid nx ex 30 # 设置过期时间防⽌死锁

#释放锁
DEL lock
if (get(lock) == uuid)
    del(lock);
```

#### <mark>位运算（位统计）</mark>
```bash
# Redis 的 String 在位运算时，本质上是按字节数组（二进制）处理，所以既不是单纯 int，也不是单纯 string
# ⼆进制安全字符串
# ⽉签到功能
SETBIT sign:10001:202106 1 1 # ⽤户10001在2021年6⽉第1天签到
BITCOUNT sign:10001:202106 # 统计当⽉签到次数
GETBIT sign:10001:202106 2 # 查询第2天是否签到
```

## 2.2 list
### 2.2.1 操作
```bash
LPUSH # 从队列左侧⼊队⼀个或多个元素
LPUSH key value [value...]

LPOP # 从队列左侧弹出⼀个元素
LPOP key

RPUSH # 从队列右侧⼊队⼀个或多个元素，命令格式为
RPUSH key value [value...]

RPOP # 从队列右侧弹出⼀个元素
RPOP key

LRANGE# 返回队列中start到end之间的元素，⽀持正负索引（0表示第⼀个元素，-1表示最后⼀个元素）
LRANGE key start end

LREM # 从列表中移除前count次出现的值为value的元素
LREM key count value

BRPOP # RPOP的阻塞版本，当列表为空时会阻塞连接直到超时或有元素可弹出，常⽤于实现延时队列
```

### 2.2.2 应用
#### <mark>栈</mark>

```
# 先进后出FILO
LPUSH + LPOP
或者
RPUSH + RPOP
```

#### <mark>队列</mark> 

```
# 先进先出FIFO
LPUSH + RPOP
或者
RPUSH + LPOP
```
#### <mark>阻塞队列</mark>  
```
LPUSH + BRPOP
或者
RPUSH + BLPOP
```
#### <mark>异步消息队列</mark> 

<img width="288" height="67" alt="image" src="https://github.com/user-attachments/assets/7dedce0a-1b64-4e34-8e8e-4026cc7bb197" />

#### <mark>固定窗⼝记录</mark>

```
LPUSH record xxx
LPUSH record xx
...

LTRIM record 0 49
LRANGE record 0 -1
```

## 2.3 hash
### 2.3.1 操作

```bash
HGET key field # 通过key和field获取对应value
HSET key field value # 设置单个键值对

HMSET key field1 value1 field2 value2... # 设置多个键值对
HMGET key field1 field2... # 获取多个field的值

HINCRBY key field increment # 对field值进⾏原⼦增减操作（如战绩统计）

HLEN key # 获取键值对数量
HDEL key field # 删除指定field
```
### 2.3.2 应用
#### <mark>对象存储 </mark> 
```bash
# string: SET role:10001 '{["name"]: "mark", ["sex"]: "male", ["age"]: 30}'
HSET role:10001 name mark sex male age 30 # ⽀持直接修改特定字段（如HSET role:10001 age 19）
# 对象属性值经常变更的情况
```
#### <mark>购物车 </mark> 
```bash
# 添加商品
HSET MyCart:10001 40001 1
# 修改数量
HINCRBY MyCart:10001 40001 ±1
# 删除商品
HDEL MyCart:10001 40001
# 统计总数
HLEN MyCart:10001
```

## 2.4 set
### 2.4.2 操作
```bash
#  添加⼀个或多个指定member元素到集合key中
SADD key member [member..]

# 计算集合元素个数
SCARD key

# 返回集合中所有成员
SMEMBERS key

# 判断成员是否在集合中
SISMEMBER key member

# 随机返回集合中⼀个或多个元素(不删除)
SRANDMEMBER key [count]

# 从集合中移除并返回随机元素
SPOP key [count]

#  返回集合与给定集合的差集
SDIFF key [key ...]

# 返回所有集合的交集
SINTER key [key ...]

# 返回多个集合的并集
SUNION key [key ...]
```
### 2.4.2 应用

#### <mark>抽奖系统 </mark> 
```bash
# 添加抽奖⽤户
SADD Award:1 1001 1002 1003 1004 1005 1006
SADD Award:1 1009

# 随机抽取多位幸运⽤户
SRANDMEMBER Award:1 10

# 抽取一等奖1人，二等奖2名 三等奖 3名
SPOP Award:1 1
SPOP Award:1 2
SPOP Award:1 3 
```
#### <mark>共同关注 </mark> 
```bash
# 添加抽奖⽤户
SADD follow:1 a b c g i o
SADD follow:2 a y q l c

SINTER follow:1 follow:2
```

#### <mark>推荐好友 </mark> 
```bash
# 添加抽奖⽤户
SADD follow:1 a b c g i o
SADD follow:2 a b c

SDIFF follow:1 follow:2
```
