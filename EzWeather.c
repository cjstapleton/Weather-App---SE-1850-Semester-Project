#include <curl/curl.h>
#include <json-c/json.h>
#include <ncurses/ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PLAYER_PAIR 5

/* PROTOTYPES */
int weatherData(char *apiURL);
int parseWeatherData();
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
const char *windDegreesConversion(int windDirectionInDegrees);
int createApiURL();
int evaluateCurrentWeather();
void drawWeatherGraphic(int weatherIdentifier);
void printWeatherSuggestions(int weatherIdentifier);
void printInstructions();
int profileSelection();
void cls();
void extremeWeatherWarnings(int weatherIdentifier);

/* GLOBAL VARIABLES */

struct Profile {
char name[50];
int zipCode;
char choice[3];
};
	
struct Profile currentProfile;

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
    cls();
    printf("Welcome to EzWeather\n");
    profileSelection();

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
    // if parseWeatherData returns -1, the zip code is invalid/doesn't exist
    parseWeatherData();
    if (parseWeatherData() == -1) {
        profileSelection();
    }
    
    // FROM THIS POINT FORWARD, ALL GLOBAL VARIABLES HAVE USABLE VALUES
    
    
    // ORIGINAL TEST CASES
    /*
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
    */
    int weatherIdentifier = evaluateCurrentWeather();
    
     // init screen and sets up screen
    initscr();
    // print to screen

	if (has_colors() == FALSE) {
        printw("Your terminal does not support color\n");
    }

    drawWeatherGraphic(weatherIdentifier);
	mvprintw(5, 22, "Current: %.2lf°F", tempValue);
    mvprintw(6, 22, "High: %.2lf°F", tempMax);
	mvprintw(7, 22, "Low: %.2lf°F", tempMin);
	mvprintw(8, 22, "Feels Like: %.2lf°F", feelsLikeValue);
	mvprintw(1, 3, "Profile: %s", currentProfile.name);
    mvprintw(2, 3, "Location: %s", location);
	mvprintw(3, 3, "Description: %s", weatherDescription);

	if (rainPerHour != -1) {
        mvprintw(10, 22, "Rain: %.2lf mm/h", rainPerHour);
    }

    if (snowPerHour != -1) {
        mvprintw(10, 22, "Snow: %.2lf mm/h", snowPerHour);
    }

    if (currentProfile.choice[0] == 'y') {
        mvprintw(5, 45, "Humidity: %d%%", humidityValue);
    }

    if (currentProfile.choice[1] == 'y') {
        mvprintw(6, 45, "Windspeed: %.2lf %s", windSpeed, windDegreesConversion(windDirectionInDegrees));
    }

    if (currentProfile.choice[2] == 'y') {
        mvprintw(7, 45, "Cloud Coverage: %d%%", cloudCoveragePercentage);
    }

    printWeatherSuggestions(weatherIdentifier);
    extremeWeatherWarnings(weatherIdentifier);

    // refreshes the screen
    refresh();
    // pause the screen output
    getch();
    free(mainOverview);
    free(weatherDescription);
    free(location);
    // deallocates memory and ends ncurses
    return 0;
    
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

    // First, get the 'main' object
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
    char *baseURL = "https://api.openweathermap.org/data/2.5/weather?zip=-----&appid=INSERT_API_KEY_HERE&units=imperial";
    
    //scanf("%s", userDefinedLocation);

    sprintf(userDefinedLocation, "%d", currentProfile.zipCode);
    //printf("%s", userDefinedLocation);
     
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
                        
int evaluateCurrentWeather() {
    int weatherIdentifier;

    // compare to current weather main description to print ASCII art and conditional suggestions accordingly
    if (strcmp(mainOverview, "Clear") == 0) {
        weatherIdentifier = 1;
    } else if (strcmp(mainOverview, "Clouds") == 0) {
        weatherIdentifier = 2;
    } else if (strcmp(mainOverview, "Rain") == 0 || 
               strcmp(mainOverview, "Drizzle") == 0 ||
               strcmp(mainOverview, "Thunderstorm") == 0) {
                
        weatherIdentifier = 3;
    } else if (strcmp(mainOverview, "Snow") == 0) {
        weatherIdentifier = 4;
    } else if (strcmp(mainOverview, "Tornado") == 0) {
        weatherIdentifier = 5;
    } else {
        weatherIdentifier = 0;
    }

    return weatherIdentifier;
}

