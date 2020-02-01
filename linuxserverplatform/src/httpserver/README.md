# http-server
multi-thread http server based on libevent writen in c++

基于libevent实现的多线程http-server，相关代码已在生产环境使用（此示例已简化处理流程），长时间运行正常。

示例采用多线程（每个Worker对应一个线程），每个线程内一个event_base实例，同时监听在同一个server fd上（可以创建多个server监听多个端口）。

示例中采用了简单的同步处理方式，直接返回json数据

生产环境代码是在此基础上实现了请求的异步处理，特别需要注意的是采用异步方式时，同一个evhttp_request需要在接收的work线程内执行，否则会出现问题。

异步实现流程通常为：

1.  uri request handler接收到请求后，进行Header、参数以及body等的解析和验证工作；

2.  将解析的请求放入任务队列（任务与work线程建立映射，）

3.  任务线程池从任务队列取得任务，并异步执行

4.  异步执行结果会通过回调函数执行，需要根据步骤2建立的映射关系，转发到同一worker线程执行reply

5.  执行完毕，释放必要的资源。



在vps（单CPU，512M内存）上面进行测试结果如下：

] cat post.data   # post测试数据随意

{"rqst": 1}

] ab -k -c 10 -n 1000000 -p post.data http://127.0.0.1:6666/post
Concurrency Level: 10
Time taken for tests: 1.877 seconds
Requests per second: 34416.93 [#/sec] (mean)
Time per request: 0.291 [ms] (mean)
Time per request: 0.029 [ms] (mean, across all concurrent requests)

介绍：http://shamaoge.com/posts/392922
