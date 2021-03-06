/*************************************************************************
 * Copyright (c) 2015, LiChuang. All rights reserved.
 * Author: lichuang(whlichuang@126.com)
 * Created Time: Thu May 14 15:55:00 2015
 * Description: 
 ************************************************************************/


#include "url_context.h"
#include "crawl_listen_handler.h"
#include "logger_container.h"
#include "http_processor.h"
#include "cmd_ctrler.h"
#include "request_recv.h"

RequestRecv::RequestRecv(HttpProcessor *httpProcessor, CmdCtrler *cmdCtrler)
    :_httpProcessor(httpProcessor), _cmdCtrler(cmdCtrler), _requestCount(0)
{
}

void RequestRecv::run()
{
    LOG(INFO, "start");
    int port = 9090;
    shared_ptr<CrawlListenHandler> handler(new CrawlListenHandler(_httpProcessor, this, _cmdCtrler));
    shared_ptr<TProcessor> processor(new CrawlServiceProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
    shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(1);
    shared_ptr<PosixThreadFactory> threadFactory = shared_ptr<PosixThreadFactory> (new PosixThreadFactory()); //PosixThreadFactory可以自定义（继承于ThreadFactory）
    threadManager->threadFactory(threadFactory);
    threadManager->start();

    TThreadPoolServer server(processor, serverTransport, transportFactory, protocolFactory, threadManager);

    try {
        server.serve();
    } catch (apache::thrift::transport::TTransportException e) {
        LOG_F(FATAL, "TTransportException %s", e.what());
    }
}

void RequestRecv::stop()
{
    this->terminate();
}

void RequestRecv::addRequestCount()
{
    atomic_add(&_requestCount, 1);
}

void RequestRecv::control(string& response, const string& cmd)
{
    char msg[1024] = {'\0'};
    snprintf(msg, 1024, "requestCount:%d", _requestCount);
    response = string(msg);
}
