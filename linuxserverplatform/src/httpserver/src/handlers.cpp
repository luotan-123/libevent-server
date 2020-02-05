
#include "handlers.h"

#include <string>
#include <stdlib.h>
#include <memory.h>

#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

void general_request_handler(struct evhttp_request *req, void *arg) {
    printf("recv msg\n");
    evhttp_send_error(req, 403, "Forbidden");
}

void ping_handler(struct evhttp_request *req, void *args) {
    printf("recv ping\n");
    evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json");
    struct evbuffer *evb = evbuffer_new();
    evbuffer_add_printf(evb, "%s", "{\"message\":\"PING -> PONG\"}");
    evhttp_send_reply(req, 200, "OK", evb);
    evbuffer_free(evb);
}

void post_handler(struct evhttp_request *req, void *args) {

    printf("recv post\n");

    const char *source_uri = evhttp_request_get_uri(req);

    /************ parse params ************/
    struct evhttp_uri* ev_uri = evhttp_uri_parse(source_uri);
    struct evkeyvalq uri_querys;
    int res = evhttp_parse_query_str(evhttp_uri_get_query(ev_uri), &uri_querys);
    for (struct evkeyval* query=uri_querys.tqh_first; query; query = query->next.tqe_next) {
        if(query->value == nullptr)
            ; // 无参数值
        else
            ; // 参数(query->key)值为query->value
    }
    evhttp_clear_headers(&uri_querys);  // 不释放会有内存泄漏
    evhttp_uri_free(ev_uri);


    /************ method ************/
    std::string http_method{"UNKNOWN"};
    enum evhttp_cmd_type cmd_type = evhttp_request_get_command(req);
    switch (cmd_type) {
        case EVHTTP_REQ_GET:        http_method = "GET"; break;
        case EVHTTP_REQ_POST:       http_method = "POST"; break;
        case EVHTTP_REQ_HEAD:       http_method = "HEAD"; break;
        case EVHTTP_REQ_PUT:        http_method = "PUT"; break;
        case EVHTTP_REQ_DELETE:     http_method = "DELETE"; break;
        case EVHTTP_REQ_OPTIONS:    http_method = "OPTIONS"; break;
        case EVHTTP_REQ_TRACE:      http_method = "TRACE"; break;
        case EVHTTP_REQ_CONNECT:    http_method = "CONNECT"; break;
        case EVHTTP_REQ_PATCH:      http_method = "PATCH"; break;
        default:                    http_method = "UNKNOWN"; break;
    }

    /************ headers ************/
    struct evkeyvalq* input_headers = evhttp_request_get_input_headers(req);
    //for (auto header = input_headers->tqh_first; header; header = header->next.tqe_next) {
    //   LOG_INFO << header->key  << " " << header->value;
    //}
    const char* p_x_forward = evhttp_find_header(input_headers, "X-Forwarded-For");
    std::string x_forward = p_x_forward?std::string(p_x_forward):"-";

    const char* remote_addr = req->remote_host;
    const unsigned short remote_port = req->remote_port;

    const char* p_host = evhttp_find_header(input_headers, "Host");
    std::string host = p_host ? std::string(p_host) : "";

    /************ body ************/
    struct evbuffer* evbuf = evhttp_request_get_input_buffer(req);
    size_t body_len = evbuffer_get_length(evbuf);
    std::string body;
    body.resize(body_len);
    memcpy(&body[0], evbuffer_pullup(evbuf, -1), body_len);

    { // 同步过程 业务逻辑处理...
        std::string resp{"{\"message\":\"PING -> PONG  PING -> PONG  PING -> PONG  PING -> PONG  PING -> PONG  PING -> PONG  PING -> PONG  PING -> PONG\"}"};

        evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json");
    
        struct evbuffer *evb = evbuffer_new();
        evbuffer_add_printf(evb, "%s", resp.c_str());
        evhttp_send_reply(req, 200, "OK", evb);
        evbuffer_free(evb);
    }
    {
        // 亦可以进行异步处理过程( 异步处理reply时需要在同一个线程内返回 )
    }
}

