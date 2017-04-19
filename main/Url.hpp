#ifndef MAIN_URL_HPP_
#define MAIN_URL_HPP_

#include "sdkconfig.h"
#include <stdio.h>
#include <list>
#include <string>

struct TParam{
	std::string paramName;
	std::string paramValue;
};

class Url {
public:
	Url(std::string& sHost, short int iPort, bool bSecure, std::string& sPath);
	Url(std::string& sUrl);

	virtual ~Url();

	void AddQueryParam(std::string& name, std::string& value);

	std::string& GetHost() { return msHost; }

	std::string& GetQuery();

	std::list<TParam>& GetQueryParams() { return mlQueryParams; }

	std::string& GetPath() { return msPath; }

	std::string& GetFragment() { return msFragment; }

	std::string& GetUrl();

	short int GetPort() { return miPort; }

	bool Selftest();

public:
	std::string UrlEncode(std::string& str);

	std::string UrlDecode(std::string str);


private:
	std::list<std::string> mlRequestHeaders;

	short int miPort;
	bool mbSecure = false;
	std::string msUrl;
	std::string msHost;
	std::string msPath;
	std::string msFragment;
	std::string msQuery;
	std::list<TParam> mlQueryParams;

	bool ParseUrl(std::string url);

	//TODO remove
	int ParseQuery(char* querystring) ;

};


#endif /* MAIN_URL_HPP_ */
