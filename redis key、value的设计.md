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
