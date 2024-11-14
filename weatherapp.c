#include <curl/curl.h>
#include <json-c/json.h>
#include <ncurses/ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// api url for ames iowa in imperial units: https://api.openweathermap.org/data/2.5/weather?zip=50012&appid=020c9db2f8a57004fe2e82f6e1bbd905&units=imperial

/*
TO DO
[x] NEED TO ADD RAIN AND SNOW VARIABLES
[x] file system
[ ] dynamic array resizing for string variables
[x] conversion of wind degrees to cardinal directions
[ ] branch conditionals
    [ ] extreme weather conditions
    [ ] what to wear
    [ ] sun tanning/UV
    [ ] activities
[ ] update what conditions you would like to see
[ ] how to use guide 
[ ] maybe a bound limit for zipcode to prevent segmentation fault on invalid zipcode?
*/

/* PROTOTYPES */
int weatherData(char *apiURL);
int parseWeatherData();
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
const char *windDegreesConversion(int windDirectionInDegrees);

/* GLOBAL VARIABLES */

// under 'weather' object
char mainOverview[20];
char weatherDescription[30];

// under 'main' object
double tempValue;
double feelsLikeValue;
double tempMin;
double tempMax;
int humidityValue;

// under 'wind' object
double windSpeed;
int windDirectionInDegrees;

// under 'rain' object
double rainPerHour = -1;

// under 'snow' object
double snowPerHour = -1;

// under 'clouds' object
int cloudCoveragePercentage;

// lone object
char location[30];



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

    // Makes the API call and creates current-weather.json
    weatherData(apiURL);
    
    // Parses current-weather.json and updates global variables 
    parseWeatherData();

    // FROM THIS POINT FORWARD, ALL GLOBAL VARIABLES HAVE USABLE VALUES

    printf("Main: %s\n", mainOverview);
    printf("Description: %s\n", weatherDescription);

    printf("Temp: %.2lf°F\n", tempValue);
    printf("Feels like: %.2lf°F\n", feelsLikeValue);
    printf("Min temp: %.2lf°F\n", tempMin);
    printf("Max temp: %.2lf°F\n", tempMax);
    printf("Humidity: %d\n", humidityValue);

    printf("Wind speed: %.2lf miles per hour\n", windSpeed);
    // TEST LINE
    // printf("Wind Direction: %d\n", windDirectionInDegrees);
    printf("Wind Direction: %s\n", windDegreesConversion(windDirectionInDegrees));

    if (rainPerHour != -1) {
        printf("Rain (millimeters per hour): %.2lf percent\n", rainPerHour);
    }

    if (snowPerHour != -1) {
        printf("Snow (millimeters per hour): %.2lf\n", snowPerHour);
    }

    printf("Cloud Coverage: %d\n", cloudCoveragePercentage);

    printf("Location: %s\n", location);
    


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
    FILE *fp;
    char buffer[1024];

    struct json_object *parsed_json;
    
    struct json_object *weather_array;
    struct json_object *weather_obj;
    struct json_object *main;
    struct json_object *description;
    
    struct json_object *main_obj;
    struct json_object *temp;
    struct json_object *feels_like;
    struct json_object *temp_min;
    struct json_object *temp_max;
    struct json_object *humidity;
    
    struct json_object *wind_obj;
    struct json_object *speed;
    struct json_object *deg;

    struct json_object *rain_obj;
    struct json_object *rain;

    struct json_object *snow_obj;
    struct json_object *snow;

    struct json_object *clouds_obj;
    struct json_object *all;

    struct json_object *name;

    fp = fopen("current-weather.json", "r");
    fread(buffer, 1024, 1, fp);
    fclose(fp);

    //printf("JSON file content:\n%s\n", buffer);

    parsed_json = json_tokener_parse(buffer);

    // Retrieve 'weather' object
    json_object_object_get_ex(parsed_json, "weather", &weather_array);
    weather_obj = json_object_array_get_idx(weather_array, 0);
    
    // Retrieve 'main' and 'description' from 'weather' object
    json_object_object_get_ex(weather_obj, "main", &main);
    json_object_object_get_ex(weather_obj, "description", &description);

    // First, get the "main" object
    json_object_object_get_ex(parsed_json, "main", &main_obj);
    // Then, get the fields inside the "main" object
    json_object_object_get_ex(main_obj, "temp", &temp);
    json_object_object_get_ex(main_obj, "feels_like", &feels_like);
    json_object_object_get_ex(main_obj, "temp_min", &temp_min);
    json_object_object_get_ex(main_obj, "temp_max", &temp_max);
    json_object_object_get_ex(main_obj, "humidity", &humidity);

    // Retrieve 'wind' object
    json_object_object_get_ex(parsed_json, "wind", &wind_obj);
    // Retrieve 'speed' and 'deg' from 'wind' object
    json_object_object_get_ex(wind_obj, "speed", &speed);
    json_object_object_get_ex(wind_obj, "deg", &deg);

    // Retrieve 'rain' object IF it exists
    json_object_object_get_ex(parsed_json, "rain", &rain_obj);
    if (rain_obj != NULL) {
        json_object_object_get_ex(rain_obj, "1h", &rain);
        rainPerHour = json_object_get_double(rain);
    }

    // Retrieve 'snow' object IF it exists
    json_object_object_get_ex(parsed_json, "snow", &snow_obj);
    if (snow_obj != NULL) {
        json_object_object_get_ex(snow_obj, "1h", &snow);
        snowPerHour = json_object_get_double(snow);
    }


    // Retrieve 'clouds' object
    json_object_object_get_ex(parsed_json, "clouds", &clouds_obj);
    // Retrieve 'all' from 'clouds' object
    json_object_object_get_ex(clouds_obj, "all", &all);

    //Retrieve 'name' object
    json_object_object_get_ex(parsed_json, "name", &name);



    // Updaate values of global variables
    strcpy(mainOverview, json_object_get_string(main));
    strcpy(weatherDescription, json_object_get_string(description));
    
    tempValue = json_object_get_double(temp);
    feelsLikeValue = json_object_get_double(feels_like);
    tempMin = json_object_get_double(temp_min);
    tempMax = json_object_get_double(temp_max);
    humidityValue = json_object_get_int(humidity);

    windSpeed = json_object_get_double(speed);
    windDirectionInDegrees = json_object_get_int(deg);

    cloudCoveragePercentage = json_object_get_int(all);

    strcpy(location, json_object_get_string(name));



    // Free the parsed JSON object
    json_object_put(parsed_json);

    return 0;
}

const char *windDegreesConversion(int windDegrees) {
    if (windDegrees > 337 || windDegrees < 22.5) {
        return "N";
    } else if (windDegrees < 67) {
        return "NE";
    } else if (windDegrees < 112) {
        return "E";
    } else if (windDegrees < 157) {
        return "SE";
    } else if (windDegrees < 202) {
        return "S";
    } else if (windDegrees < 247) {
        return "SW";
    } else if (windDegrees < 292) {
        return "W";
    } else if (windDegrees < 337) {
        return "NW";
    }
}