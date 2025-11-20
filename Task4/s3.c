#include "s3.h"

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{   
    char working_directory[MAX_PROMPT_LEN-6];
    strcpy(shell_prompt, getcwd(working_directory, MAX_PROMPT_LEN-6 ));
    strcat(shell_prompt, "[s3]$ ");
}

void init_lwd(char* working_directory){
    getcwd(working_directory, MAX_PROMPT_LEN-6 );
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
void child(char *args[], int argsc, int input, int output)
{
    char *operator = redirection_operator(args, argsc);
    if(redirection_operator(args, argsc) != NULL) {
        if (strcmp(operator, "<") == 0) {
            child_with_input_redirected(args, argsc, input, output);
        }
        else if (strcmp(operator, ">") == 0 || strcmp(operator, ">>") == 0) {
            child_with_output_redirected(args, argsc, input, output);
        }
    }

    else{ 
        if (input != STDIN_FILENO) {
            dup2(input, STDIN_FILENO);
            close(input);
        }

        if (output != STDOUT_FILENO) {
            dup2(output, STDOUT_FILENO);
            close(output);
        }

        execvp(args[ARG_PROGNAME], args);
    }  
}

void launch_program(char *args[], int argsc, int input, int output)
{
    if(strcmp(args[ARG_PROGNAME] ,"exit") == 0 ){
        exit(1);
    }
    int rc = fork();
    if (rc < 0) { // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) { // child (new process)
        child(args, argsc, input, output);
    } else { // parent goes down this path 
        
        if (input != STDIN_FILENO) {
            close(input);
           
        }

        if (output != STDOUT_FILENO) {
            close(output);
       
        }

        if (input == 0 && output == 1) {
            int wc = wait(NULL);
        }

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

        int fd;
        if(finding_file(args, argsc) != -1) {
            fd = open(args[finding_file(args, argsc)], flags, 0644); // third argument: in the case the file is created we set the permissions
        }
        else {
            fd = -1;
        }
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


void child_with_output_redirected(char *args[], int argsc, int input, int output){

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
    if (input != STDIN_FILENO) {
        dup2(input, STDIN_FILENO);
        close(input);
    }

    execvp(args[ARG_PROGNAME], clean_args(args, argsc));

}

 void child_with_input_redirected(char *args[], int argsc, int input, int output){
    int fd = -1;
    char* operator = redirection_operator(args, argsc);

    if (operator && !strcmp(operator,"<")){
        fd=opening_file(args, argsc, O_RDONLY);
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (output != STDOUT_FILENO){
        dup2(output, STDOUT_FILENO);
        close(output);
    }
    execvp(args[ARG_PROGNAME], clean_args(args, argsc));

}


void launch_program_with_redirection(char *args[], int argsc, int input, int output){
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
            child_with_output_redirected(args, argsc, input, output);
        }

        else if (!strcmp( operator, "<") ){
            child_with_input_redirected(args, argsc, input, output);
        }
    
        

    } else { // parent goes down this path 
        int wc = wait(NULL);
    }

}

//implementing change of directory



// Returns 1 if the command is a 'cd' command, 0 otherwise.
int is_cd(char line[]) {
    int i = 0;

    // 1. Skip leading spaces/tabs
    while (line[i] == ' ' || line[i] == '\t') {
        i++;
    }

    // 2. If line is empty or just whitespace → not cd
    if (line[i] == '\0' || line[i] == '\n') {
        return 0;
    }

    // 3. Check first two non-space chars are 'c' 'd'
    if (line[i] != 'c' || line[i+1] != 'd') {
        return 0;
    }

    // 4. Next char after "cd" must be:
    //    - end of string, or
    //    - whitespace (space/tab/newline)
    char next = line[i+2];
    if (next == '\0' || next == ' ' || next == '\t' || next == '\n') {
        return 1;   // it's a cd command
    }

    // e.g. "cdxyz" → not cd
    return 0;
}


//running the command
void run_cd(char *args[], int argsc, char *lwd){
    char tmp[MAX_PROMPT_LEN-6];
    getcwd(tmp, MAX_PROMPT_LEN-6 );
    if (argsc==1){
        chdir(getenv("HOME"));
    }
    else if (!strcmp(args[1], "-")) {
        printf("DEBUG: cd - requested, lwd = '%s'\n", lwd);

        if (chdir(lwd) == -1) {
            perror("DEBUG: chdir(lwd) failed");
        } else {
            printf("DEBUG: chdir(lwd) succeeded, now in '%s'\n", lwd);
        }
    }
    else if (args[1][0]== '~'){
        chdir(getenv("HOME"));
        if (args[1][1]== '/'){
            chdir(args[1] +2);
        }
    }
    else{
        chdir(args[1]);
    }

    strncpy(lwd, tmp, MAX_PROMPT_LEN-6 - 1);
    lwd[MAX_PROMPT_LEN-6-1 ] = '\0';
}

int command_with_pipelining(char line[MAX_LINE]){
        if((strstr(line ,"|") != NULL)){
            return 1;
        }       
    return 0;
}

void pipeline(char line[]) {
    char* req [MAX_ARGS];
    int count = 0;
    char*token = strtok(line, "|");

    while (token != NULL && count < MAX_ARGS) {
        req[count++] = token;
        token = strtok(NULL, "|");
    }

    int fd[2];
    int input_old = STDIN_FILENO;

    for (int i = 0; i < count; i++) {
        char *args[MAX_ARGS];
        int argsc;
        parse_command(req[i], args, &argsc);

        int input_new = input_old;
        int output_new;

        if(i < count) {
            if (i != count - 1) {
                pipe(fd);
                output_new = fd[1];
            }
            else {
                output_new = STDOUT_FILENO;
            }
        }
        
        launch_program(args, argsc, input_new, output_new);

        if (i < count - 1) {
            input_old = fd[0];
        }

        
    }
    for(int i = 0; i < count; i++) {
        wait(NULL);
    }

}