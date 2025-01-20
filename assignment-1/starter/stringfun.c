#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here


int setup_buff(char *buff, char *user_str, int len){
    char *buffer = buff;
	const char *str = user_str;
	int count = 0;
	int space = 0;
	int str_len = 0;

 	while (*str != '\0') {
		// Check if the current character is a space or tab
        if (*str == ' ' || *str == '\t') {  
        // If it's a space or tab and the previous character was not a space   
           if (space == 0) {  
                *buffer = ' ';
                buffer++;
                count++;
                space = 1; 
            }
        } else {
        //copy the character from the user string to the buffer
            *buffer = *str; 
            buffer++;
            count++;
            space = 0; 
        }
        str++; 
    }

	//if string is empty make buffer only dots
	if (count == 1 && *buffer == '\0') {
		*(buffer - 1) = '.';
		count = 0;
	}
	
	//handling if string is larger than buffer max
	if (count > len) {
        printf("The user supplied string is too large");
    	return -1;

}

	str_len = count;

	//filling the rest of the buffer with dots 
	while (count < len) {
		*buffer = '.';
		buffer++;
		count++;
	}

	return str_len;

}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    int count = 0;
	int word_found = 0;

    //handling if the string is too large
	if (str_len > len) {
		printf("The user supplied string is too large");
        return -1;
	}
	
	//looping through buffer to count each word based on spaces or dots
	for (int i = 0; i < str_len; i++) {
		if (buff[i] == ' ' || buff[i] == '.') {
			if (word_found) {
				word_found = 0;
			}
		} else {
			if (!word_found) {
                count++;  
                word_found = 1;
			}
		}
	}
    return count;
}

void reverse_string(char *buff, int str_len) {
	char *end = buff + str_len -1;
	char reversed[50]; 
	char *reversed_ptr = reversed;

    //copies reversed version of buffer into a new string
	while (end >= buff) {
		*reversed_ptr = *end;
		end--;
		reversed_ptr++;
	}
	
	*reversed_ptr = '\0';
	
	printf("Reversed string: %s\n", reversed);
}

int word_print(char *buff, int len, int str_len) {
    if (str_len > len) {
        return -1;
    }

    int char_count = 0;
    int word_count = 0;
    int i = 0;

    printf("Word Print\n");
    printf("----------\n");

    // Loop through each character of the buffer
    while (i < str_len) {
        if (buff[i] != ' ' && buff[i] != '\0' && buff[i] != '\t') {
            // Print each word
            char_count++;
        } else {
            if (char_count > 0) {
                word_count++;
                printf("%d. ", word_count);
                for (int j = i - char_count; j < i; j++) {
                    printf("%c", buff[j]);
                }
                printf(" (%d)\n", char_count);
                //reset for next word
                char_count = 0;
            }
        }
        i++;
    }

    // Print the last word if there is one
    if (char_count > 0) {
        word_count++;
        printf("%d. ", word_count);
        for (int j = i - char_count; j < i; j++) {
            printf("%c", buff[j]);
        }
        printf(" (%d)\n", char_count);
    }

    return 0;
}
//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    // #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //     argc is the number of arguments so by checking if argc < 2 it is seeing if more than one argument was inputted.
    //     The first argument is always the program name so if argc < 2 this means ther was no command or flag given. 
    //     Its also safe to use argv[1] as this is checking if the first character in the givan flag is a - . The correct
    //     format for a flag is -[valid letter]. Hence, if it doesnt start with the hypen, its invalid.
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    // #2 Document the purpose of the if statement below
    //    checking to see if a string was given in the input. If there was no sring--so only the file name and the flag--
    //    the program will exit.
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    // #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
	buff = (char*)malloc(BUFFER_SZ + 1);
	if (buff == NULL) {
		printf("Memory allocation failed\n");
		exit(99);
	}

	
    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':

        	printf("The string length is %d\n",user_str_len);
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;
        case 'r':
			reverse_string(buff, user_str_len);       	
        	break;

        case 'w':
			word_print(buff, BUFFER_SZ, user_str_len); 
        break;
        //  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        
        case 'x':
        	printf("Not implemented!\n");
        	break;

        default:
            usage(argv[0]);
            exit(1);
    }

    //  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?

//  		Passing both the pointer and length just makes things safer. It avoids assuming the buffer size, 
//          helps prevent buffer overflows, and gives more flexibility if the buffer size changes. It also
//          makes it clearer what the function is supposed to do and helps catch errors better. 
