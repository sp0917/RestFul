/*
 * RestServerInternal.h
 *
 *  Created on: 2018年6月25日
 *      Author: sp0917
 */
#include "RequestWorker.h"
#include "RestServer.h"
#include <sys/types.h>
#include <sys/socket.h>

#ifndef PROJECTS_RESTFUL_RESTSERVER_RESTSERVERINTERNAL_H_
#define PROJECTS_RESTFUL_RESTSERVER_RESTSERVERINTERNAL_H_

namespace RestFul {

typedef struct {
	boost::regex 	expression;
	HttpMethod 		method;
	CRestServer::RestReqProc proc;
}RestRequestMap;

class CRequestReceiver{
public:
	CRequestReceiver(struct event_base* base, struct evhttp* http);
	virtual ~CRequestReceiver();

	bool start();
	bool stop();
	bool cleanup();

private:
	void workThread();

private:
	bool m_exit_flag;
	boost::thread_group	m_thread_pool;
	struct event_base*	m_base;
	struct evhttp*		m_httpd;
};

class CRestServer::CInternal {
public:
	CInternal();
	virtual ~CInternal();
	bool init(unsigned short port = 1234, unsigned short threadNum = 1);
	bool start();
	bool stop();
	void setRequestProc(RestRequestMap restRequest);

private:
	int bindSocket(unsigned short port);

public:
	void pushTask(struct evhttp_request* req);
	void delegateTask(struct evhttp_request* req);
	void popTask(struct evhttp_request* req);

private:
	static void eventLog(int severity, const char *msg);
	static void genericHandler(struct evhttp_request* req, void* arg);

private:
	int 	m_fd;
	bool 	m_is_inited;
	std::list<CRequestReceiver*> 	m_receiver_list;
	std::list<CRequestWorker*>		m_worker_list;
	boost::mutex					m_request_mutex;
	std::list<struct evhttp_request*>	m_request_list;
};
}



#endif /* PROJECTS_RESTFUL_RESTSERVER_RESTSERVERINTERNAL_H_ */
