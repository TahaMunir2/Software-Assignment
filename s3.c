#include "s3.h"

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, "[s3]$ ");
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    ///See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL)
    {
        perror("fgets failed");
        exit(1);
    }
    ///Remove newline (enter)
    line[strlen(line) - 1] = '\0';
}

void parse_command(char line[], char *args[], int *argsc)
{
    ///Implements simple tokenization (space delimited)
    ///Note: strtok puts '\0' (null) characters within the existing storage, 
    ///to split it into logical cstrings.
    ///There is no dynamic allocation.

    ///See the man page of strtok(...)
    char *token = strtok(line, " ");
    *argsc = 0;
    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        args[(*argsc)++] = token;
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL; ///args must be null terminated
}

///Launch related functions
void child(char *args[], int argsc)
{
    execvp(args[ARG_PROGNAME], args);
    
}

void launch_program(char *args[], int argsc)
{
    if(strcmp(args[ARG_PROGNAME] ,"exit") == 0 ){
        exit(1);
    }
    
    int rc = fork();
    if (rc < 0) { // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) { // child (new process)
        child(args, argsc);
    } else { // parent goes down this path 
        int wc = wait(NULL);
    }
}


int command_with_redirection(char line[MAX_LINE]){
        if((strstr(line ,">>") != NULL) || (strstr(line ,">") != NULL) || (strstr(line ,"<") != NULL)){
            return 1;
        }       
    return 0;
}


int finding_file(char *args[], int argsc){
    for (int i=0; i< argsc; i++){
        if((strcmp(args[i] , ">>") == 0) || (strcmp(args[i] , ">") == 0) || (strcmp(args[i] , "<") == 0)){
            return i+1;
        }
    }
    return -1;
}


char* redirection_operator(char *args[], int argsc){
    for (int i=0; i< argsc; i++){
        if((strcmp(args[i] , ">>") == 0) ){
            return ">>";
        }
        else if ((strcmp(args[i] , ">") == 0)){
            return ">";
        }
        else if((strcmp(args[i] , "<") == 0)){
            return "<";
        }
    }
    return NULL;
}


int opening_file(char *args[], int argsc, int flags){

        int fd = open(args[finding_file(args, argsc)], flags, 0644); // third argument: in the case the file is created we set the permissions
        
        //error check
        if (fd == -1){
            perror("open failed");
            exit(1);
        }

        return fd;
}

char ** clean_args(char *args[], int argsc){

    char **new_args = malloc(sizeof(char*) * (argsc - 1)); // the number of pointers we need is the total number of arguments +1(for the null pointer) and -2 (for the 2 arguments we are removing)
    int k=0;

    for (int i=0; i< argsc; i++){
        if((i != finding_file(args, argsc)) && (i != finding_file(args, argsc) -1)){
            new_args[k++]= args[i];
        }
    }
    new_args[k]=NULL;
    return new_args;
}


void child_with_output_redirected(char *args[], int argsc){

    int fd = -1;
    char* operator = redirection_operator(args, argsc);

    if (operator && !strcmp(operator,">>")){
        fd=opening_file(args, argsc, O_WRONLY| O_CREAT| O_APPEND);
    }
    else if (operator &&!strcmp (operator, ">") ){
        fd=opening_file(args, argsc, O_WRONLY| O_CREAT| O_TRUNC);
    }
    if (fd != -1){
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    execvp(args[ARG_PROGNAME], clean_args(args, argsc));
}

 void child_with_input_redirected(char *args[], int argsc){
    int fd = -1;
    char* operator = redirection_operator(args, argsc);

    if (operator && !strcmp(operator,"<")){
        fd=opening_file(args, argsc, O_RDONLY);
    }
    if (fd != -1){
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    clean_args(args, argsc);
    execvp(args[ARG_PROGNAME], clean_args(args, argsc));
}


void launch_program_with_redirection(char *args[], int argsc){
    if(strcmp(args[ARG_PROGNAME] ,"exit") == 0 ){
        exit(1);
    }
    
    int rc = fork();
    if (rc < 0) { // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    } 

    else if (rc == 0) { // child (new process)

        char *operator = redirection_operator(args, argsc);

        if ((!strcmp(operator,">>")) || (!strcmp (operator, ">") ) ){
            child_with_output_redirected(args, argsc);
        }

        else if (!strcmp( operator, "<") ){
            child_with_input_redirected(args, argsc);
        }

    } else { // parent goes down this path 
        int wc = wait(NULL);
    }
}
