#include "reactor.h"

// 创建Reactor对象
reactor_t *
create_reactor() {
    reactor_t *r = (reactor_t *)malloc(sizeof(*r));
    r->epfd = epoll_create(1);
    r->listenfd = 0;
    r->stop = 0;
    r->iter = 0;
    r->events = (event_t*)malloc(sizeof(event_t) * MAX_CONN);
    memset(r->events, 0, sizeof(event_t)*MAX_CONN);
    memset(r->fire, 0, sizeof(struct epoll_event) * MAX_EVENT_NUM);
    return r;
}

// 释放Reactor对象
void
release_reactor(reactor_t * r) {
    free(r->events);
    close(r->epfd);
    free(r);
}

// 获取Reactor的事件堆event上的空闲事件对象
static event_t *
_get_event_t(reactor_t *r) {
    r->iter++;
    while (r->events[r->iter & MAX_CONN].fd > 0) {
        r->iter++;
    }
    return &r->events[r->iter];
}

// 创建事件对象
event_t *
new_event(reactor_t *R, int fd,
    event_callback_fn rd,
    event_callback_fn wt,
    error_callback_fn err) {
    assert(rd != 0 || wt != 0 || err != 0);
    // 获取空闲事件对象
    event_t *e = _get_event_t(R);

    // 初始化事件对象
    e->r = R;
    e->fd = fd;
    buffer_init(&e->in, 1024*16);
    buffer_init(&e->out, 1024*16);
    e->read_fn = rd;
    e->write_fn = wt;
    e->error_fn = err;
    return e;
}

// 释放事件对象分配的buffer空间
void
free_event(event_t *e) {
    buffer_free(&e->in);
    buffer_free(&e->out);
}

// 设置非阻塞的fd
int
set_nonblock(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

// 添加事件
int
add_event(reactor_t *R, int events, event_t *e) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.ptr = e;
    if (epoll_ctl(R->epfd, EPOLL_CTL_ADD, e->fd, &ev) == -1) {
        printf("Add event error, fd = %d\n", e->fd);
        return 1;
    }
    return 0;
}

// 删除事件
int
del_event(reactor_t *R, event_t *e) {
    epoll_ctl(R->epfd, EPOLL_CTL_DEL, e->fd, NULL);
    free_event(e);
    return 0;
}

// 修改事件（读事件 or 写事件）
int
enable_event(reactor_t *R, event_t *e, int readable, int writeable) {
    struct epoll_event ev;
    ev.events = (readable ? EPOLLIN : 0) | (writeable ? EPOLLOUT : 0);
    ev.data.ptr = e;
    if (epoll_ctl(R->epfd, EPOLL_CTL_MOD, e->fd, &ev) == -1) {
        return 1;
    }
    return 0;
}

// 一次事件循环
void
eventloop_once(reactor_t * r, int timeout) {
    int n = epoll_wait(r->epfd, r->fire, MAX_EVENT_NUM, timeout);
    for (int i = 0; i < n; i++) {
        struct epoll_event *e = &r->fire[i];
        int mask = e->events;
        if (e->events & EPOLLERR) mask |= EPOLLIN | EPOLLOUT;
        if (e->events & EPOLLHUP) mask |= EPOLLIN | EPOLLOUT;
        event_t *et = (event_t*) e->data.ptr;
        // 处理读事件
        if (mask & EPOLLIN) {
            if (et->read_fn)
                et->read_fn(et->fd, EPOLLIN, et);
        }
        // 处理写事件
        if (mask & EPOLLOUT) {
            if (et->write_fn)
                et->write_fn(et->fd, EPOLLOUT, et);
            else {
                uint8_t * buf = buffer_write_atmost(&et->out);
                event_buffer_write(et, buf, buffer_len(&et->out));
            }
        }
    }
}

// 停止事件循环
void
stop_eventloop(reactor_t * r) {
    r->stop = 1;
}

// 事件循环
void
eventloop(reactor_t * r) {
    while (!r->stop) {
        eventloop_once(r, -1); // 阻塞等待事件
    }
}

// 创建服务器
int
create_server(reactor_t *R, short port, event_callback_fn func) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        printf("Create listenfd error!\n");
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    // 设置地址重用，允许快速重启服务器
    int reuse = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(int)) == -1) {
        printf("Reuse address error: %s\n", strerror(errno));
        return -1;
    }

    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
        printf("Bind error: %s\n", strerror(errno));
        return -1;
    }

    if (listen(listenfd, 5) < 0) {
        printf("Listen error: %s\n", strerror(errno));
        return -1;
    }

    if (set_nonblock(listenfd) < 0) {
        printf("Set nonblock error: %s\n", strerror(errno));
        return -1;
    }

    R->listenfd = listenfd;

    event_t *e = new_event(R, listenfd, func, 0, 0);
    add_event(R, EPOLLIN, e);

    printf("Listening on port: %d\n", port);
    return 0;
}

// 读取数据
int
event_buffer_read(event_t *e) {
    int fd = e->fd;
    int num = 0;
    while (1) {
        // TODO: 不要在这里使用固定大小的缓冲区
        char buf[1024] = {0};
        int n = read(fd, buf, 1024);
        if (n == 0) { // 半关闭状态
            printf("Close connection, fd = %d\n", fd);
            if (e->error_fn)
                e->error_fn(fd, "Close socket");
            del_event(e->r, e);
            close(fd);
            return 0;
        } else if (n < 0) { // 异常
            if (errno == EINTR) // 被中断，重试
                continue;
            if (errno == EWOULDBLOCK)  // 阻塞，因为read buffer为空
                break;
            printf("Read error, fd = %d, err = %s\n", fd, strerror(errno));
            if (e->error_fn)
                e->error_fn(fd, strerror(errno));
            del_event(e->r, e);
            close(fd);
            return 0;
        } else { // 正常
            printf("Received data from client: %s", buf);
            buffer_add(&e->in, buf, n);
        }
        // 多次读取的话，按序接在后面
        num += n;
    }
    return num;
}

// 向对端发送数据
static int
_write_socket(event_t *e, void * buf, int sz) {
    int fd = e->fd;
    while (1) {
        int n = write(fd, buf, sz);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            if (errno == EWOULDBLOCK)
                break;
            if (e->error_fn)
                e->error_fn(fd, strerror(errno));
            del_event(e->r, e);
            close(e->fd);
        }
        return n;
    }
    return 0;
}

// 写数据
int
event_buffer_write(event_t *e, void * buf, int sz) {
    buffer_t *r = &e->out;
    if (buffer_len(r) == 0) {
        int n = _write_socket(e, buf, sz);
        if (n == 0 || n < sz) {
            // 发送失败，将未发送的数据写入缓冲区，并注册写事件
            buffer_add(&e->out, (char *)buf + n, sz - n);
            enable_event(e->r, e, 1, 1);
            return 0;
        } else if (n < 0) 
            return 0;
        return 1;
    }
    buffer_add(&e->out, (char *)buf, sz);
    return 1;
}
