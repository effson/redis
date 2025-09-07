# 1. 高可用问题

# 2. 获取和释放锁为同一对象

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

