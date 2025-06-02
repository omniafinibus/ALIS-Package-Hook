#include "main.h"

int main(int argc, char *argv[]) {
    // Check the directory if the file is correct
    if(argc < 2) {
        printf("==> No directory given\n");
        return(0);
    }
    else{
        FILE *file  = fopen(argv[1], "r");
        if (file) {
            printf("==> Overwriting file: %s\n", argv[1]);
            fclose(file);
        } else {
            printf("==> File doesn't exist: %s\n", argv[1]);
            return(0);
        }
    }

    int character;
    FILE *stream;
    char buffer[BUFFER_LENGTH];
    char confBuffer[LABEL_LENGTH];
    uint8_t crntState = STATE_BREAK;

    size_t nameIndex = 0;
    package focusedPackage;

    char aurList[LIST_SIZE];
    char pacList[LIST_SIZE];
    size_t pacIndex = 0;
    size_t aurIndex = 0;

    // Prepare the buffers
    memset(confBuffer, '\0', LABEL_LENGTH);
    memset(buffer, '\0', sizeof(buffer));
    update_buffer(buffer, '\n', BUFFER_LENGTH); // Required to make sure the first package is not skipped

    // Open a new stream
    stream = popen(READ_COMMAND,"r");

    // Check if the stream was successfully opened
    if(stream == NULL) {
        puts("Unable to open process");
        return(1);
    }

    // Read each character of the output
    while( (character=fgetc(stream)) != EOF) {
        //Add the character to the buffer
        update_buffer(buffer, character, BUFFER_LENGTH);

        // A new row has been found
        if (buffer[0] == '\n'){
            // Update the current state
            crntState = get_state(buffer, crntState);
            
            // Check if the new line is the start of a new object
            if (crntState == STATE_NAME) {
                // Add explicitly installed packages to the list
                if(focusedPackage.source == PACMAN && add_entry(pacList, &focusedPackage, &pacIndex)) {
                    // printf("[PAC ADDED] %s\n", focusedPackage.name);
                }
                else if (focusedPackage.source == YAY && add_entry(aurList, &focusedPackage, &aurIndex)) {
                    // printf("[AUR ADDED] %s\n", focusedPackage.name);
                }

                // Reset package values
                nameIndex = 0;
                for (size_t i = 0; i < sizeof(focusedPackage.name)/sizeof(char); i++) {
                    focusedPackage.name[i] = '\0';
                }
                focusedPackage.explicit = false;
                focusedPackage.source = PACMAN;
            }
        }
        else if (!buffer_contains_eol(buffer) && crntState != STATE_BREAK) {
            switch (crntState) {
                case STATE_NAME: // Add the character to the name array
                    focusedPackage.name[nameIndex++] = character;
                    break;
                case STATE_PACK: // Check if the buffer contains the Unknown packager string (Yay packages)
                    if (is_it_aur(buffer)) {
                        focusedPackage.source = YAY;
                        crntState = STATE_BREAK;
                    }
                    break;
                case STATE_REASON: // Set the explicitly installed flag for the crnt object
                    if(character == 'E'){ focusedPackage.explicit = true; }
                    else if(character != 'I'){ printf("==> Warning: Unknown reason of installation found: %s\n", focusedPackage.name); }
                    crntState = STATE_BREAK; // The rest of the string is not important any more
                    break;
                default:
                    printf("Unknown state\n");
                    return(1);
                    break;
            }
        }
    }    

    printf("==> Added packages to ALIS\n");

    pclose(stream);

    // Rewrite the config with the new list
    FILE *config = fopen(argv[1], "w");

    size_t i = 0;
    while (i < sizeof(CONTENTS)/sizeof(char)) {

        // Check for the flag character
        if (CONTENTS[i] == FLAG_CHAR) {
            while(confBuffer[0] != FLAG_CHAR) { // Load whole flag into the buffer
                update_buffer(confBuffer, CONTENTS[i++], LABEL_LENGTH);
            }

            // Check if the LABEL_DATE is present
            if(!strcmp(LABEL_DATE, confBuffer)){
                // Retrieve the system time
                stream = popen(TIME_COMMAND,"r");

                // Check if the stream was successfully opened
                if(stream == NULL) {
                    puts("Unable to read time");
                    fwrite("ERR", sizeof(char), 3, config);
                }
                else {
                    // Write time to the config file
                    while((character=fgetc(stream)) != EOF){
                        fwrite(&character, sizeof(char), 1, config);
                    }
                }
                pclose(stream);
            }
            // Check if the LABEL_PACMAN is present
            else if(!strcmp(LABEL_PACMAN, confBuffer)){
                fwrite(pacList, sizeof(char), pacIndex, config);
            }
            // Check if the LABEL_AUR is present
            else if(!strcmp(LABEL_AUR, confBuffer)){
                fwrite(aurList, sizeof(char), aurIndex, config);
            }

            fwrite(&CONTENTS[i], sizeof(char), 1, config); // Required for the trailing "
            update_buffer(confBuffer, CONTENTS[i++], LABEL_LENGTH); // Make sure to exit this if statement
        }
        else{fwrite(&CONTENTS[i++], sizeof(char), 1, config);} // No flag was found, go the the next character
    }

    fclose(config);
    printf("==> Finished updating file %s", argv[1]);
    
    return(0);
};



