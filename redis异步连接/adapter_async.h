#ifndef _MARK_ADAPTER_
#define _MARK_ADAPTER_

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include "reactor.h"

// Redis事件对象
typedef struct {
    event_t e;              // Reactor事件对象
    int mask;               // 存储注册的事件
    redisAsyncContext *ctx; // hiredis异步上下文
} redis_event_t;

// Redis对象读事件回调
static void redisReadHandler(int fd, int events, void *privdata) {
    (void)fd;
    (void)events;
    event_t *e = (event_t*)privdata;
    redis_event_t *re = (redis_event_t *)(char *)e;
    redisAsyncHandleRead(re->ctx);
}

// Redis对象写事件回调
static void redisWriteHandler(int fd, int events, void *privdata) {
    (void)fd;
    (void)events;
    event_t *e = (event_t*)privdata;
    redis_event_t *re = (redis_event_t *)(char *)e;
    redisAsyncHandleWrite(re->ctx);
}

/**
 * @brief 更新Reactor管理的事件对象
 * @param privdata Redis事件对象
 * @param flag 要设置的epoll事件类型 
 * @param remove 1 删除该事件，0 添加该事件
 */
static void redisEventUpdate(void *privdata, int flag, int remove) {
    redis_event_t *re = (redis_event_t *)privdata;
    reactor_t *r = re->e.r;
    int prevMask = re->mask;
    int enable = 0;             
    // Redis事件对象删除该事件
    if (remove) {
        if ((re->mask & flag) == 0) {
            return;
        }
        re->mask &= ~flag;
        enable = 0;
    } 
    // Redis事件对象添加该事件
    else {
        if (re->mask & flag) {
            return;    
        }           
        re->mask |= flag;
        enable = 1;
    }
    
    // 对Reactor事件对象的处理
    // 1. 删除该事件
    if (re->mask == 0) {
        del_event(r, &re->e);
    } 
    // 2. 添加该事件（首次加入）
    else if (prevMask == 0) {
        add_event(r, re->mask, &re->e);
    } 
    // 3. 修改该事件
    else {
        // 注册读事件
        if (flag & EPOLLIN) {
            enable_event(r, &re->e, enable, 0);
        } 
        // 注册写事件
        else if (flag & EPOLLOUT) {
            enable_event(r, &re->e, 0, enable);
        }
    }
}

// 添加读事件
static void redisAddRead(void *privdata) {
    redis_event_t *re = (redis_event_t *)privdata;
    re->e.read_fn = redisReadHandler;
    redisEventUpdate(privdata, EPOLLIN, 0);
}

// 删除读事件
static void redisDelRead(void *privdata) {
    redis_event_t *re = (redis_event_t *)privdata;
    re->e.read_fn = 0;
    redisEventUpdate(privdata, EPOLLIN, 1);
}

// 添加写事件
static void redisAddWrite(void *privdata) {
    redis_event_t *re = (redis_event_t *)privdata;
    re->e.write_fn = redisWriteHandler;
    redisEventUpdate(privdata, EPOLLOUT, 0);
}

// 删除写事件
static void redisDelWrite(void *privdata) {
    redis_event_t *re = (redis_event_t *)privdata;
    re->e.write_fn = 0;
    redisEventUpdate(privdata, EPOLLOUT, 1);
}

// 释放Redis事件对象
static void redisCleanup(void *privdata) {
    redis_event_t *re = (redis_event_t *)privdata;
    reactor_t *r = re->e.r;
    del_event(r, &re->e);
    free(re);
}

// 绑定Redis异步上下文到Reactor
static int redisAttach(reactor_t *r, redisAsyncContext *ac) { 
    redisContext *c = &(ac->c); // 获取Redis同步上下文
    redis_event_t *re;          // Redis事件对象
    
    /* 确保没有重复绑定 */
    if (ac->ev.data != NULL)
        return REDIS_ERR;

    /* 创建容器用于ctx和读写事件 */
    re = (redis_event_t*)malloc(sizeof(*re));
    if (re == NULL) {
        return REDIS_ERR;
    }  

    // 初始化Redis事件对象
    re->ctx = ac;       // 绑定Redis异步上下文
    re->e.fd = c->fd;   // 绑定Redis的fd
    re->e.r = r;        // 绑定Reactor
    re->mask = 0;       // 初始化事件掩码

    // 设置hiredis的事件接口，用户实现这些接口
    ac->ev.addRead = redisAddRead;
    ac->ev.delRead = redisDelRead;
    ac->ev.addWrite = redisAddWrite;
    ac->ev.delWrite = redisDelWrite;
    ac->ev.cleanup = redisCleanup;
    ac->ev.data = re;

    return REDIS_OK;
}

#endif
