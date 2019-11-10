#include "CommonHead.h"
#include "configManage.h"
#include "Define.h"
#include "log.h"
#include "MyCurl.h"

string  MyCurl::RsaPrivateKey;

size_t MyCurl::parseStreamCallback(void *contents, size_t length, size_t nmemb, void *userp)
{
	string*  respon = (string*)userp;
	size_t real_size = length * nmemb;
	respon->append((const char*)contents, real_size);
	return real_size;
}

size_t MyCurl::WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	tMemory *mem = (struct MemoryStruct *)userp;
	mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL)
	{
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	return realsize;
}

/* parse headers for Content-Length */
size_t MyCurl::getcontentlengthfunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int r;
	long len = 0;

	/* _snscanf() is Win32 specific */
	// r = _snscanf(ptr, size * nmemb, "Content-Length: %ld\n", &len);
	r = sscanf((const char *)ptr, "Content-Length: %ld\n", &len);
	if (r) /* Microsoft: we don't read the specs */
		* ((long*)stream) = len;

	return size * nmemb;
}

/* discard downloaded data */
size_t MyCurl::discardfunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
	return size * nmemb;
}

/* read data to upload */
size_t MyCurl::readfunc(void *ptr, size_t size, size_t nmemb, void *stream)
{
	FILE *f = (FILE *)stream;
	size_t n;

	if (ferror(f))
		return CURL_READFUNC_ABORT;

	n = fread(ptr, size, nmemb, f) * size;

	return n;
}

int MyCurl::getUrl(const string &url, const vector<string> &vUrlHeader, string &result, bool bSSLCert)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *headers = NULL;
	// headers = curl_slist_append(headers, urlHeader.c_str());
	for (size_t i = 0; i < vUrlHeader.size(); ++i)
	{
		headers = curl_slist_append(headers, vUrlHeader[i].c_str());
	}
	curl = curl_easy_init();    // 初始化
	if (curl)
	{
		if (headers)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);// 改协议头
		}

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);

		curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);

		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);

		// CURLOPT_FOLLOWLOCATION 设置支持302重定向
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, MyCurl::parseStreamCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&result);

		if (bSSLCert)
		{
			curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
			curl_easy_setopt(curl, CURLOPT_SSLKEY, MyCurl::RsaPrivateKey.c_str());
		}
		res = curl_easy_perform(curl);   // 执行
		if (res != 0)
		{
			if (headers)
			{
				curl_slist_free_all(headers);
			}
			curl_easy_cleanup(curl);
			return -1;
		}
		if (headers)
		{
			curl_slist_free_all(headers);
		}
		curl_easy_cleanup(curl);
	}
	else {
		
		return -1;
	}
	
	return 0;
}

int MyCurl::postUrl(const string &url, const vector<string> &vUrlHeader, const string &postFields, string &result, bool bSSLCert)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *headers = NULL;
	// headers = curl_slist_append(headers, urlHeader.c_str());
	for (size_t i = 0; i < vUrlHeader.size(); ++i)
	{
		headers = curl_slist_append(headers, vUrlHeader[i].c_str());
	}
	curl = curl_easy_init();
	if (curl)
	{
		// curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "cookie.txt"); 
		tMemory chunk;
		chunk.memory = (char *)malloc(1);
		chunk.size = 0;
		if (headers)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		}
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		// curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); 
		// curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);

		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		//curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000L);

		// CURLOPT_FOLLOWLOCATION 设置支持302重定向
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, MyCurl::WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
		curl_easy_setopt(curl, CURLOPT_POST, true);
		if (bSSLCert)
		{
			curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
			curl_easy_setopt(curl, CURLOPT_SSLKEY, MyCurl::RsaPrivateKey.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		res = curl_easy_perform(curl);
		if (res != 0) {
			
			if (headers)
			{
				curl_slist_free_all(headers);
			}
			curl_easy_cleanup(curl);
			return -1;
		}
		if (headers)
		{
			curl_slist_free_all(headers);
		}
		result = chunk.memory;
		free(chunk.memory);
		chunk.memory = NULL;
		curl_easy_cleanup(curl);
	}
	else
	{
		return -1;
	}
	
	return 0;
}

