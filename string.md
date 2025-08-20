
## 1. SET GET DEL
基本操作<br>
<br>
<br>
## 2. APPEND key value
### 含义
- 将字符串 value 附加到 key 原有的值末尾，如果 key 不存在，则新建该 key 并赋值为 value
- 返回的是 key 对应新字符串的总长度（以字节为单位）

### 实例
<img width="221" alt="image" src="https://github.com/user-attachments/assets/bb68a2e0-9bef-4cef-8567-8394eb31e363" /><br>

## 3. INCR key
### 含义
- 字符串操作命令，用于对键 key 的值执行 INCR 是原子的，自增操作，只增加 1
- 如果键不存在，Redis 会将其初始化为 0，然后执行 +1 操作
- 如果键的值不是一个整数格式（如 "abc"），会报错
- 增加后的值将作为结果返回
- INCR 只能用于整数操作（64 位有符号整型）

### 实例
<img width="325" alt="image" src="https://github.com/user-attachments/assets/f83a5b68-28ce-42df-b04a-0ee2d8d4f868" />

## 4. DECR key<br>
### 含义
- 对整数值递减 1的命令，它也是原子操作，与INCR相反

### 实例
<img width="233" alt="image" src="https://github.com/user-attachments/assets/58405fe3-eff2-44e6-bc3e-60f2fc0d00fc" />

## 5. INCRBY key increment
### 含义
- increment为增加的整数（可以为负数，相当于递减）

### 实例
<img width="216" alt="image" src="https://github.com/user-attachments/assets/03ceae95-7f75-483f-a25f-7e3cabe48c02" />

## 6. DECRBY key decrement
### 含义
- 用法同INCYBY一样
### 实例

<img width="227" alt="image" src="https://github.com/user-attachments/assets/1a853a80-a7c8-487d-9b1c-dbfd3509c3b4" />

## 7. SETNX key value<br>
### 含义
- 只有当 key 不存在时，才设置它的值<br>
- 如果 key 不存在，则设置为 "value"，并返回 1（成功）<br>
- 如果 key 已存在，则不做任何操作，返回 0（失败）<br>

### 实例
<img width="197" alt="image" src="https://github.com/user-attachments/assets/85a66574-5ad5-44ae-8870-872f984c91b5" />

## 8. SETBIT key offset value
### 含义
- 将某个字符串键（key）上某个 位（bit） 的值设置为 0 或 1。这是操作 二进制位（bitmaps） 的基础命令之一
- key：要设置的字符串键<br>
- offset：从字符串开头计算的位偏移量（以 bit 为单位，从 0 开始）<br>
- value：只能是 0 或 1<br>

### 实例

<img width="322" alt="image" src="https://github.com/user-attachments/assets/91a6f59c-ab73-42d6-b6bc-9d941af9608e" />

## 9. GETBIT key offset
### 含义
- 获取某个字符串键在指定偏移位置上的位（bit）值<br>

### 实例
<img width="322" alt="image" src="https://github.com/user-attachments/assets/91a6f59c-ab73-42d6-b6bc-9d941af9608e" /><br>

## 10. BITCOUNT key [start end]<br>
### 含义
- 统计一个字符串键中被设置为 1 的 bit（位）的数量<br>
- start 和 end（可选）：<b><u>按字节（byte）</u></b>范围进行统计，索引从 0 开始。可以为负数（表示从末尾倒数）

### 实例
<img width="320" alt="image" src="https://github.com/user-attachments/assets/5f1563df-9b7d-4fe6-85f2-91f726f90379" />
