## 1. LPUSH key element [element ...]
### 含义
LPUSH 命令用于向一个列表（List）的头部插入一个或多个值：
- 如果 key 不存在，它会先创建一个新的空列表
- 将所有 element 按从左到右的顺序依次插入到这个列表的最前面
- 命令会返回插入操作完成后，列表的新长度

### 实例

<img width="352" height="30" alt="image" src="https://github.com/user-attachments/assets/cad40a7b-de94-4af1-afff-e9250f468290" />

## 2. LRANGE key start stop
### 含义
- 从 Redis 的一个**列表（List）**中获取指定范围内的元素
- 返回从索引 start 到 stop（包括这两个索引）之间的所有元素

### 实例

<img width="233" height="88" alt="image" src="https://github.com/user-attachments/assets/5ff1cbff-5338-4809-b8e8-6cfb30aee7af" />
