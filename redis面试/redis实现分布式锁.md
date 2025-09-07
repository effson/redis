# 1. 高可用问题
锁本身是一种资源
## 哨兵模式

## cluster模式
- 使用主从异步复制，数据容易丢失
- redis只能实现非公平锁

# 2. 获取和释放锁为同一对象
使用uuid

# 3. 互斥的语义
## 加锁
```bash
SETNX lock uuid
```
## 释放锁
```bash
if uuid == holder.uuid then
    DEL lock
end
```
# 4. 锁超时
```bash
SET lock uuid NX EX 10
```
