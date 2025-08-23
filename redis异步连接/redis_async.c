// gcc -I.  chainbuffer/chainbuffer.c redis_async.c reactor.c -o redis-async -lhiredis
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <time.h>

#include "reactor.h"
#include "adapter_async.h"

static reactor_t *R;

static const char *rtype[] = {
    "^o^",     // 0: 未定义/占位
    "STRING",  // 1: REDIS_REPLY_STRING
    "ARRAY",   // 2: REDIS_REPLY_ARRAY
    "INTEGER", // 3: REDIS_REPLY_INTEGER
    "NIL",     // 4: REDIS_REPLY_NIL
    "STATUS",  // 5: REDIS_REPLY_STATUS
    "ERROR",   // 6: REDIS_REPLY_ERROR
    "DOUBLE",  // 7: REDIS_REPLY_DOUBLE
    "BOOL",    // 8: REDIS_REPLY_BOOL
    "MAP",     // 9: REDIS_REPLY_MAP
    "SET",     // 10: REDIS_REPLY_SET
    "ATTR",    // 11: REDIS_REPLY_ATTR
    "PUSH",   // 12: REDIS_REPLY_PUSH
    "BIGNUM",  // 13: REDIS_REPLY_BIGNUM
    "VERB"     // 14: REDIS_REPLY_VERB
};

void dumpReply(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = (redisReply *)r;
    switch(reply->type) {
    case REDIS_REPLY_STATUS:
    case REDIS_REPLY_STRING:
        printf("[req = %s] reply:(%s) %s \n", (char *)privdata, rtype[reply->type], reply->str);
        break;
    case REDIS_REPLY_NIL:
        printf("[req = %s] reply:(%s)nil\n", (char *)privdata, rtype[reply->type]);
        break;
    case REDIS_REPLY_INTEGER:
        printf("[req = %s] reply:(%s)%lld\n", (char *)privdata, rtype[reply->type], reply->integer);
        break;
    case REDIS_REPLY_ARRAY:
        printf("[req = %s] reply(%s): number of elements = %1u\n", (char *)privdata, rtype[reply->type], reply->elements);
        for (int i = 0; i < reply->elements; i++) {
            printf("\t %1u : %s\n", i, reply->element[i]->str);
        }
        break;        
    case REDIS_REPLY_ERROR:
        printf("[req = %s] reply(%s): ERROR = %s\n", (char *)privdata, rtype[reply->type], reply->str);
        break;
    default:
        printf("[req = %s] reply(%s)\n", (char *)privdata, rtype[reply->type]);
        break;
    }
}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        stop_eventloop(R);
        return;
    }
    printf("Connected...\n");
    redisAsyncCommand((redisAsyncContext*)c, dumpReply, 
        "hmset role:10001", "hmset role:10001 name jeff age 29 sex male");
    redisAsyncCommand((redisAsyncContext*)c, dumpReply, 
        "hgetall role:10001", "hgetall role:10001");
}

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
    R = create_reactor();
    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    if (c->err) {
        /* 暂时让c泄漏 */
        printf("Error: %s\n", c->errstr);
        return 1;
    }

    redisAttach(R, c);

    redisAsyncSetConnectCallback(c, connectCallback);
    redisAsyncSetDisconnectCallback(c, disconnectCallback);

    eventloop(R);

    release_reactor(R);
    return 0;
}
