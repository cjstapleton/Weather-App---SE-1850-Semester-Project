#include <curl/curl.h>
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// api url for ames iowa in imperial units: https://api.openweathermap.org/data/2.5/weather?zip=50012&appid=020c9db2f8a57004fe2e82f6e1bbd905&units=imperial

/* PROTOTYPES */
int weatherData(char *apiURL);
int parseWeatherData();
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);

int main (void) {
    //retrieve user input
    char userDefinedLocation[6];
    char *baseURL = "https://api.openweathermap.org/data/2.5/weather?zip=-----&appid=020c9db2f8a57004fe2e82f6e1bbd905&units=imperial";

    
    //input loop to make sure user enters a 5 digit zip code
    printf("Enter the zip code you would like to see the weather at.\n");
    scanf("%s", userDefinedLocation);
    
    while (strlen(userDefinedLocation) != 5) {
        printf("Please enter a valid zip code.\n");
        scanf("%s", userDefinedLocation);
    }
    
    
    //modify the API url for the correct location

    // use strchr() to find first instance of '-'
    char *zipCodeInsertLocation = strchr(baseURL, '-');
    // strchr() returns a string of first instance of '-' and onwards
    // subtract two strings to get the difference in memory location 
    // (i.e. the index number of the first '-')
    int strInsertionIndexNum = zipCodeInsertLocation - baseURL;

    // create new string to avoid modifying string literal in memory
    char apiURL[256];

    // copy baseURL to apiURL
    strcpy(apiURL, baseURL);
    
    // replace each instance of '- with each num in userDefinedLocation
    // since strchr() returns the first instance of '-' in the baseURL, 
    // we know that the first five characters are '-', so we loop over the first
    // five and replace them with each digit of the user inputted zip code
    for (int i = 0; i < 5; i++) {
        apiURL[strInsertionIndexNum] = userDefinedLocation[i];

        strInsertionIndexNum += 1;
    }

    // *FROM THIS POINT, apiURL IS USABLE

    weatherData(apiURL);

    // 11/3 PARSE JSON FILE CONTENTS
    
    parseWeatherData();
}

// callback function to handle data from API
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

// use curl to make an HTTP request and write weather forecast into a .json file
int weatherData(char *apiURL) {
    CURL *curl = curl_easy_init();
    FILE *fp;
    CURLcode result;

    const char *outfilename = "current-weather.json";

    if (curl == NULL) {
        fprintf(stderr, "HTTP request failed\n");
        return -1;
    }

    // create the .json file where "wb" writes in binary mode, unchanging the data
    fp = fopen(outfilename, "wb");

    curl_easy_setopt(curl, CURLOPT_URL, apiURL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    result = curl_easy_perform(curl);


    if (result != CURLE_OK) {
        fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
        fclose(fp);
        return -1;
    }

    fclose(fp);
    curl_easy_cleanup(curl);

    return 0;
}

int parseWeatherData() {
    printf("Test");
    FILE *fp;
    char buffer[1024];

    struct json_object *parsed_json;
    struct json_object *main_obj;
    struct json_object *temp;

    fp = fopen("current-weather.json", "r");
    fread(buffer, 1024, 1, fp);
    fclose(fp);

    printf("JSON file content:\n%s\n", buffer);

    parsed_json = json_tokener_parse(buffer);

    // First, get the "main" object
    json_object_object_get_ex(parsed_json, "main", &main_obj);
    // Then, get the "temp" field inside the "main" object
    json_object_object_get_ex(main_obj, "temp", &temp);

    printf("Temperature: %.2f\n", json_object_get_double(temp));

    // Free the parsed JSON object
    json_object_put(parsed_json);

    return 0;
}
