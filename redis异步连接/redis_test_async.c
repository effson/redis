#define _POSIX_C_SOURCE 200809L
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <time.h>

#include "reactor.h"
#include "adapter_async.h"

static reactor_t *R;
static int cnt, before, num;

// 获取当前时间的毫秒数
int current_tick() {
    int t = 0;
    struct timespec ti;
    clock_gettime(CLOCK_MONOTONIC, &ti);
    t = (int)ti.tv_sec * 1000;
    t += ti.tv_nsec / 1000000;
    return t;
}

// 回调函数，用于处理Redis响应
void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    if (reply == NULL) return;
    printf("argv[%s]: %lld\n", (char*)privdata, reply->integer);

    /* 当收到所有回复后，断开连接并停止事件循环 */
    cnt++;
    if (cnt == num) {
        int used = current_tick() - before;
        printf("After %d exec redis commands, used %d ms\n", num, used);
        redisAsyncDisconnect(c);
    }
}

// 连接回调函数
void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        stop_eventloop(R);
        return;
    }
    printf("Connected...\n");
}

// 断开连接回调函数
void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        stop_eventloop(R);
        return;
    }

    printf("Disconnected...\n");
    stop_eventloop(R);
}

int main(int argc, char **argv) {
    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    if (c->err) {
        /* 暂时让c泄漏 */
        printf("Error: %s\n", c->errstr);
        return 1;
    }

    R = create_reactor();

    redisAttach(R, c);

    redisAsyncSetConnectCallback(c, connectCallback);
    redisAsyncSetDisconnectCallback(c, disconnectCallback);

    before = current_tick();

    num = (argc > 1) ? atoi(argv[1]) : 1000;

    for (int i = 0; i < num; i++) {
        redisAsyncCommand(c, getCallback, "count", "INCR counter");
    }

    eventloop(R);

    release_reactor(R);
    return 0;
}
