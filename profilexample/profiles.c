#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
	//Create flag for loop and struct for profiles
	char display = 'n';
	
	struct Profile {
    char name[50];
    int zipCode;
    char choice[3];
	};
	
	
	FILE *fptr;

    struct Profile profiles[3];  // Array to store up to 3 profiles
    int profileCount = 0;         // To keep track of how many profiles were read
	
	
	fptr = fopen("example.txt", "r");
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
	
	fptr = fopen("example.txt", "w");		
	// Write some text to the file
	
	fprintf(fptr, "%s %d %s\n", profiles[0].name, profiles[0].zipCode, profiles[0].choice);
	fprintf(fptr, "%s %d %s\n", profiles[1].name, profiles[1].zipCode, profiles[1].choice);
	fprintf(fptr, "%s %d %s\n", profiles[2].name, profiles[2].zipCode, profiles[2].choice);
	

	// Close the file
	fclose(fptr);
	
	printf("Hello Welcome to the Weather App\n");
	while(display != 'y') {
		//initialize local variables
		int initchoice = -1;
		int currpro = -1;
		char usernamestring[20];
		
		char new = 'p';
		
		//ask question to go into switch funtcion
		printf("Would you like to display weather(1), change/add weather profiles(2), or see a guide to our app(3)\n");
		
		while(initchoice < 1 || initchoice > 3) {
			scanf(" %d", &initchoice);
			if(initchoice < 1 || initchoice > 3)
				printf("That is not one of your choices, choose 1, 2, or 3.\n");
		}
		
		switch(initchoice){
			case(1):
				//check if any available profiles to display
				if(profiles[0].name[0] == '?' && profiles[1].name[0] == '?' && profiles[2].name[0] == '?') {
					printf("No old profiles\n");
					break;
				}
				//choose which one to display
				printf("Which profile would you like to display? %s, %s, %s(1,2,3)\n", profiles[0].name, profiles[1].name, profiles[2].name);
				while(currpro < 1|| currpro > 3) { 
					scanf("%d", &currpro);
					if(currpro > 0 && currpro < 4)
						break;
					printf("That is not a profile. Try Again.\n");
				}
				//print profile values
				printf("Profile : %s \n", profiles[currpro-1].name);
				printf("Zip code : %d\n", profiles[currpro-1].zipCode);
				printf("Humidity : %c\n", profiles[currpro-1].choice[0]);
				printf("Wind speed : %c\n", profiles[currpro-1].choice[1]);
				printf("Cloud Coverage : %c\n", profiles[currpro-1].choice[2]);
				
				
				//ask if they want to go into ncurses display
				printf("Would you like to see this info?(y/n)\n");
				scanf(" %c", &display);
				break;
			case(2):
				// ask if they want to delete or create profile
				printf("Would you like to add a new profile or delete an old one?(n/d)\n");
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
						printf("Profiles full erase one first");
						break;
					}
					printf("%d", currpro);
					//ask and assign relative values based on user input for new values
					printf("What would you like your profile name to be?\n");
					scanf("%s", usernamestring);
					strcpy(profiles[currpro-1].name, usernamestring);
					printf("What is the zip code of preferred weather?\n");
					scanf("%d", &profiles[currpro-1].zipCode);
					printf("Would you like to display humidity?(y/n)\n");
					scanf(" %c", &profiles[currpro-1].choice[0]);
					printf("Would you like to display wind speed?(y/n)\n");
					scanf(" %c", &profiles[currpro-1].choice[1]);
					printf("Would you like to display cloud coverage?(y/n)\n");
					scanf(" %c", &profiles[currpro-1].choice[2]);
					
				}
				if(new == 'd'){
					//change profile name to ? to "delete"
					printf("Which profile would you like to delete? %s, %s, %s(1,2,3)\n", profiles[0].name, profiles[1].name, profiles[2].name);
					scanf("%d", &currpro);
					if(currpro == 1)
						strcpy(profiles[0].name, "?");
					if(currpro == 2)
						strcpy(profiles[1].name, "?");
					if(currpro == 3)
						strcpy(profiles[2].name, "?");
				}
				break;
			case(3):
				printf("HOW TO\n");
				break;
		}
			// Open profile file in writing mode
		fptr = fopen("example.txt", "w");
		
		// Write profiles to the file
		fprintf(fptr, "%s %d %s\n", profiles[0].name, profiles[0].zipCode, profiles[0].choice);
		fprintf(fptr, "%s %d %s\n", profiles[1].name, profiles[1].zipCode, profiles[1].choice);
		fprintf(fptr, "%s %d %s\n", profiles[2].name, profiles[2].zipCode, profiles[2].choice);

		// Close the file
		fclose(fptr);
		
	
	}
    
}
