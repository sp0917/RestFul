/*
 * RestWorker.cpp
 *
 *  Created on: 2018年6月29日
 *      Author: sp0917
 */

#include "RequestWorker.h"

namespace RestFul {

struct err_msg_class{
	long errCode;
	const char* errMsg;
};

static const err_msg_class err_msg[] = {
		{HttpOk, "{\"code\" : \"200\", \"message\": \"Success\" }"},
		{HttpNotFound, "{\"code\" : \"404\", \"message\": \"Not Found\" }"},
		{HttpBadMethod, "{\"code\" : \"405\", \"message\": \"Method Forbidden\" }"},
		{HttpInternal, "{\"code\" : \"500\", \"message\": \"Internal Error\" }"},
		{HttpServUnavail, "{\"code\" : \"503\", \"message\": \"Server Unavailable\" }"}
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(X) sizeof(err_msg)/sizeof(err_msg[0])
#endif

CRequestWorker::CRequestWorker(CRestServer::CInternal* server)
: m_work_sem(1)
{
	m_exit_flag = false;
	m_rest_server = server;
	m_err_msg.clear();
	m_rest_proc_list.clear();

	for (unsigned i = 0; i < ARRAY_SIZE(err_msg); ++i) {
		m_err_msg.insert(std::map<long, std::string>::value_type(
				err_msg[i].errCode, err_msg[i].errMsg));
	}
}

bool CRequestWorker::start()
{
	m_thread_pool.create_thread(boost::bind(&CRequestWorker::workThread, this));

	return true;
}

bool CRequestWorker::stop()
{
	m_thread_pool.join_all();
	m_rest_proc_list.clear();

	return true;
}

void CRequestWorker::postSem()
{
	m_work_sem.post();
}

bool CRequestWorker::setRequestProc(RestRequestMap task)
{
	m_rest_proc_list.push_back(task);

	return true;
}

void CRequestWorker::workThread()
{
	while(!m_exit_flag)
	{
		struct evhttp_request * req = NULL;
		m_work_sem.wait();

		m_rest_server->delegateTask(req);
		if (NULL == req)
			continue;

		if (processReq(req))
			m_rest_server->pushTask(req);
	}
}

bool CRequestWorker::processReq(struct evhttp_request* req)
{
	struct evbuffer *evb = NULL;
	const char *docroot = "/home/si_peng/Downloads/";
	const char *uri = evhttp_request_get_uri(req);
	struct evhttp_uri *decoded = NULL;
	const char *path;
	char *decoded_path;
	char *whole_path = NULL;
	size_t len;
	int fd = -1;
	//struct stat st;

	HttpMethod method = (HttpMethod)evhttp_request_get_command(req);

	// decode the uri
	decoded = evhttp_uri_parse(uri);
	if (!decoded)
	{
		evhttp_send_error(req, HttpBadRequest, 0);
		goto done;
	}

	// let's see the whole path
	path = evhttp_uri_get_path(decoded);
	if (!path)
		path = "/";

	decoded_path = evhttp_uridecode(path, 0, NULL);
	if (decoded_path == NULL)
		goto err;

	if (strstr(decoded_path, ".."))
		goto err;

	len = strlen(decoded_path) + strlen(docroot) + 2;
	if (!(whole_path = (char *)malloc(len))){
		goto err;
	}
	evutil_snprintf(whole_path, len, "%s/%s", docroot, decoded_path);

	evb = evbuffer_new();
	evhttp_send_reply(req, HttpOk, "Success", evb);
	goto done;

err:
	evhttp_send_error(req, HttpNotFound, "Document was not found");

done:
	if (decoded)
		evhttp_uri_free(decoded);

	if (decoded_path)
		free(decoded_path);

	if (whole_path)
		free(whole_path);

	if (evb)
		evbuffer_free(evb);

	return true;
}

CRequestWorker::~CRequestWorker()
{
}

}


