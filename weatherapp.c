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
[x] dynamic array sizing for string variables
[x] conversion of wind degrees to cardinal directions
[ ] branch conditionals
    [ ] extreme weather conditions
    [ ] what to wear
    [ ] sun tanning/UV
    [ ] activities
[ ] update what conditions you would like to see
[ ] how to use guide 
[x] error message for zip code that does not exist
    - had to create pointers for apiURL and turn user input loop into a functino for simplicity
    - maybe go back to createApiUrl and see if it can be condensed further?
        (uses lots of pointers, maybe not necessary)
*/

/* PROTOTYPES */
int weatherData(char *apiURL);
int parseWeatherData();
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
const char *windDegreesConversion(int windDirectionInDegrees);
int createApiURL();

/* GLOBAL VARIABLES */

// under 'weather' object
char *mainOverview = NULL;
char *weatherDescription = NULL;

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
double rainPerHour = -1; // This value will be changed accordingly if the 'rain' object exists.

// under 'snow' object
double snowPerHour = -1; // This value will be changed accordingly if the 'snow' object exists.

// under 'clouds' object
int cloudCoveragePercentage;

// lone object
char *location = NULL;


int main (void) {
    printf("Enter the zip code you would like to see the weather at.\n");

    //retrieve user input
    char apiURL[256];
    char *apiURLPtr = apiURL;

    // createApiURL function will 
    createApiURL(apiURLPtr);
    // TEST LINE - Check to make sure createApiUrl correctly copied the API URL to apiURL
    // printf("%s\n", apiURL);

    // *FROM THIS POINT, apiURL IS IN A USABLE STATE

    // Makes the API call and creates/updates current-weather.json
    weatherData(apiURL);
    
    // Parses current-weather.json and updates global variables 
    // if the zip code is invalid, parseWeatherData() will return -1.
    parseWeatherData();
    
    while (parseWeatherData() == -1) {
        printf("The zip code you entered does not exist or is otherwise invalid.\n");
        printf("Please enter another zip code.\n");
        printf("\n");

        createApiURL(apiURLPtr);
        weatherData(apiURL);
        parseWeatherData();
    }
    
    // FROM THIS POINT FORWARD, ALL GLOBAL VARIABLES HAVE USABLE VALUES
    printf("\n");
    printf("Location: %s\n", location);
    
    printf("\n");
    printf("Main: %s\n", mainOverview);
    printf("Description: %s\n", weatherDescription);

    printf("\n");
    printf("Temp: %.2lf°F\n", tempValue);
    printf("Feels like: %.2lf°F\n", feelsLikeValue);
    printf("Min temp: %.2lf°F\n", tempMin);
    printf("Max temp: %.2lf°F\n", tempMax);
    printf("Humidity: %d%%\n", humidityValue);


    printf("\n");
    printf("Wind speed: %.2lf miles per hour %s\n", windSpeed, 
        windDegreesConversion(windDirectionInDegrees));
    // TEST LINE - Prints wind direction in meteorological degrees
    // (Use to test funcionality of windDegreesConversion function)
    // printf("Wind Direction: %d\n", windDirectionInDegrees);

    if (rainPerHour != -1) {
        printf("\n");
        printf("Rain: %.2lf millimeters per hour\n", rainPerHour);
    }

    if (snowPerHour != -1) {
        printf("\n");
        printf("Snow: %.2lf millimeters per hour\n", snowPerHour);
    }

    printf("\n");
    printf("Cloud Coverage: %d%%\n", cloudCoveragePercentage);

    free(mainOverview);
    free(weatherDescription);
    free(location);

}

// use curl to make an HTTP request and
// write the weather forecast into a .json file using 
// OpenWeather's current weather API
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

    struct json_object *message;
    
    // Create many json_object structs that are used to access object values
    // in the .json file
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

    // TEST LINE - Prints unorganized contents of current-weather.json
    //printf("JSON file content:\n%s\n", buffer);

    parsed_json = json_tokener_parse(buffer);

    // check for invalid zip code
    json_object_object_get_ex(parsed_json, "message", &message);
    if (message != NULL) {
        return -1;
    }


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

    // Update values of global variables
    
    // allocate memory for strings
    mainOverview = malloc(sizeof(char) * strlen(json_object_get_string(main)));
    weatherDescription = malloc(sizeof(char) * strlen(json_object_get_string(description)));
    location = malloc(sizeof(char) * strlen(json_object_get_string(name)));
    
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

// callback function to handle data from API
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int createApiURL(char *apiURL) {
    char userDefinedLocation[6];
    char *baseURL = "https://api.openweathermap.org/data/2.5/weather?zip=-----&appid=020c9db2f8a57004fe2e82f6e1bbd905&units=imperial";
    
    //input loop to make sure user enters a 5 digit zip code
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
    char apiURLTemp[256];

    // copy baseURL to apiURL
    strcpy(apiURLTemp, baseURL);
    
    // replace each instance of '-' with each num in userDefinedLocation
    // since strchr() returns the first instance of '-' in the baseURL, 
    // we know that the first five characters are '-', so we loop over the first
    // five and replace them with each digit of the user inputted zip code
    for (int i = 0; i < 5; i++) {
        apiURLTemp[strInsertionIndexNum] = userDefinedLocation[i];

        strInsertionIndexNum += 1;
    }

    strcpy(apiURL, apiURLTemp);

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