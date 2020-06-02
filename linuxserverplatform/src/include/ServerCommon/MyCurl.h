#pragma once

#include <string>
#include <vector>
#include "curl/curl.h"

using namespace std;

#define REMOTE_URL "ftp://%s:%s@%s/%d/%s.json"

class MyCurl
{
public:
	MyCurl() {}
	~MyCurl() {}

	typedef struct MemoryStruct
	{
		char *memory;
		size_t size;
	}tMemory;

	int getUrl(const string &url, const vector<string> &vUrlHeader, string &result, bool bSSLCert = false);
	int postUrl(const string &url, const vector<string> &vUrlHeader, const string &postFields, string &result, bool bSSLCert = false);
	int postUrlHttps(const string &url, const vector<string> &vUrlHeader, const string &postFields, string &result, bool bSSLCert = false);
	static size_t parseStreamCallback(void *contents, size_t length, size_t nmemb, void *userp);
	static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
	static size_t getcontentlengthfunc(void *ptr, size_t size, size_t nmemb, void *stream);
	static size_t discardfunc(void *ptr, size_t size, size_t nmemb, void *stream);
	static size_t readfunc(void *ptr, size_t size, size_t nmemb, void *stream);
	int uploadUrl(const char *videocode, int gameID, int timeout, int tries);

	static string RsaPrivateKey;
};