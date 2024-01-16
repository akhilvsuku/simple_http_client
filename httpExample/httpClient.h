#pragma once
#include <iostream>
#include <curl/curl.h>

#define Z_SOLO
#define STANDALONE_DEBUG

#define MODE_GET 0
#define MODE_POST 1
#define MODE_DOWNLOAD 2

#ifdef STANDALONE_DEBUG
struct CUserProfile
{
	DWORD CustomerID;
	DWORD SessionID;
};
#endif

typedef struct _t_curl_response
{
	uint8_t n_type;
	uint8_t n_result_code;
	uint32_t n_len;
	void* ptr_out;
	uint32_t n_err;
}CurlResponse;

class httpClient
{

private:
	CURL* m_o_curl;					/*  handle is used to hold and control a single network transfer.
									It is encouraged to reuse easy handles for repeated transfers.*/

	FILE* fp;						// file pointer used when downloading 
	std::string str_request;		// request body
	std::string str_response;		// respose received from server
	uint8_t m_n_mode;				// mode of httpClient MODE_GET MODE_POST MODE_DOWNLOAD

public:

	int n_curl_err;					// error can be used for class purpose
	int n_response_code;			// response code of last request
	static int n_response_body_len; // response body length

	httpClient();					
	httpClient(const char* str_url);

	httpClient(const CUserProfile* prof, const char* url)
	{
		Init(url, MODE_POST, 0);
		AddSessionInfo(prof->CustomerID, prof->SessionID);
	}
	virtual ~httpClient();
	void		Init(std::string str_url, int n_mode,int n_timeout = 0, std::string str_filename = "");
	
	void AddParam(const char* name, const char* value);
	void AddParam(const char* name, int);
	void AddParam(const char* name, float);
	void AddParamF(const char* pch_name, float f_val) {
		AddParam(pch_name, f_val);
	}


	void AddSessionInfo(DWORD id, DWORD sid)
	{
		AddParam("s_id", (int)id);
		AddParam("s_key", (int)sid);
	}

	CURLcode		Issue();
	static size_t ResponseHandler(void* contents, size_t size, size_t nmemb, std::string* output);
	static size_t DownloadHandler(void* contents, size_t size, size_t nmemb, void* userp);

	int ParseData(std::string str_data);
	void		DeInit();

};

int decompress_data(const char* compressed_data, size_t compressed_size, char** uncompressed_data, size_t* uncompressed_size);
int compress_data(const char* input_data, size_t input_size, char** compressed_data, size_t* compressed_size);