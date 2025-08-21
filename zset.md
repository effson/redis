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
