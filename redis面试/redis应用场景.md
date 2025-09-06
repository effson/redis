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

### 2.1.1 操作:
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
### 2.1.2 应用:
对象存储： 将对象序列化为JSON字符串存储
```bash
SET role:10001 '{["name"]: "mark", ["sex"]: "male", ["age"]: 30}'
```
