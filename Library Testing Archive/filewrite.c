#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

// Write callback function to handle data received from the API
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int main() {
    CURL *curl;
    FILE *fp;
    CURLcode res;

    const char *url = "https://api.openweathermap.org/data/2.5/weather?lat=42.03&lon=-93.63&appid=020c9db2f8a57004fe2e82f6e1bbd905&units=imperial";  // Replace with your API URL
    const char *outfilename = "output.json";

    // Initialize curl
    curl = curl_easy_init();
    if(curl) {
        // Open file to write
        fp = fopen(outfilename, "wb");
        if (!fp) {
            perror("File opening failed");
            return EXIT_FAILURE;
        }

        // Set curl options
        curl_easy_setopt(curl, CURLOPT_URL, url);                     // Set API URL
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);    // Set write callback function
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);                // Pass the file pointer to the callback

        // Perform the request
        res = curl_easy_perform(curl);
        
        // Clean up
        fclose(fp);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