void extremeWeatherWarnings(int weatherIdentifier) {
    if (weatherIdentifier == 5) {
        mvprintw(20, 3, "There is a tornado warning/tornado watch in the area.");
        mvprintw(21, 3, "Please seek shelter accordingly.");
    }

    if (windSpeed >= 58) {
        mvprintw(20, 3, "Severe Thunderstorm Warning: Thunderstorms are likely occuring or imminent in the area.");
    } else if (windSpeed >= 30) {
        mvprintw(20, 3, "Wind Advisory: High wind speeds in the area.");
    }

    if (tempValue >= 100 || (feelsLikeValue >= 90 && humidityValue >= 75)) {
        mvprintw(20, 3, "Heat advisory: High temperature or high feeling temperature in the area.");
    }
}

void drawWeatherGraphic(int weatherIdentifier) {
    switch (weatherIdentifier) {
        case 0:
            start_color();
            //init_pair(PLAYER_PAIR, COLOR_RED, COLOR_MAGENTA);
            //attron(COLOR_PAIR(PLAYER_PAIR));
            mvprintw(5, 3, "        .~~.");
            mvprintw(6, 3, "      _(    )");
            mvprintw(7, 3, " .-._(       '.");
            mvprintw(8, 3, "(______________)");

            break; 
        case 1:
            start_color();
            init_pair(PLAYER_PAIR, COLOR_YELLOW, COLOR_BLACK);
            attron(COLOR_PAIR(PLAYER_PAIR));
            mvprintw(5, 3, "      \\ | /");
            mvprintw(6, 3, "    '-.;;;.-'");
            mvprintw(7, 3, "   -==;;;;;==-");
            mvprintw(8, 3, "    .-';;;'-.");
            mvprintw(9, 3, "      / | \\");
            attroff(COLOR_PAIR(PLAYER_PAIR));;

            break;
        case 2:
        	start_color();
            //init_pair(PLAYER_PAIR, COLOR_RED, COLOR_MAGENTA);
            //attron(COLOR_PAIR(PLAYER_PAIR));
            mvprintw(5, 3, "        .~~.");
            mvprintw(6, 3, "      _(    )");
            mvprintw(7, 3, " .-._(       '.");
            mvprintw(8, 3, "(______________)");

            break;
        case 3:
            start_color();
            mvprintw(5, 3, "        .~~.");
            mvprintw(6, 3, " .--.__(    )._");
            mvprintw(7, 3, "(______________')");
            init_pair(PLAYER_PAIR, COLOR_BLUE, COLOR_BLACK);
            attron(COLOR_PAIR(PLAYER_PAIR));
            mvprintw(8, 3, "  :;|;||:||:|;|");
            attroff(COLOR_PAIR(PLAYER_PAIR));

            break;
        case 4:
            start_color();
            mvprintw(5, 3, "        .~~.");
            mvprintw(6, 3, " .--.__(    )._");
            mvprintw(7, 3, "(______________')");
            init_pair(PLAYER_PAIR, COLOR_CYAN, COLOR_BLACK);
            attron(COLOR_PAIR(PLAYER_PAIR));
            mvprintw(8, 3, "  +*#*#+*##*+*#");
            attroff(COLOR_PAIR(PLAYER_PAIR));

            break;
    }
}

