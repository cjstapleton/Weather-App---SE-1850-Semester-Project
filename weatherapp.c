#include <curl/curl.h>
#include<json-c/json.h>
#include <stdio.h>
#include <stdlib.h>

// api url for ames iowa in imperial units: https://api.openweathermap.org/data/2.5/weather?lat=42.03&lon=-93.63&appid=020c9db2f8a57004fe2e82f6e1bbd905&units=imperial

int weatherAPICall(userDefinedLocation);

int main (void) {
    char userDefinedLocation[15];
    printf("Please enter a location you would like to see the weather at.\n");
    scanf("%s", userDefinedLocation);

    printf("%s", userDefinedLocation);

}

int weatherAPICall(userDefinedLocation) {
    CURL *curl;
    CURLcode result;

    curl = curl_easy_init();
    if (curl == NULL) {
        fprintf(stderr, "HTTP request failed\n");
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, userDefinedLocation);

    result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
        return -1;
    }

    curl_easy_cleanup(curl);

    return 0;
}
