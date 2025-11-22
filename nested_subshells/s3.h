#ifndef _S3_H_
#define _S3_H_

///See reference for what these libraries provide
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <string.h> 

///Constants for array sizes, defined for clarity and code readability
#define MAX_LINE 1024
#define MAX_ARGS 128
#define MAX_PROMPT_LEN 256

///Enum for readable argument indices (use where required)
enum ArgIndex
{
    ARG_PROGNAME,
    ARG_1,
    ARG_2,
    ARG_3,
};

///With inline functions, the compiler replaces the function call 
///with the actual function code;
///inline improves speed and readability; meant for short functions (a few lines).
///the static here avoids linker errors from multiple definitions (needed with inline).
static inline void reap()
{
    wait(NULL);
}

///Shell I/O and related functions (add more as appropriate)
void init_lwd(char* working_directory);
void read_command_line(char line[]);
void construct_shell_prompt(char shell_prompt[]);
void parse_command(char line[], char *args[], int *argsc);

///Child functions (add more as appropriate)
void child(char *args[], int argsc, int input, int output);

///Program launching functions (add more as appropriate)
void launch_program(char *args[], int argsc, int input, int output);


//implementing redirection
int command_with_redirection(char line[]);
void launch_program_with_redirection(char *args[], int argsc , int input, int output);
void child_with_input_redirected(char *args[], int argsc, int input, int output);
void child_with_output_redirected(char *args[], int argsc, int input, int output);
char* redirection_operator(char *args[], int argsc);

//implementing redirection with subshells
void redirection_subshell(char line[]);


//implementing change of directory
int is_cd(char* line);
void run_cd(char *args[], int argsc, char *lwd);


//implementing pipelining
int command_with_pipelining(char line[MAX_LINE]);
void pipeline(char line[]); 


//implementing batched commands
void seperate_batchs(char line[MAX_LINE]);
int command_with_batching(char line[MAX_LINE]);


//subshell implementation
int is_subshell(char line[]);
void parse_command_subshell(char line[], char *args[], int *argsc, int *subshell_loc);

//debugging
void debug_print_tokens(char *args[], int argsc);
void debug_parse_command_subshell_output(
    const char *label,
    char *args[],
    int argsc,
    int subshell_loc
);

//subshell subroutine
void subshsell_subroutine (char line[], char *args_subshell[], int argsc_subshell, char lwd[], int input, int output );
#endif
