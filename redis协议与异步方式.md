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
发布者用 PUBLISH 向某个频道（channel）发消息；订阅者用 SUBSCRIBE（或 PSUBSCRIBE）接收。消息按发送顺序推送给订阅者。
### SUBSCRIBE channel

<img width="281" height="74" alt="image" src="https://github.com/user-attachments/assets/814952ad-ba6e-4f5e-a7ca-d20ed246a31d" />
