### 编译执行

```bash
root@worker02:/home/jeff/redis# gcc -I.  chainbuffer/chainbuffer.c redis_test_async.c reactor.c -o redis-test-async -lhiredis
```

### 同步时延结果
发送1000条命令：

<img width="304" height="16" alt="image" src="https://github.com/user-attachments/assets/1560df62-f466-4dc3-8540-1e7a76e771f8" />


### 异步时延结果
发送1000条命令：

<img width="263" height="20" alt="image" src="https://github.com/user-attachments/assets/cb14d1e1-cf7d-415f-8496-535a2dc5e1d5" />