void printWeatherSuggestions(int weatherIdentifier) {
    switch (weatherIdentifier) {
    // only need case 3 and 4, because those are rain and snow.
    // 0, 1, and 2 should all be roughly the same. (0 being anything besides the listed conditions, and 1/2 are clear/cloudy)
        case 3:
            mvprintw(15, 1, "It is currently raining.");
            mvprintw(16, 1, "Stay dry by wearing layers and also a raincoat, or use an umbrella.");
            mvprintw(17, 1, "Stay inside and stay dry; watch a movie or read a book.");

            break;
        case 4:
            mvprintw(15, 1, "It is currently snowing.");
            mvprintw(16, 1, "Stay warm by wearing layers and an insulated coat.");
            mvprintw(17, 1, "Stay inside and stay warm; read a book by the fire.");

            break;
        default:
            if (feelsLikeValue < 0) {
                mvprintw(15, 1, "It currently feels like it is below 0 degrees.");
                mvprintw(16, 1, "Wear warm layers and an insulated coat.");
                mvprintw(17, 1, "Stay inside today. Keep warm with a fireplace and watch a movie.");
            } else if (feelsLikeValue < 32) {
                mvprintw(15, 1, "It currently feels like it is below freezing.");
                mvprintw(16, 1, "Make sure to wear a jacket and warm clothing.");
                mvprintw(17, 1, "Stay inside if possible today. Read a book indoors or play a boardgame.");
            } else if (feelsLikeValue < 60) {
                mvprintw(15, 1, "It currently feels like it is below 60 degrees.");
                mvprintw(16, 1, "It may be a bit chilly, wear a jacket outside.");
                mvprintw(17, 1, "Consider staying inside today. Maybe watch a movie or go for a small walk.");
            } else if (feelsLikeValue < 70) {
                mvprintw(15, 1, "It currently feels like it is around 70 degrees.");
                mvprintw(16, 1, "It is very nice outside, no need for a coat!");
                mvprintw(17, 1, "Today is a good day to go outside. Go to the park or take the dog for a walk.");
            } else if (feelsLikeValue < 80) {
                mvprintw(15, 1, "It currently feels like it is around 80 degrees.");
                mvprintw(16, 1, "It's a little bit warm, dress lightly.");
                mvprintw(17, 1, "It is hot outside. Go to the pool/beach and stay cool.");
            } else {
                mvprintw(15, 1, "It currently feels like it is over 80 degrees.");
                mvprintw(16, 1, "It is likely pretty hot out, consider a t-shirt and light clothing.");
                mvprintw(17, 1, "It is very hot, keep cool at the beach/pool or stay in the air conditioning.");
            }
            
            break;
    }
}

void printInstructions() {
    printf("How to use: \n");
    printf("Create a profile with a five digit zip code to view the weather at.\n");
    printf("If the zip code is invalid or otherwise does not exist, you will be given an error.\n");
    printf("\n");

    printf("You can choose your settings by entering the settings menu and following the prompts.\n");
    printf("By changing your settings, you may choose which optional conditions you would like to see.\n");
    printf("These settings will be saved to your profile.\n");
    printf("\n");
}

