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

## 3. RPUSH key element [element ...]
### 含义
- 如果 key 不存在，Redis 会先创建一个新的空列表
- 将所有 element 按从左到右的顺序依次插入到这个列表的最末端
- 操作完成后，命令会返回列表的新长度

### 实例

<img width="257" height="159" alt="image" src="https://github.com/user-attachments/assets/fb5ddb87-2143-449b-8eea-20c0b7552d6e" />

## 4. LLEN key
### 含义
- 返回链表的长度，也就是元素个数

### 实例

<img width="170" height="27" alt="image" src="https://github.com/user-attachments/assets/2617928e-3ccc-4d3e-acb3-3dbbb1ec57ea" />

## 5. LPOP key [count]
### 含义
- 不带 count 参数：这是 LPOP 的基本用法。它会从列表的头部移除一个元素，并将其返回
- 带 count 参数（Redis 6.0+）：如果你指定了一个正整数 count，LPOP 会从列表头部移除多个元素，并将它们作为一个列表返回。它会尽量返回 count 个元素，但如果列表中的元素不足 count 个，它会返回所有剩余的元素

### 实例

<img width="187" height="86" alt="image" src="https://github.com/user-attachments/assets/cdd6a213-7ed9-4d51-895e-37e5340be2a1" />

## 6. RPOP key [count]
### 含义
- 不带 count 参数：这是 RPOP 的基本用法。它会从列表的尾部移除一个元素，并将其返回
- 带 count 参数（Redis 6.0+）：如果你指定一个正整数 count，RPOP 会从列表尾部移除多个元素，并将它们作为一个列表返回。如果列表中元素不足 count 个，它会返回所有剩余的元素

### 实例

<img width="179" height="90" alt="image" src="https://github.com/user-attachments/assets/8517ac75-0ef7-4200-a75c-e62db30960eb" />

## 7. BLPOP key [key ...] timeout
