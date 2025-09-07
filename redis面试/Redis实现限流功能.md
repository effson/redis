# 1.限流
<mark>一段时间内(period)某行为(action)允许的最大操作次数(max_count)</mark>
# 2.实现方式

## 2.1 截断限流(固定窗口限流)
固定窗口内的行为统计
### 2.1.1 图示

<img width="631" height="114" alt="image" src="https://github.com/user-attachments/assets/aa7d7fd8-4900-40a5-8cce-560572ae42d3" />

### 2.1.2 局限性
实际情况中请求不是均匀的，而是随机的，如上图所示。在很小的时间段内出现了18次操作，不能很好限流

### 2.1.3 代码实现
```
incr limits
expire limits 10
```

```
# pipeline
set limits 0 nx ex 10
incr limits
```
## 2.2 滑动窗口限流
```lua
local function is_action_allowed(red, userid, action, period, max_count)
    local key = tab_concat({"hist", userid, action}, ":")
    local now = zv.time()
    red:init_pipeline()

    red:zadd(key, now, now)
    red:zremrangebyscore(key, 0, now - period * 100)

    red:zcard(key)
    red:expire(key, period + 1)
    local res = red:commit_pipeline()
    return  res[3] <= max_count
end
```


## 2.3 漏斗限流


## 2.4 令牌桶限流
