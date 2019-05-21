#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// ------------------------------ Builtin Functions -----------------------------

// A function that changes the current working directory for the user to either what is specified or home
void getCd(char input[]);

// A function that prints the current working directory for the user
void getPwd();

// A function that gets the value of a specified environment variable
void getEnvCmd(char input[]);

// A function that sets the value of a specified environment variable or sets the value to null if no value is specified
void setEnvVars(char* setArgs);

// ------------------------------ Non-Builtin Functions -------------------------

// A function that parses through user input for non-builtin commands
int nonBuildIn(char input[]);

// A function that checks whether a foreground process has terminated and prints results if so
void checkForegroundProcess(int forkValue, char** inputArgs);

// A function that checks whether any background processes terminated and prints results if so
void checkBackgroundProcesses();

// A function that executes a child process for a given command
void childProcExecution(char** inputArgs);
