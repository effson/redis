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
## 3.1 什么是事务
- 事务是一组命令的集合，这些命令在执行时被视为一个单一的、不可分割的操作单元
- 要么全部执行，要么全部不执行
## 3.2 什么情况下探讨事务
- 单条客户端连接不需要考虑
- 当多个客户端同时访问和修改同一个数据时，可能会发生竞态条件。事务可以有效地防止这类问题
## 3.3 为什么需要事务
- 事务能够确保在操作的整个过程中，数据保持逻辑上的一致状态
- 需要基于旧值做决定，且并发很高，担心脏读

## 3.2 ACID
### 3.2.1 A — Atomicity 原子性
- 事务中的操作要么全部成功，要么全部不做；失败时必须能回滚到事务开始前的状态
- Redis不支持回滚，如果命令本身语法正确，但在执行时由于数据类型不匹配等原因导致错误（例如，对一个哈希对象执行 INCR 命令），Redis 仍然会继续执行队列中的其他命令。对于出错的命令，它会返回一个错误信息，但不会回滚之前已经成功执行的命令
### 3.2.2 C — Consistency 一致性
#### 完整约束的一致（Schema / 结构一致性）
- 事务必须遵守数据库本身定义的所有规则。这些规则通常被称为完整性约束（Integrity Constraints），它们是数据库模式（Schema）的一部分。
- 当一个事务执行时，数据库系统会确保所有操作都不会违反这些预设的约束。如果事务中的任何一个操作会导致违反这些约束，数据库会自动拒绝该操作并回滚整个事务
举例：
- 对string类型的对象使用lpush操作会报错
- MySQL某列唯一性约束，不能出现重复项
- 外键约束、非空约束、数据类型约束等等
