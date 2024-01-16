// httpExample.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#  include <unistd.h>
#endif
#include <curl/curl.h>
#include "httpClient.h"

#define CLASSMODE

size_t WriteCallbackEx(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

int main() {

    /*const char input_data[] = "This is a simple example of zlib compression.";
    const size_t input_size = strlen(input_data);

    char* compressed_data;
    size_t compressed_size;

    compress_data(input_data, input_size, &compressed_data, &compressed_size);

    char* uncompressed_data;
    size_t uncompressed_size;

    decompress_data(compressed_data, compressed_size, &uncompressed_data, &uncompressed_size); */

    // Initialize libcurl

    //CurlClient client("http://localhost:8080");

    // Perform the request with "hello" as POST data
    //client.performRequest("hello");
   // std::cout << "Response: " << client.getResponse() << std::endl;

#ifdef CLASSMODE
    httpClient oClient;
    //oClient.SingleConnectionCall();
    int mode = 1;
    if (mode == 0)
    {
        //oClient.Init("https://aliyunapi.thewarz.cn/api/api_GetDataGameRewards.aspx", MODE_GET, 0);
        oClient.Init("http://localhost:56016/api_Login.aspx", MODE_GET, 0);
    }
    else if (mode == 1)
    {
        oClient.Init("http://localhost:56016/api_Login.aspx", MODE_POST, 0);
    }
    else
        oClient.Init("https://download.sysinternals.com/files/DebugView.zip", MODE_DOWNLOAD, 0,"D:\\DebugView.zip");
    oClient.Issue();

    oClient.DeInit();
#else
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Create a CURL object
    CURL* curl = curl_easy_init();
    if (curl) {
        // Set the URL to send the request to
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8080");

        // Set the callback function to handle the response
        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackEx);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            // Print the received response
            std::cout << "Response: " << response << std::endl;
        }

        // Clean up
        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "Failed to initialize libcurl." << std::endl;
    }

    // Cleanup libcurl
    curl_global_cleanup();
#endif
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