int profileSelection() {
	//Create flag for loop and struct for profiles
	char display = 'n';
	
	
	FILE *fptr;

    struct Profile profiles[3];  // Array to store up to 3 profiles
    int profileCount = 0;         // To keep track of how many profiles were read
	
	
	fptr = fopen("profiles.txt", "r");
    if (fptr == NULL) {
        // If the file cannot be opened, print an error message and exit
        perror("Error opening file");
        return 1;
    }

    // Read the profiles until the end of the file or the array limit is reached
    while (fscanf(fptr, "%s %d %s", profiles[profileCount].name, 
                   &profiles[profileCount].zipCode, 
                   profiles[profileCount].choice) == 3) {
        profileCount++;

        // If we exceed the array limit, stop reading further
        if (profileCount >= 10) {
            printf("Maximum number of profiles (10) reached.\n");
            break;
        }
    }

    // Close the file after reading
    fclose(fptr);
	
	fptr = fopen("profiles.txt", "w");		
	// Write some text to the file
	
	fprintf(fptr, "%s %d %s\n", profiles[0].name, profiles[0].zipCode, profiles[0].choice);
	fprintf(fptr, "%s %d %s\n", profiles[1].name, profiles[1].zipCode, profiles[1].choice);
	fprintf(fptr, "%s %d %s\n", profiles[2].name, profiles[2].zipCode, profiles[2].choice);
	

	// Close the file
	fclose(fptr);

    int currpro;
	while(display != 'y') {
		if (parseWeatherData() == -1) {
            printf("Error retreiving previous request: Zip code for profile is invalid.\n");
        }
        
        //initialize local variables
		int initchoice = -1;
		currpro = -1;
		char usernamestring[20];
		
		char new = 'p';
		
		//ask question to go into switch funtcion
		printf("\nPlease select an option:\nDisplay the weather (1)\nChange/add weather profiles (2)\nSee a guide to our app (3)\n");
		
		while(initchoice < 1 || initchoice > 3) {
			scanf(" %d", &initchoice);
			if(initchoice < 1 || initchoice > 3) {
				printf("That is not one of your choices; choose 1, 2, or 3.\n");
            }
		}
		
		switch(initchoice){
			case(1):
				cls();
                //check if any available profiles to display
				if(profiles[0].name[0] == '?' && profiles[1].name[0] == '?' && profiles[2].name[0] == '?') {
					printf("No old profiles. \n");
					break;
				}
				//choose which one to display
				printf("Which profile would you like to display? \n1. %s\n2. %s\n3. %s\n(Enter 1, 2, or 3) ", profiles[0].name, profiles[1].name, profiles[2].name);
				while(currpro < 1|| currpro > 3) { 
					scanf("%d", &currpro);
					if(currpro > 0 && currpro < 4)
						break;
					printf("That is not a profile. Try Again.\n");
				}
				//print profile values
				printf("\nProfile: %s \n", profiles[currpro-1].name);
				printf("Zip code: %d\n", profiles[currpro-1].zipCode);
				printf("Humidity: %c\n", profiles[currpro-1].choice[0]);
				printf("Wind speed: %c\n", profiles[currpro-1].choice[1]);
				printf("Cloud Coverage: %c\n", profiles[currpro-1].choice[2]);
				
				
				//ask if they want to go into ncurses display
				printf("Would you like to see the report for this profile? (y/n) ");
				scanf(" %c", &display);
                cls();
				break;
			case(2):
				cls();
                // ask if they want to delete or create profile
				printf("\nWould you like to add a new profile or delete an old one? (n/d) ");
				scanf(" %c", &new);
				//choose which profile slot to put new info in
				if(new == 'n'){
					if(profiles[0].name[0] == '?')
						currpro = 1;
					else if(profiles[1].name[0] == '?')
						currpro = 2;
					else if(profiles[2].name[0] == '?')
						currpro = 3;
					else {
						printf("Profiles full; erase one first.");
						break;
					}
					//ask and assign relative values based on user input for new values
					printf("Enter profile name: ");
					scanf("%s", usernamestring);
					strcpy(profiles[currpro-1].name, usernamestring);

					printf("Enter zip code: ");
					scanf("%d", &profiles[currpro-1].zipCode);
                    char tempCheck[6]; 

                    sprintf(tempCheck, "%d", profiles[currpro-1].zipCode);

                    while (strlen(tempCheck) != 5) {
                        printf("Please enter a valid zip code. ");
                        scanf("%d", &profiles[currpro-1].zipCode);
                        sprintf(tempCheck, "%d", profiles[currpro-1].zipCode);
                    }

					printf("Display humidity? (y/n) ");
					scanf(" %c", &profiles[currpro-1].choice[0]);

					printf("Display wind speed? (y/n) ");
					scanf(" %c", &profiles[currpro-1].choice[1]);

					printf("Display cloud coverage? (y/n) ");
					scanf(" %c", &profiles[currpro-1].choice[2]);
					
                    cls();
				}
				if(new == 'd'){
					//change profile name to ? to "delete"
					printf("Which profile would you like to delete? \n1. %s\n2. %s\n3. %s\n(Enter 1, 2, or 3) ", profiles[0].name, profiles[1].name, profiles[2].name);
					scanf("%d", &currpro);
					if(currpro == 1)
						strcpy(profiles[0].name, "?");
					if(currpro == 2)
						strcpy(profiles[1].name, "?");
					if(currpro == 3)
						strcpy(profiles[2].name, "?");
				}
                cls();
				break;
			case(3):
                cls();
                printInstructions();
				break;
		}
			// Open profile file in writing mode
		fptr = fopen("profiles.txt", "w");
		
		// Write profiles to the file
		fprintf(fptr, "%s %d %s\n", profiles[0].name, profiles[0].zipCode, profiles[0].choice);
		fprintf(fptr, "%s %d %s\n", profiles[1].name, profiles[1].zipCode, profiles[1].choice);
		fprintf(fptr, "%s %d %s\n", profiles[2].name, profiles[2].zipCode, profiles[2].choice);

		// Close the file
		fclose(fptr);
		
	}
    
    currentProfile = profiles[currpro-1];

}

void cls() {
    printf("\033[2J"); // Clear the entire screen
    printf("\033[H");  // Move the cursor to the top-left corner
}
