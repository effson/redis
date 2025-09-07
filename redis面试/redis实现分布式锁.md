# 1. 高可用问题

## 哨兵模式

## cluster模式

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
