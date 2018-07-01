/*
 * RestWorker.h
 *
 *  Created on: 2018年6月29日
 *      Author: sp0917
 */

#ifndef PROJECTS_RESTFUL_RESTSERVER_REQUESTWORKER_H_
#define PROJECTS_RESTFUL_RESTSERVER_REQUESTWORKER_H_

#include "RestServerInternal.h"

namespace RestFul {

class CRequestWorker{
public:
	CRequestWorker(CRestServer::CInternal* server = NULL);
	virtual ~CRequestWorker();

	bool start();
	bool stop();
	void postSem();
	bool setRequestProc(RestRequestMap task);

private:
	bool processReq(struct evhttp_request* req);
	void workThread();

private:
	bool							m_exit_flag;
	CRestServer::CInternal			*m_rest_server;
	std::map<long, std::string>		m_err_msg;
	std::list<RestRequestMap>		m_rest_proc_list;
	boost::thread_group				m_thread_pool;
	boost::interprocess::interprocess_semaphore	m_work_sem;
};

}

#endif /* PROJECTS_RESTFUL_RESTSERVER_REQUESTWORKER_H_ */
