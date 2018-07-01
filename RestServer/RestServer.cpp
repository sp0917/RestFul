/*
 * RestServer.cpp
 *
 *  Created on: 2018年6月22日
 *      Author: sp0917
 */

#include "RestServerInternal.h"

using namespace boost;
namespace RestFul {

#define SERVER_NUM	2


CRequestReceiver::CRequestReceiver(struct event_base* base, struct evhttp* http)
{
	m_exit_flag = false;
	m_base = base;
	m_httpd = http;
}

CRequestReceiver::~CRequestReceiver()
{
}

bool CRequestReceiver::start()
{
	m_thread_pool.create_thread(boost::bind(&CRequestReceiver::workThread, this));

	return true;
}

bool CRequestReceiver::stop()
{
	if (m_exit_flag)
		return true;

	// stop
	if (event_base_loopbreak(m_base)) {
		// log
		return false;
	}

	m_thread_pool.join_all();

	// destroy thread
	m_exit_flag = true;

	return true;
}

bool CRequestReceiver::cleanup()
{
	if (m_httpd) {
		evhttp_free(m_httpd);
		m_httpd = NULL;
	}

	if (m_base) {
		event_base_free(m_base);
		m_base = NULL;
	}

	return true;
}

void CRequestReceiver::workThread()
{
	while(!m_exit_flag)
	{
		event_base_loop(m_base, EVLOOP_NONBLOCK);
		this_thread::sleep_for(boost::chrono::milliseconds(1));
	}
}

CRestServer::CRestServer()
{
	m_internal = NULL;
	m_internal = new CInternal();
	assert(m_internal != NULL);
}

CRestServer::~CRestServer()
{
	if (m_internal) {
		delete m_internal;
		m_internal = NULL;
	}

	// log
}

bool CRestServer::init(unsigned short port, unsigned short threadNum)
{
	if (!m_internal) {
		// log
		return false;
	}

	return m_internal->init(port, threadNum);
}

bool CRestServer::start()
{
	if (m_internal && !m_internal->start()) {
		// log
		return false;
	}

	return true;
}

bool CRestServer::stop()
{
	if (m_internal && !m_internal->stop()) {
		// log
		return false;
	}

	return true;
}

void CRestServer::setRequestProc(const std::string& what, HttpMethod method, RestReqProc proc)
{
	RestRequestMap restRequest;
	restRequest.expression = what;
	restRequest.method = method;
	restRequest.proc = proc;
	m_internal->setRequestProc(restRequest);
}

CRestServer::CInternal::CInternal()
{
	m_fd = 0;
	m_is_inited = false;
}

CRestServer::CInternal::~CInternal()
{}

bool CRestServer::CInternal::init(unsigned short port, unsigned short threadNum)
{
	if (m_is_inited)
		return true;

	if (evthread_use_pthreads() == -1) {
		// log
		return false;
	}

	int fd = bindSocket(port);
	if (fd < 0)
	{
		// log
		return false;
	}

	for (unsigned i = 0;  i < SERVER_NUM; ++i) {
		struct event_base * base = event_base_new();
		if (!base) {
			// log
			return false;
		}

		struct evhttp * httpd = evhttp_new(base);
		if (!httpd) {
			// log
			return false;
		}

		if (evhttp_accept_socket(httpd, fd)) {
			// log
			return false;
		}

		CRequestReceiver * receiver = new CRequestReceiver(base, httpd);
		if (receiver == NULL) {
			// log
			return false;
		}
		evhttp_set_gencb(httpd, CRestServer::CInternal::genericHandler, this);
		m_receiver_list.push_back(receiver);
	}

	event_set_log_callback(CRestServer::CInternal::eventLog);

	for (unsigned i = 0; i < threadNum; ++i) {
		CRequestWorker * worker = new CRequestWorker(this);
		if (worker == NULL) {
			// log
			return false;
		}

		m_worker_list.push_back(worker);
	}
	m_is_inited = true;

	return true;
}

bool CRestServer::CInternal::start()
{
	if (!m_is_inited) {
		// log
		return false;
	}
	std::list<CRequestReceiver*>::iterator receiver_it = m_receiver_list.begin();
	for ( ; receiver_it != m_receiver_list.end(); ++receiver_it) {
		(*receiver_it)->start();
	}

	std::list<CRequestWorker*>::iterator worker_it = m_worker_list.begin();
	for (; worker_it != m_worker_list.end(); ++worker_it) {
		(*worker_it)->start();
	}

	return true;
}

bool CRestServer::CInternal::stop()
{
	if (!m_is_inited)
		return true;

	std::list<CRequestReceiver*>::iterator receiver_it = m_receiver_list.begin();
	for (; receiver_it != m_receiver_list.end(); ++receiver_it) {
		(*receiver_it)->stop();
	}

	std::list<CRequestWorker*>::iterator worker_it = m_worker_list.begin();
	for (; worker_it != m_worker_list.end(); ++worker_it) {
		(*worker_it)->stop();
		delete (*worker_it);
	}
	m_worker_list.clear();

	receiver_it = m_receiver_list.begin();
	for (; receiver_it != m_receiver_list.end(); ++receiver_it) {
		(*receiver_it)->cleanup();
		delete (*receiver_it);
	}
	m_receiver_list.clear();
	m_is_inited = false;

	return true;
}

void CRestServer::CInternal::setRequestProc(RestRequestMap restRequest)
{
	std::list<CRequestWorker*>::iterator it = m_worker_list.begin();
	for(; it != m_worker_list.end(); ++it)
	{
		(*it)->setRequestProc(restRequest);
	}
}

int CRestServer::CInternal::bindSocket(unsigned short port)
{
	int serverFd = -1;
	int on = 1;
	struct sockaddr_in server;

	// 1.socket
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd == -1) {
		// log
		return serverFd;
	}

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
		// log
		return -1;
	}

	// 2.bind
	if (::bind(serverFd, (struct sockaddr *)&server, sizeof(server)) < 0) {
		// log
		return -1;
	}

//	if (port == 0) {
//		socklen_t serverLen = sizeof(server);
//		if (getsockname(serverFd, (struct sockaddr *)&server, &serverLen) == -1) {
//			// log
//			return -1;
//		}
//		port = ntohs(server.sin_port);
//	}

	// 3.listen
	if (listen(serverFd, 10240)) {
		// log
		return -1;
	}

	m_fd = serverFd;
	return m_fd;
}

void CRestServer::CInternal::eventLog(int severity, const char *msg)
{
	switch (severity) {
		case EVENT_LOG_DEBUG:
			break;

		case EVENT_LOG_MSG:
			break;

		case EVENT_LOG_WARN:
			break;

		case EVENT_LOG_ERR:
			break;

		default:
			break;
	}
}

void CRestServer::CInternal::genericHandler(struct evhttp_request* req, void* arg)
{
	CRestServer::CInternal * listener = (CRestServer::CInternal *)arg;
	if (listener)
		listener->pushTask(req);
}

void CRestServer::CInternal::pushTask(struct evhttp_request* req)
{
	m_request_mutex.lock();
	m_request_list.push_back(req);
	m_request_mutex.unlock();

	std::list<CRequestWorker*>::iterator it = m_worker_list.begin();
	for (; it != m_worker_list.end(); ++it)
	{
		(*it)->postSem();
	}
}

void CRestServer::CInternal::delegateTask(struct evhttp_request* req)
{
	m_request_mutex.lock();
	req = m_request_list.front();
	m_request_list.pop_front();
	m_request_mutex.unlock();
}

void CRestServer::CInternal::popTask(struct evhttp_request* req)
{
	// send response
}



























}/* namespace RestFul */


