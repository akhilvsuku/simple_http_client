#include "httpClient.h"
#include <string>
#include <zlib.h>

int httpClient::n_response_body_len = 0;

httpClient::httpClient()
{
    n_curl_err = 0;
    m_n_mode = MODE_GET;
    m_o_curl = 0;
    fp = 0;
    str_request = "";
    str_response = "";
}

httpClient::httpClient(const char* str_url) : httpClient()
{
    Init(str_url, MODE_POST,0);
}

httpClient::~httpClient()
{
}

/*              function for initialization.
*               sample values :
*               str_url => https://aliyunapi.thewarz.cn/api/api_GetDataGameRewards.aspx
*               n_mode  => MODE_GET
*               n_timeout => timeout value, not mandatory
*               str_filename => fully qualified file name when downloading 
*/
void httpClient::Init(std::string str_url,int n_mode,int n_timeout,std::string str_filename)
{
    
    curl_global_init(CURL_GLOBAL_DEFAULT);  /*  If you did not already call curl_global_init before calling 
                                                curl_easy_init function, curl_easy_init does it automatically. 
                                                This may be lethal in multi-threaded cases, if 
                                                curl_global_init is not thread-safe in your system, 
                                                and it may then result in resource 
                                                problems because there is no corresponding cleanup.*/

    // Create a CURL object
    m_o_curl = curl_easy_init();

    if (m_o_curl) {
        n_curl_err = 0;
        m_n_mode = n_mode;
        if (n_mode == MODE_DOWNLOAD) // setting parameters for download operation.
        {
            errno_t err;
            //file opening for saving the downloaded data, extension of file should be provided.
            if ((err = fopen_s(&fp, str_filename.c_str(), "w+")) != 0) 
            {
                printf("File was not opened\n");
            }

            // Set the callback function to handle the downloaded data
            curl_easy_setopt(m_o_curl, CURLOPT_WRITEFUNCTION, httpClient::DownloadHandler);

            // Pass the file pointer to the callback function
            curl_easy_setopt(m_o_curl, CURLOPT_WRITEDATA, fp);
        }
        else
        {
            // Set the callback function to handle the get post requests data
            curl_easy_setopt(m_o_curl, CURLOPT_WRITEFUNCTION, httpClient::ResponseHandler);

            // Set response data variable
            curl_easy_setopt(m_o_curl, CURLOPT_WRITEDATA, &str_response);

            if (n_mode == MODE_POST) //configuration for MODE_POST
            {

                // Set the request type to POST
                curl_easy_setopt(m_o_curl, CURLOPT_POST, 1L);

            }
            else
                curl_easy_setopt(m_o_curl, CURLOPT_HTTPGET, 1L); // Set the request type to GET

        }
         
        if(n_timeout > 0) //if specified setting timeout. Not mandatory
            curl_easy_setopt(m_o_curl, CURLOPT_CONNECTTIMEOUT, n_timeout); 

        // providing the URL for the request
        curl_easy_setopt(m_o_curl, CURLOPT_URL, str_url.c_str());

    }
    else
    {
        n_curl_err = 1;
    }
}

/* 
*           Overloaded function used to Add params, this will sent as the request data.
*/

void httpClient::AddParam(const char* pch_name, const char* pch_value)
{
    /*struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;

    curl_formadd(&post, &last,
        CURLFORM_COPYNAME, name,
        CURLFORM_COPYCONTENTS, str_value,
        CURLFORM_END);


    curl_easy_setopt(m_o_curl, CURLOPT_HTTPPOST, post); */

    curl_mime* mime;
    curl_mimepart* part;

    /* Build an HTTP form with a single field named "data", */
    mime = curl_mime_init(m_o_curl);
    part = curl_mime_addpart(mime);
    curl_mime_data(part, pch_value, CURL_ZERO_TERMINATED);
    curl_mime_name(part, pch_name);

    /* Post and send it. */
    curl_easy_setopt(m_o_curl, CURLOPT_MIMEPOST, mime);

}

/*
*           Overloaded function used to Add params, this will sent as the request data.
*/
void httpClient::AddParam(const char* pch_name, int n_value)
{
    AddParam(pch_name, std::to_string(n_value).c_str());
}

/*
*           Overloaded function used to Add params, this will sent as the request data.
*/
void httpClient::AddParam(const char* pch_name, float f_value)
{
    return AddParam(pch_name, std::to_string(f_value).c_str());
}

