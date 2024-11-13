#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
	
	char name1[] = "Emmett";
	char name2[] = "Collin";
	char name3[] = "Steve";
	
	int zip1 = 52332;
	int zip2 = 52349;
	int zip3 = 50012;
	
	int choices1 = 10101110;
	int choices2 = 11010101;
	int choices3 = 11110101;
	
	struct Profile {
    char username[50];
    int zipCode;
    int otherInteger;
	};
	
	
	FILE *fptr;

	// Open a file in writing mode
	fptr = fopen("example.txt", "w");

	// Write some text to the file
	fprintf(fptr, "%s %d %d\n", name1, zip1, choices1);
	fprintf(fptr, "%s %d %d\n", name2, zip2, choices2);
	fprintf(fptr, "%s %d %d\n", name3, zip3, choices3);

	// Close the file
	fclose(fptr);
	
	
	
// Define a structure to hold profile information



    FILE *file;
    struct Profile profiles[10];  // Array to store up to 10 profiles
    int profileCount = 0;         // To keep track of how many profiles were read

    // Open the file in read mode
    file = fopen("example.txt", "r");
    if (file == NULL) {
        // If the file cannot be opened, print an error message and exit
        perror("Error opening file");
        return 1;
    }

    // Read the profiles until the end of the file or the array limit is reached
    while (fscanf(file, "%s %d %d", profiles[profileCount].username, 
                   &profiles[profileCount].zipCode, 
                   &profiles[profileCount].otherInteger) == 3) {
        profileCount++;

        // If we exceed the array limit, stop reading further
        if (profileCount >= 10) {
            printf("Maximum number of profiles (10) reached.\n");
            break;
        }
    }

    // Close the file after reading
    fclose(file);

    // Print the profiles that were read
    printf("Profiles read from file:\n");
    for (int i = 0; i < profileCount; i++) {
        printf("Profile #%d\n", i + 1);
        printf("  Username: %s\n", profiles[i].username);
        printf("  Zip Code: %d\n", profiles[i].zipCode);
        printf("  Other Integer: %d\n\n", profiles[i].otherInteger);
    }

    return 0;
}
