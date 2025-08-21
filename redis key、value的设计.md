## 1. key的设计
### Redis 的 key 始终是一个字符串string
一个功能对应一个key:
```bash
set teacher xxx
set func xx
set company xxxx
...
```
相同功能多个key:
```bash
以冒号 : 作为分隔符
set teacher:t1 xxx
set teacher:t2  xxx
...

set func:f1 xx
set func:f2 xx
...

set company:c1 xxxx
set company:c2 xxxx
...
```
复杂例子：
```bash
set name:id:activity xxxx xxx ...
```
## 2. value的设计
根据存储的对象的特点选择合适的对象<br>
### 存储用户信息
使用hash：
```bash
hmset custom:1 name alice age 26 sex female
```
### 商品购物车
hash + list:
```bash
# 添加商品
hmset Mychart:10001 40001 1 cost 5099 desc "laptop" 
lpush MyItem:10001 40001

# 增加/减少数量
hincrby Mychart:10001 40001 1
hincrby Mychart:10001 40001 -1

#显示所有物品数量
hlen Mychart:10001
```
