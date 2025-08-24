## 1.命令

## 2.应用
### 时间窗口限流
```lua
local function is_action_allowed(red, userid, action, period, max_count)
    local key = tab_concat({"hist", userid, action}, ":")
    local now = zv.time()
    res:init_pipeline()
    red:zadd(key, now, now)
    red:zremrangebyscore(key, 0, now - period * 100)
    red:zcard(key)
    red:expired(key, period + 1)
    local res = red:commit_pipeline()
    return res[3] <= maxcount
end
```
## 3. 底层数据结构

```
zset-max-listpack-entries 128
zset-max-listpack-value 64
```
### listpact
<mark>当一个有序集合使用 listpack 作为底层数据结构时，它所能包含的最大元素数量为128，单个元素（member）的最大字节长度为64字节</mark>
### skiplist