/*
*           Function sending the request. Response will process before passing to ParseData
*/
CURLcode httpClient::Issue()
{
    /*
    * Disable SSL/TLS Verification (Not Recommended for Production)
    curl_easy_setopt(m_o_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_o_curl, CURLOPT_SSL_VERIFYHOST, 0L);
    

    curl_easy_setopt(m_o_curl, CURLOPT_POSTFIELDS, str_params.c_str());*/

    CURLcode res = curl_easy_perform(m_o_curl);
    if (res != CURLE_OK) {
        n_curl_err = 2;
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    }
    else {
        // PRocess response

        curl_easy_getinfo(m_o_curl, CURLINFO_RESPONSE_CODE, &n_response_code);

        if (n_response_code == 200)
        {
            switch (m_n_mode)
            {
            case MODE_GET: {
                /*
                *  Process GET response
                */
                n_curl_err = ParseData(str_response);


            }
                         break;
            case MODE_POST: {
                /*
                * Process GET response
                */

            }
                          break;
            case MODE_DOWNLOAD: {
                /*
                * Process downloaded data
                */
            }
                              break;
            }
        }

    }
    return res;
}

/*
*               callback function for POST-GET request
*/
size_t httpClient::ResponseHandler(void* contents, size_t size, size_t nmemb, std::string* output)
{
    size_t totalSize = size * nmemb;


    output->append(static_cast<char*>(contents), totalSize); // removing the header
    httpClient::n_response_body_len = totalSize;


    return totalSize;
}

/*
*               callback function for Download request
*/
size_t httpClient::DownloadHandler(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t total_size = size * nmemb;
    fwrite(contents, size, nmemb, (FILE*)userp);
    return total_size;
}

/*
*               Parse data received through GET, 
*             
*/
int httpClient::ParseData(std::string str_data)
{
    if (str_data.length() >= 2 && str_data[0] == '$' && str_data[1] == '1')
    {

    }

    if (str_data.length() < 4)
    {
        return 1;
    }

    

    char* pch_data = (char*)str_data.c_str();
    if (str_data[0] == '<' && str_data[1] == '?' && str_data[2] == 'x')
    {
        return 2;
    }

    if (str_data[0] != 'W' || str_data[1] != 'O' || str_data[2] != '_')
    {
        return 3;
    }


    int resultCode = str_data[3] - '0';

    /* paarse data logic*/
}

/*
*               function for deinitilization of CURL
*/
void httpClient::DeInit()
{
    /*curl_global_init call must have a corresponding call to 
            curl_easy_cleanup when the operation is complete.*/
    if(fp)
        fclose(fp);

    if(m_o_curl)
        curl_easy_cleanup(m_o_curl);

    curl_global_cleanup();
}

/*
*   function can be used for decompressing the received data, if it is compressed.
*/
int decompress_data(const char* compressed_data, size_t compressed_size, char** uncompressed_data, size_t* uncompressed_size) {
    // Set up the zlib stream for decompression
    z_stream stream;
    memset(&stream, 0, sizeof(stream));

    if (inflateInit(&stream) != Z_OK) {
        fprintf(stderr, "Failed to initialize zlib.\n");
        return -1;
    }

    // Allocate memory for the uncompressed data buffer
    *uncompressed_data = (char*)malloc(compressed_size * 2); // Use an initial estimate for the size
    if (*uncompressed_data == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        return -1;
    }

    stream.next_in = (Bytef*)compressed_data;
    stream.avail_in = (uInt)compressed_size;
    stream.next_out = (Bytef*)(*uncompressed_data);
    stream.avail_out = (uInt)(compressed_size * 2); // Use an initial estimate for the size

    // Decompress the data
    int ret = inflate(&stream, Z_FINISH);
    if (ret != Z_STREAM_END) {
        fprintf(stderr, "Failed to decompress data. Error code: %d\n", ret);
        inflateEnd(&stream);
        free(*uncompressed_data);
        return -1;
    }

    // Set the actual size of the uncompressed data
    *uncompressed_size = stream.total_out;

    // Clean up the zlib stream
    inflateEnd(&stream);

    return 0;
}


int compress_data(const char* input_data, size_t input_size, char** compressed_data, size_t* compressed_size) {
    // Set up the zlib stream for compression
    z_stream stream;
    memset(&stream, 0, sizeof(stream));

    if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
        fprintf(stderr, "Failed to initialize zlib for compression.\n");
        return -1;
    }

    // Allocate memory for the compressed data buffer
    *compressed_data = (char*)malloc(input_size * 2); // Use an initial estimate for the size
    if (*compressed_data == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        return -1;
    }

    stream.next_in = (Bytef*)input_data;
    stream.avail_in = (uInt)input_size;
    stream.next_out = (Bytef*)(*compressed_data);
    stream.avail_out = (uInt)(input_size * 2); // Use an initial estimate for the size

    // Compress the data
    int ret = deflate(&stream, Z_FINISH);
    if (ret != Z_STREAM_END) {
        fprintf(stderr, "Failed to compress data. Error code: %d\n", ret);
        deflateEnd(&stream);
        free(*compressed_data);
        return -1;
    }

    // Set the actual size of the compressed data
    *compressed_size = stream.total_out;

    // Clean up the zlib stream
    deflateEnd(&stream);

    return 0;
}