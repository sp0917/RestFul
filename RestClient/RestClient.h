/*
 * RestClient.h
 *
 *  Created on: 2018年6月22日
 *      Author: sp0917
 */

#ifndef SRC_RESTCLIENT_RESTCLIENT_H_
#define SRC_RESTCLIENT_RESTCLIENT_H_

namespace RestFul {

class CRestClient {
public:
	CRestClient();
	virtual ~CRestClient();
	bool setHeaderInfo();

public:
	long get();
	long post();
	long put();
	long del();

private:

};

} /* namespace RestFul */

#endif /* SRC_RESTCLIENT_RESTCLIENT_H_ */
