/*
 * To compile this code, run the make command in this directory.
 * To run the program, type ./unixshell
*/

// Include library statements
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "unixshell.h"

// Constant that represents the maximum number of characters a command can be
#define INPUTLENGTH 100

// Main method for program execution
int main(int argc, char **argv) {

	// Declare necessary variables
	char* prompt = "308sh> "; // shell prompt
	char input[INPUTLENGTH]; // input from user
	
	// Check if invalid command line arguments were received and set prompt if so
	if (argc != 1 && argc != 3) {
		// If so, return from main to exit program
		printf("Invalid number of command line arguments. Expected 2.\n");
		return 0;
	} else if (argc == 3) { // check if there are 2 command line arguments
		// check if the first argument is -p
		if (strlen(argv[1]) != 2 || strncmp(argv[1], "-p", 2) != 0) {
			// If so, return from main to exit program
			printf("Invalid command line arguments. Expecting [-p prompt]\n");
			return 0;
		} else {
			// Otherwise, set prompt variable to second command line argument
			prompt = argv[2];
		}
	}
	
	// User input infinite loop
	while (1) {
	
		// Give the user the prompt
		printf(prompt);
		
		// Get user input
		fgets(input, INPUTLENGTH, stdin);
		
		// Convert user input's last character from newline to null terminator
		int inputLength = strlen(input);
		if (inputLength > 0 && input[inputLength-1] == '\n') {
			input[inputLength-1] = '\0';
		}
		
		// ---------- Builtin Commands ---------------------------------------
		
		checkBackgroundProcesses();
		
		// Continue to next iteration if user enters blank command
		if (strlen(input) == 0) {
			continue;
		}
		
		// Check if user gave exit command
		if (strcmp(input, "exit") == 0) {
			// exit if so
			return 0;
		} else if (strcmp(input, "pid") == 0) { // Check if user gave pid command and give process ID of shell
			printf("Shell Process ID: %d\n", getpid());
		} else if (strcmp(input, "ppid") == 0) { // Check if user gave ppid command and give parent process ID
			printf("Shell's Parent Process ID: %d\n", getppid());
		} else if (strncmp(input, "cd", 2) == 0) { // Check user gave cd command and change to specified directory or home
			getCd(input);
		} else if (strcmp(input, "pwd") == 0) { // Check if user have pwd command and print current working directory
			getPwd();
		} else if (strncmp(input, "set", 3) == 0) { // Check if user gave set environment variable
			setEnvVars(input);
		} else if (strncmp(input, "get", 3) == 0) { // Check if user gave get environment variable and print current value
		 	getEnvCmd(input);
		 } else {
		
			// ---------- Non-Builtin Commands ------------------------------------
			
			// Determine return result and return accordingly
			int returnResult = nonBuiltIn(input);
			
			if (returnResult == 255) {
				return 255;
			}	
		}
	}

	return 0; 
}

// ----------------------------------- Helper Functions ---------------------------------------------

void getPwd() {
	// Stores pwd directory information
	char pwdOutput[1024]; 

	// Declare char array to store pwd in
	getcwd(pwdOutput, 1024);
	
	// Print cwd to shell
	printf("%s\n",  pwdOutput);
}

void getCd(char input[]) {
	if (strlen(input) == 2) { // switch to user's home directory if no other arguments are provided.iowa state canvas
		chdir(getenv("HOME"));
	} else { // switch to provided directory otherwise
		// Obtain the substring directory
		char cdDirectory[strlen(input)-3];
		strcpy(cdDirectory, input+3);
		
		if (chdir(cdDirectory) == -1) {
			printf("Invalid Directory. Please enter a valid directory.\n");
		}
	}
}

void setEnvVars(char input[]) {
	// Separate user input by space
	char* setArgs = strtok(input, " ");
	
	// Declare variables to store user input
	int numArgs = 0;
	char* setVar;
	char* setValue;
	
	// Get the values of user input
	while (setArgs != NULL) {
		if (numArgs == 1) {
			setVar = setArgs;
		} else if (numArgs == 2) {
			setValue = setArgs;
		}
		
		setArgs = strtok(NULL, " ");
		numArgs++;
	}
	
	if (numArgs == 2) { // clear the specified environment variable
		if (setenv(setVar, NULL, 1) == -1) {
			printf("Set Environment Variable Failed");
		}
	} else if (numArgs == 3) { // set the environment variable
		if (setenv(setVar, setValue, 1) == -1) {
			printf("Set Environment Variable Failed");
		}
	} else {
		printf("Invalid Number of Arguments. 2 arguments required.\n");
	}
}

void getEnvCmd(char input[]) {
	char envGetInput[strlen(input)-4]; 
	strcpy(envGetInput, input+4); // obtain evn variable name

	char* envOutput = getenv(envGetInput);
	if (envOutput == NULL) {
		printf("Invalid Environment Variable\n");
	} else {
		printf("%s\n", envOutput);
	}
}


int nonBuiltIn(char input[]) {
	// Stores whether the command needs to be a background command
	int background = 0;
	
	// Declare variables to store user input
	int numArgs = 1;

	// Count the number of arguments
	int i;
	for (i = 0; i < strlen(input); i++) {
		if (input[i] == ' ') {
			numArgs++;
		}
	}
	
	// Check if user input is background process
	if (input[strlen(input)-1] == '&') {
		background = 1;
	}

	// Separate the arguments by spaces
	char* cmdArgs = strtok(input, " ");

	// Declare Variables to store user arguments
	char* inputArgs[numArgs+1];

	// Get the command and arguments from user
	i = 0;
	while (cmdArgs != NULL) {
		inputArgs[i] = cmdArgs;
		cmdArgs = strtok(NULL, " ");
	
		i++;
	}

	// Terminate list of arguments by NULL pointer
	if (background == 0) {
		inputArgs[numArgs] = (char*) NULL; // foreground process
	} else {
		inputArgs[numArgs-1] = (char*) NULL; // background process
	}
	
	// Create child process for command
	int forkValue = fork();

	// Check if we are in child process
	if (forkValue == 0) {
		childProcExecution(inputArgs);
	
		// Return from child process
		return 255;
	} else { // parent process
		usleep(500); // for correct output
		if (background == 0) { // not a background process
			checkForegroundProcess(forkValue, &inputArgs[0]);
		}
		// Check background process
		checkBackgroundProcesses();
	}
	
	return 0;
}

void checkForegroundProcess(int forkValue, char** inputArgs) {
	// Have parent process wait until child process completes
	int status;
	waitpid(forkValue, &status, 0);

	// Print exit information
	printf("[%d] %s, Exit %d\n", forkValue, inputArgs[0], WEXITSTATUS(status));
}

void checkBackgroundProcesses() {
	// Check for background processes finish
	int status;
	int waitID = 1;

	// Output all background processes that have finished execution
	while (waitID > 0) {
		waitID = waitpid(-1, &status, WNOHANG);

		if (waitID > 0) {
			// Print exit information
			printf("[%d] %s, Exit %d\n", waitID, "bgProc", WEXITSTATUS(status)); 
		}
	}
}

void childProcExecution(char** inputArgs) {
	// Print output
	printf("[%d] %s\n", getpid(), inputArgs[0]);

	// Execute specified command
	execvp(inputArgs[0], inputArgs);

	// Error if point is reach
	perror("The following error occurred");
}