bool add_entry(char * listString, package * focusedPackage, size_t * lastChar) {
    /**==============================================
     **              add_entry
     *?  Add the name of a package to a list, which is seperated by a space
     *@param listString char*  
     *@param focusedPackage package  
     *@param lastChar size_t * 
     *@return bool
     *=============================================**/
    if (focusedPackage->explicit) {
        size_t i = 0;
        // Add a space after the last name
        listString[(*lastChar)++] = ' ';

        // Add the string to the list
        while (focusedPackage->name[i] != '\0' && i <= sizeof(focusedPackage->name)/sizeof(char)) {
            listString[(*lastChar)++] = focusedPackage->name[i++];
        }

        // Notify that the package was added
        return true;
    }
    // Package was not added to the list
    return false;
}

bool is_it_aur(char *buffer){
    /**==============================================
     **              is_it_aur
     *?  Check if the packager name is from the AUR or pacman
     *@param buffer char*  
     *@return bool
     *=============================================**/
    //Check if the current buffer holds data indicating the package originates from the AUR
    char * label = "Unknown Packager";
    size_t labelIndex = 0;
    for(size_t i=0; i < BUFFER_LENGTH; i++) {
        // Return true once all characters were found in the correct order
        if (labelIndex >= sizeof(label)/sizeof(char)) {return true;}
        // Go to the next label character
        else if (buffer[i] == label[labelIndex]) { labelIndex++; }
        // Found a break in the match, set labelIndex to 0;
        else{ labelIndex = 0; }
    }

    return false;
}

bool buffer_contains_eol(char *buffer){
    /**==============================================
     **              buffer_contains_eol
     *?  Check if there is a end of line character in the buffer
     *@param buffer char*  
     *@return bool
     *=============================================**/
    for(size_t i = 1; i < BUFFER_LENGTH; i++) {
        if (buffer[i] == '\n') {
            return true;
        }
    }

    // None were found
    return false;
}

void update_buffer(char *buffer, char newChar, size_t bufferSize){
    /**==============================================
     **              update_buffer
     *?  Shift the buffer and add a character to the end of the buffer
     *@param buffer char*  
     *@param newChar char  
     *@param bufferSize size_t  
     *@return void
     *=============================================**/
    // Move all characters left by 1
    for(size_t i = 1; i < bufferSize; i++) {
        buffer[i-1] = buffer[i];
    }

    // Append the new character
    buffer[bufferSize-2] = newChar; // -2 instead of -1 to take last \0 into account
}

uint8_t get_state(char *buffer, uint8_t crntState){
    /**==============================================
     **              get_state
     *?  Get the currentl applicable state based on the contents of the buffer
     *@param buffer char*  
     *@param crntState uint8_t  
     *@return uint8_t
     *=============================================**/
    if (strcmp(buffer, LABEL_NAME) == 0) { return STATE_NAME; }
    if (strcmp(buffer, LABEL_PACK) == 0) { return STATE_PACK; }
    if (strcmp(buffer, LABEL_REASON) == 0) { return STATE_REASON; }
    return -1;
}