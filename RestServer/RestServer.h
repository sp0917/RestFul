/*
 * RestServer.h
 *
 *  Created on: 2018年6月22日
 *      Author: sp0917
 */

#include <event2/event.h>
#include <event2/http.h>
#include <event2/util.h>
#include <event2/thread.h>
#include <event2/buffer.h>
#include <boost/regex.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include "json/json.h"

#ifndef SRC_RESTSERVER_RESTSERVER_H_
#define SRC_RESTSERVER_RESTSERVER_H_

//using namespace boost::interprocess;

namespace RestFul {

typedef enum{
	httpMethodGet 		= 1 << 0,
	httpMethodPost 		= 1 << 1,
	httpMethodHead		= 1 << 2,
	httpMethodPut		= 1 << 3,
	httpMethodDelete	= 1 << 4,
	httpMethodOptions	= 1 << 5,
	httpMethodTrace		= 1 << 6,
	httpMethodConnect	= 1 << 7,
	httpMethodPatch		= 1 << 8
}HttpMethod;

typedef enum{
	HttpOk					=	200,	/**< request completed ok */
	HttpNoContent			=	204,	/**< request does not have content */
	HttpMovePerm			=	301,	/**< the uri moved permanently */
	HttpMoveTemp			=	302,	/**< the uri moved temporarily */
	HttpNotModified			=	304,	/**< page was not modified from last */
	HttpBadRequest			=	400,	/**< invalid http request was made */
	HttpNotFound			= 	404,	/**< could not find content for uri */
	HttpBadMethod			=	405, 	/**< method not allowed for this uri */
	HttpEntityTooLarge		=	413,	/**<  */
	HttpExpectationFailed	=	417,	/**< we can't handle this expectation */
	HttpInternal			=	500,    /**< internal error */
	HttpNotImplemented		=	501,    /**< not implemented */
	HttpServUnavail			=	503		/**< the server is not available */
}HttpStatus;

class CRestServer;
class CRestRequest{
public:
	bool getContent(std::string& content);

private:
	bool setContent(const std::string& content);

private:
	std::string m_content;
};

class CRequestWorker;
class CRestServer{
public:
	CRestServer();
	virtual ~CRestServer();

	bool init(unsigned short port = 1234, unsigned short threadNum = 1);
	bool start();
	bool stop();
	bool cleanup();

	typedef long(*RestReqProc)(bool, const Json::Value&, Json::Value&);
	void setRequestProc(const std::string& what, HttpMethod method, RestReqProc proc);

	friend class CRequestWorker;
private:
	class CInternal;
	CInternal	*m_internal;
};

}/* namespace RestFul */

#endif /* SRC_RESTSERVER_RESTSERVER_H_ */
