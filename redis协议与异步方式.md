## 1. Redis Pipeline
### 定义
- Pipeline 是一种“把多条命令一次性发到服务器、再成批读回回复”的用法
- 不改变命令在服务端的执行语义（仍然顺序、一条条执行），是客户端的行为，和服务器无关
- 显著减少网络往返 RTT 次数，提升吞吐并降低总体延迟
### 工作机制（底层原理）
- 1. 客户端先把多条命令缓存在本地发送缓冲区；
- 2. 一次 write() 全部发给 Redis；
- 3. Redis 仍按到达顺序逐条执行，并把每条结果写回 socket；
- 4. 客户端一次或分批 read()，按相同顺序取回所有结果。 回复顺序与发送顺序一一对应，没有乱序与并发交错。
 
## 2. 发布/订阅模式
发布者用 PUBLISH 向某个频道（channel）发消息；订阅者用 SUBSCRIBE（或 PSUBSCRIBE）接收。消息按发送顺序推送给订阅者。仅做实时广播，不会保存消息。
### 2.1 SUBSCRIBE [channel]
在三个客户端订阅频道：

<img width="281" height="74" alt="image" src="https://github.com/user-attachments/assets/814952ad-ba6e-4f5e-a7ca-d20ed246a31d" />

### 2.2 PUBLISH [channel] [content]

<img width="1018" height="584" alt="image" src="https://github.com/user-attachments/assets/dce92e97-3ce1-4c54-9956-0d7b70463f95" />

### 2.3 PSUBSCRIBE [channel]
模式匹配

<img width="280" height="67" alt="image" src="https://github.com/user-attachments/assets/d87bab49-5161-49e8-aa1e-6131a3e58930" />

<img width="1027" height="576" alt="image" src="https://github.com/user-attachments/assets/b5d55b60-a844-4ae5-8df0-9dfae1de6f5b" />

# 3. redis事务
## 什么是事务
- 事务是一组命令的集合，这些命令在执行时被视为一个单一的、不可分割的操作单元
- 要么全部执行，要么全部不执行
## 什么情况下探讨事务
- 单条客户端连接不需要考虑
- 当多个客户端同时访问和修改同一个数据时，可能会发生竞态条件。事务可以有效地防止这类问题
## 为什么需要事务
- 事务能够确保在操作的整个过程中，数据保持逻辑上的一致状态
- 需要基于旧值做决定，且并发很高，担心脏读

## 3.1 ACID
### 3.1.1 A — Atomicity 原子性
- 事务中的操作要么全部成功，要么全部不做；失败时必须能回滚到事务开始前的状态
- Redis不支持回滚，如果命令本身语法正确，但在执行时由于数据类型不匹配等原因导致错误（例如，对一个哈希对象执行 INCR 命令），Redis 仍然会继续执行队列中的其他命令。对于出错的命令，它会返回一个错误信息，但不会回滚之前已经成功执行的命令
### 3.1.2 C — Consistency 一致性
#### 完整约束的一致（Schema / 结构一致性）
- 事务必须遵守数据库本身定义的所有规则。这些规则通常被称为完整性约束（Integrity Constraints），它们是数据库模式（Schema）的一部分。
- 当一个事务执行时，数据库系统会确保所有操作都不会违反这些预设的约束。如果事务中的任何一个操作会导致违反这些约束，数据库会自动拒绝该操作并回滚整个事务<br>

举例：
- 对string类型的对象使用lpush操作会报错
- MySQL某列唯一性约束，不能出现重复项
- 外键约束、非空约束、数据类型约束等等

####  用户逻辑的一致性
应用程序层面的一致性，指的是事务必须遵守业务逻辑上的正确性。这些逻辑规则可能没有直接在数据库的约束中体现，而是由应用程序代码来保证,例如:
- 平衡状态：例如，银行转账中，账户 A 的扣款额必须等于账户 B 的加款额
- 总和不变：例如，库存系统中的商品总库存量必须等于各仓库库存量之和
- 例如“余额不能为负“

### 3.1.3 I — Isolation 隔离性
并发执行的事务互不干扰;Redis是单线程执行，天然具备隔离性

### 3.1.4 D — Durability 持久性
Redis只有在AOF持久化策略的时候，并且需要在redis.conf中appendfsync=always才具备持久性:实际项目几乎不会使用AOF持久化策略<br>
<mark>lua脚本满足原子性和隔离性，一致性和持久性不满足</mark>

## 3.2 核心命令
实际应用中很少使用
### 3.2.1 MULTI
开启事务，进入事务状态，之后的命令不立即执行，只入队<br>

<img width="140" height="43" alt="image" src="https://github.com/user-attachments/assets/b65f62b1-d15a-4d86-bb5a-cd9685bdb24c" />

### 3.2.2 EXEC
MULTI开启事务后，事务执行过程中，单个命令是入队操作，知道调用EXEC 命令才会一起执行

<img width="293" height="151" alt="image" src="https://github.com/user-attachments/assets/2b5188b6-0906-4791-af66-395456d79eb5" />

### 3.2.3 DISCARD
放弃事务，清空命令队列

### 3.2.4 WATCH key
在 EXEC 前如果任一被 watch 的键被别的客户端修改过，EXEC 会直接放弃执行（返回 nil）

## 3.3 lua脚本
### 3.3.1 EVAL
一旦set就无法回滚，所以要先判定后写入，脚本运行中途出错不会撤销已写入的修改
```bash
EVAL 'local key = KEYS[1]; local val = redis.call("get", key); redis.call("set", key, val * 2); return val * 2;' 1 score
```

<img width="197" height="26" alt="image" src="https://github.com/user-attachments/assets/baf22d19-8a7c-4dc3-9c25-41bc57e36d4e" />

<img width="842" height="29" alt="image" src="https://github.com/user-attachments/assets/1b9b26fe-0076-4845-8ee5-6b6543a9acd8" />

```bash
EVAL 'local key = KEYS[1]; local val = redis.call("get", key); if not val then val = 1 end; redis.call("set", key, val * 2);
return val * 2;' 1 score1
```

<img width="1007" height="86" alt="image" src="https://github.com/user-attachments/assets/55b22eba-180a-4ee9-acf2-f6ff8b710b9e" />


### 3.3.2 EVALSHA
```bash
SCRIPT LOAD 'local key = KEYS[1]; local val = redis.call("get", key); if not val then val = 1 end; redis.call("set", key, val * 2); return val * 2;'
```

<img width="1003" height="44" alt="image" src="https://github.com/user-attachments/assets/1b1594be-59c6-4e69-8637-6d45dc23db28" />

```bash
EVALSHA eb36bea95c939777b0034e2f40554d10491500e3 1 score
```
<img width="448" height="43" alt="image" src="https://github.com/user-attachments/assets/c2404b0e-ba24-4bd2-866c-f11b9365e852" />

# 4. Redis异步连接