int MyCurl::postUrlHttps(const string &url, const vector<string> &vUrlHeader, const string &postFields, string &result, bool bSSLCert)
{
	CURL *curl;
	CURLcode res;
	struct curl_slist *headers = NULL;

	for (size_t i = 0; i < vUrlHeader.size(); ++i)
	{
		headers = curl_slist_append(headers, vUrlHeader[i].c_str());
	}

	curl = curl_easy_init();

	if (curl)
	{
		tMemory chunk;
		chunk.memory = (char *)malloc(1);
		chunk.size = 0;
		if (headers)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		}

		// curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "cookie.txt"); 

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1);

		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		//curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 5000L);

		// CURLOPT_FOLLOWLOCATION 设置支持302重定向
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, MyCurl::WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
		curl_easy_setopt(curl, CURLOPT_POST, true);
		if (bSSLCert)
		{
			curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "PEM");
			curl_easy_setopt(curl, CURLOPT_SSLKEY, MyCurl::RsaPrivateKey.c_str());
		}

		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		res = curl_easy_perform(curl);
		if (res != 0) 
		{
			if (headers)
			{
				curl_slist_free_all(headers);
			}
			curl_easy_cleanup(curl);

			ERROR_LOG(" curl error %s\n", curl_easy_strerror(res));

			return -1;
		}
		if (headers)
		{
			curl_slist_free_all(headers);
		}
		result = chunk.memory;
		free(chunk.memory);
		chunk.memory = NULL;
		curl_easy_cleanup(curl);
	}
	else
	{
		return -1;
	}
	return 0;
}

int MyCurl::uploadUrl(const char *videocode, int gameID, int timeout, int tries)
{
	int uploaded_len = 0;
	CURL* curl = curl_easy_init();
	//CURLcode r = CURLE_GOT_NOTHING;

	const FtpConfig& ftpConfig = ConfigManage()->GetFtpConfig();

	char tempbuff[512] = "";
	sprintf(tempbuff, REMOTE_URL, ftpConfig.ftpUser, ftpConfig.ftpPasswd, ftpConfig.ftpIP, gameID, videocode);

	char localPathBuf[512] = "";
	sprintf(localPathBuf, "%s%s.json", SAVE_JSON_PATH, videocode);

	FILE* f = fopen(localPathBuf, "rb");
	if (!f)
	{
		perror(NULL);
		return 0;
	}

	if (curl)
	{
		CURLcode r = CURLE_GOT_NOTHING;
		int c;
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		curl_easy_setopt(curl, CURLOPT_URL, tempbuff);

		if (timeout)
			curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, timeout);

		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &uploaded_len);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, discardfunc);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, readfunc);
		curl_easy_setopt(curl, CURLOPT_READDATA, f);

		/* disable passive mode */
		curl_easy_setopt(curl, CURLOPT_FTPPORT, "-");
		curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);


		for (c = 0; (r != CURLE_OK) && (c < tries); c++)
		{
			if (c)
			{
				curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
				curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

				r = curl_easy_perform(curl);
				if (r != CURLE_OK)
				{
					continue;
				}

				curl_easy_setopt(curl, CURLOPT_NOBODY, 0L);
				curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

				fseek(f, uploaded_len, SEEK_SET);

				curl_easy_setopt(curl, CURLOPT_APPEND, 1L);
			}
			else { /* no */
				curl_easy_setopt(curl, CURLOPT_APPEND, 0L);
			}

			r = curl_easy_perform(curl);
		}

		fclose(f);
		curl_easy_cleanup(curl);
		if (r == CURLE_OK)
			return 1;
		else {
			ERROR_LOG(" curl error %s\n", curl_easy_strerror(r));
			return 0;
		}
	}
	return 0;
}