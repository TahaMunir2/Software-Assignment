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


void parse_command_subshell(char line[], char *args[], int *argsc, int *subshell_loc)
{
    //we initialize the subshell location
    *subshell_loc= -1;
    char *token = strtok(line, " ");

    *argsc = 0;
    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        if (token[0] == '"') {

            // Skip the opening quote
            char *start = token + 1;// points just after the "

            char *current = start;
            int len = strlen(current);

            // Keep appending tokens until the last char is a closing quotation mark: "
            while (len == 0 || current[len - 1] != '"') {
                char *next = strtok(NULL, " "); //at each iteration in the loop we "tokenize" and advance to the next word to append it to the whole string
                if (next == NULL) {
                    // No closing quotation mark found
                    break;
                }

                int next_len = strlen(next);

                // Add a space between tokens inside the string (so that it renders the real string)
                current[len] = ' ';

                len++; //incrementing the length for the extra space we just added

                //specific case of concatenation where memory overlaps (due to strtok), must use "memmove" from the C library (information from the man page of memmove)
                memmove(current + len, next, next_len + 1);
                len += next_len; //we increment the string length with the specific length of the word added by memmove
            }

            // If it ends with a quote, remove the trailing "
            if (current[len - 1] == '"') {
                current[len - 1] = '\0';
            }

            // Store the merged quoted argument (without the quotes)
            args[(*argsc)++] = start;
        }

 //handling parenthesis for subshell implementation       
 //handling parenthesis for subshell implementation       
        else if (token[0] == '(') {
            // we store the location of the subshell in the array of arguments
            *subshell_loc = *argsc;

            // We want everything inside the OUTERMOST (...) to be one argument
            char *start = token + 1;   // skip the first '('
            char *current = start;
            int len = strlen(current);

            // Parenthesis depth: we already saw one '('
            int depth = 1;

            // Count any '(' or ')' in the remainder of this first token (after the first char)
            for (int i = 0; start[i] != '\0'; i++) {
                if (start[i] == '(') {
                    depth++;
                } else if (start[i] == ')') {
                    depth--;
                }
            }

            // Keep appending tokens until we've closed the outermost '(' (depth == 0)
            while (depth > 0) {
                char *next = strtok(NULL, " ");
                if (next == NULL) {
                    // No matching closing parenthesis found
                    break;
                }

                int next_len = strlen(next);

                // Add a space between tokens inside the parentheses
                current[len] = ' ';
                len++;

                // Append next token (safe for overlap)
                memmove(current + len, next, next_len + 1);
                len += next_len;

                // Update depth based on this appended token
                for (int i = 0; next[i] != '\0'; i++) {
                    if (next[i] == '(') {
                        depth++;
                    } else if (next[i] == ')') {
                        depth--;
                    }
                }
            }

            // Trim any trailing spaces/tabs at the end
            while (len > 0 && (current[len - 1] == ' ' || current[len - 1] == '\t')) {
                current[--len] = '\0';
            }

            // Remove one trailing ')' that matches the outermost '('
            if (len > 0 && current[len - 1] == ')') {
                current[--len] = '\0';
            }

            // Store everything that was inside the parentheses as one argument
            args[(*argsc)++] = start;
        }

        // Normal token, unchanged
        else {

            args[(*argsc)++] = token;
        }
        // Get next token (space delimited) ( not included in the double quotation conditional)
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL; ///args must be null terminated
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
        if (token[0] != '"') {
            // Normal token, unchanged
            args[(*argsc)++] = token;
        } else {

            // Skip the opening quote
            char *start = token + 1;// points just after the "

            char *current = start;
            int len = strlen(current);

            // Keep appending tokens until the last char is a closing quotation mark: "
            while (len == 0 || current[len - 1] != '"') {
                char *next = strtok(NULL, " "); //at each iteration in the loop we "tokenize" and advance to the next word to append it to the whole string
                if (next == NULL) {
                    // No closing quotation mark found
                    break;
                }

                int next_len = strlen(next);

                // Add a space between tokens inside the string (so that it renders the real string)
                current[len] = ' ';

                len++; //incrementing the length for the extra space we just added

                //specific case of concatenation where memory overlaps (due to strtok), must use "memmove" from the C library (information from the man page of memmove)
                memmove(current + len, next, next_len + 1);
                len += next_len; //we increment the string length with the specific length of the word added by memmove
            }

            // If it ends with a quote, remove the trailing "
            if (current[len - 1] == '"') {
                current[len - 1] = '\0';
            }

            // Store the merged quoted argument (without the quotes)
            args[(*argsc)++] = start;
        }

        // Get next token (space delimited) ( not included in the double quotation conditional)
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL; ///args must be null terminated
}


void debug_print_tokens(char *args[], int argsc) {
    printf("argsc = %d\n", argsc);
    for (int i = 0; i < argsc; i++) {
        if (args[i] == NULL) {
            printf("args[%d] = NULL\n", i);
        } else {
            printf("args[%d] = '%s'\n", i, args[i]);
        }
    }
    if (args[argsc] == NULL) {
        printf("args[%d] (NULL terminator) = NULL\n", argsc);
    } else {
        printf("WARNING: args[%d] is not NULL (='%s')\n", argsc, args[argsc]);
    }
    printf("----\n");
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



//function that returns the index of the first occurence of a character inside a char buffer (equivalent to a string of characters)
int finding_index(char line[MAX_LINE], char character){
    for (int i =0; i< strlen(line); i++){
        if (line[i]== character){
            return i;
        }
    }
    return -1;// character not available
}


//function that checks if a specific string is inside double quotations
int inside_quot(char line[MAX_LINE], char string[]){
    if(strchr(line,'"')){
        //storing indexes:
        int opening = finding_index(line,'"' );
        char *p = line + opening + 1; //we start after the first quotation to detext the next quotation mark (which is the closing quotation mark)
        int closing = finding_index(p, '"') + opening + 1;
        //locating the string in the line using the strstr function
        char * string_starting_loc = strstr(line, string);
        //converting from pointer to integer so that we can operate the comparisons
        int integer_loc = string_starting_loc - line;
        if (integer_loc> opening && integer_loc< closing){
            return 1;
        }
    }
    return 0;
}


int command_with_redirection(char line[MAX_LINE]){


        if (is_subshell(line)==1){ //checking if we are in a subshell
            return 0;
        }


        if(strstr(line ,">>") != NULL){
            if (strstr(line,"\"")){ //safety check so that we don't call the inside_quot function without making sure that there are quotes in the line
                if (inside_quot(line,">>")){
                    return 0;
                }
                
            }
            return 1;
        }

        else if (strstr(line ,">") != NULL){
            if (strstr(line,"\"")){ //safety check so that we don't call the inside_quot function without making sure that there are quotes in the line
                if (inside_quot(line,">")){
                    return 0;
                }
            }
            return 1;
        }

        else if (strstr(line ,"<") != NULL){
            if (strstr(line,"\"")){ //safety check so that we don't call the inside_quot function without making sure that there are quotes in the line
                if (inside_quot(line,"<")){
                    return 0;
                }
            }
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
    //if we are in a subshell return immediately
    if (is_subshell(line)==1){
        return 0;
    }
    
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


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!REVIEW THE SECTION  !!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!REVIEW THE SECTION  !!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!REVIEW THE SECTION  !!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!REVIEW THE SECTION  !!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!REVIEW THE SECTION  !!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!REVIEW THE SECTION  !!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!REVIEW THE SECTION  !!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!REVIEW THE SECTION  !!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!REVIEW THE SECTION  !!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!REVIEW THE SECTION  !!!!!!!!!!!!!!!!!!!!!!!!!!!

//DO WE HAVE TO IMPLEMENT SUBSHELL BEHAVIOUR HERE?



//running the command
//no subshell can happen within cd 
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

    if (is_subshell(line)==1){ //checking if the line itself is a subshell
        return 0;
    }

    if((strstr(line ,"|") != NULL)){
        return 1;
    }       
    return 0;
}


void pipeline(char line[]) {
    char* req [MAX_ARGS];
    int count = 0;
    char*token = strtok(line, "|");

    while (token != NULL) {
        req[count++] = token;
        token = strtok(NULL, "|");
    }

    int fd[2];
    int input_old = STDIN_FILENO;

    for (int i = 0; i < count; i++) {
        char *args[MAX_ARGS];
        int argsc;
        int subshell_loc;
        parse_command_subshell(req[i], args, &argsc,&subshell_loc);

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

        if (subshell_loc==0){//the request itself is a subshell
            //args for subshell
            char *args_subshell[MAX_ARGS];

            //argsc for subshell
            int argsc_subshell;

            parse_command(args[subshell_loc], args_subshell, &argsc_subshell); // parses the line inside the brackets
            char lwd[MAX_PROMPT_LEN-6];
            init_lwd(lwd);
            subshsell_subroutine(args[subshell_loc], args_subshell, argsc_subshell, lwd, input_new, output_new); //execute the subshell


        }
        else{ //not a subshell
            launch_program(args, argsc, input_new, output_new);
        }

        if (i < count - 1) {
            input_old = fd[0];
        }
    }

    for(int i = 0; i < count; i++) {
        wait(NULL);
    }

}


int command_with_batching(char line[MAX_LINE]){
    if (is_subshell(line)==1){
        return 0;
    }
    if((strstr(line ,";") != NULL)){
        return 1;
    }       
    return 0;
}


void seperate_batchs(char line[MAX_LINE]) {
    char* req [MAX_ARGS];
    int count = 0;
    char*token = strtok(line, ";");

    while (token != NULL) {
        req[count++] = token;
        token = strtok(NULL, ";");
    }
    char lwd[MAX_PROMPT_LEN-6]; 
    init_lwd(lwd);
    for(int i = 0; i < count; i++) {
        char *args[MAX_ARGS];
        int argsc;
        line = req[i];
        if(command_with_pipelining(line)) {
            pipeline(line);
        }
        
        else if(is_cd(line)){
            parse_command(line, args, &argsc);
            run_cd(args, argsc, lwd); 
        }
        
        else if(command_with_redirection(line)){
           parse_command(line, args, &argsc);
           launch_program_with_redirection(args, argsc, 0, 1);
           reap();
       }
       else {
            if (is_subshell(line) == 1){
                //args for subshell
                char *args_subshell[MAX_ARGS];

                //argsc for subshell
                int argsc_subshell;

                //int subshell index in the argument array
                int subshell_loc;


                parse_command_subshell(line, args, &argsc, &subshell_loc ); //removes the brackets
                parse_command(args[subshell_loc], args_subshell, &argsc_subshell); // parses the line inside the brackets
                subshsell_subroutine(args[subshell_loc], args_subshell, argsc_subshell, lwd, 0, 1); //execute the subshell
                reap();
            }
            else{ //no subshells
                parse_command(line, args, &argsc);
                launch_program(args, argsc, 0, 1);
                reap();
            }
       }
    }
}


//checking if the specific argument is a subshell 
//IT DOES NOT REMOVE THE BRACKETS BECAUSE WE ARE USING A COPY OF THE STRING WHEN CALLING PARSE_COMMAND_SUBSHELL
int is_subshell(char argument[]){

    char tmp[MAX_LINE];
    strcpy(tmp, argument);

    //args for subshell
    char *args_subshell[MAX_ARGS];


    //argsc for subshell
    int argsc_subshell;

    //int subshell index in the argument array
    int subshell_loc;

    parse_command_subshell(tmp, args_subshell, &argsc_subshell, &subshell_loc );

    if (argsc_subshell == 1 && subshell_loc == 0){ //the whole argument is a subshell
        return 1;
    }
    return 0;
}

/*
// Returns 1 if the *whole* argument is a subshell "( ... )", possibly nested,
// and only spaces/tabs around it. Returns 0 otherwise.
int is_subshell(char argument[])
{
    char *p = argument;

    // 1. Skip leading spaces/tabs
    while (*p == ' ' || *p == '\t') {
        p++;
    }

    // 2. First non-space must be '('
    if (*p != '(') {
        return 0;
    }

    int depth = 0;
    char *q = p;
    char *outer_end = NULL;

    // 3. Scan to find where the outermost '(' closes
    for (; *q != '\0'; q++) {
        if (*q == '(') {
            depth++;
        } else if (*q == ')') {
            depth--;
            if (depth == 0) {
                outer_end = q;   // matching ')' for the first '('
                break;
            }
            if (depth < 0) {
                // More ')' than '('
                return 0;
            }
        }
    }

    // If we never closed the initial '(', or parentheses unbalanced
    if (depth != 0 || outer_end == NULL) {
        return 0;
    }

    // 4. After that matching ')', only spaces/tabs are allowed
    q = outer_end + 1;
    while (*q != '\0') {
        if (*q != ' ' && *q != '\t') {
            return 0;
        }
        q++;
    }

    // If we got here, the whole (trimmed) string is a subshell
    return 1;
}
*/

//pattern to repeat everytime we encounter a subshell
void subshsell_subroutine (char line[], char *args_subshell[], int argsc_subshell, char lwd[], int input, int output ){
    pid_t pid = fork();//creating a subshell
    if (pid == 0) {
    // RUN THE SUBSHELL (we are in the subshell process which is a child of the shell)

        // Connecting th subshell with the specified input and output

        if (input != STDIN_FILENO) {
            if (dup2(input, STDIN_FILENO) == -1) {
                perror("dup2 input in subshell");
                _exit(1);
            }
            close(input);
        }

        if (output != STDOUT_FILENO) {
            if (dup2(output, STDOUT_FILENO) == -1) {
                perror("dup2 output in subshell");
                _exit(1);
            }
            close(output);
        }


        if(command_with_batching(line)) {
            seperate_batchs(line);
        }

        else if(command_with_pipelining(line)) {
            pipeline(line);
        }
                
        else if(is_cd(line)){///Implement this function
            parse_command(line, args_subshell, &argsc_subshell);
            run_cd(args_subshell, argsc_subshell, lwd); 
        }
                
        else if(command_with_redirection(line)){
                redirection_subshell(line);
            }

       else {

//IMPLEMENTING NESTED SUBSHELLS
            if (is_subshell(line) == 1){ //the whole line is a subshell
                //args for subshell
                char *args_subshell_subshell[MAX_ARGS];

                //argsc for subshell
                int argsc_subshell_subshell;

                //int subshell index in the argument array
                int subshell_subshell_loc;


                parse_command_subshell(line, args_subshell, &argsc_subshell, &subshell_subshell_loc ); //removes the brackets
                subshsell_subroutine(args_subshell[subshell_subshell_loc], args_subshell_subshell, argsc_subshell_subshell, lwd, STDIN_FILENO, STDOUT_FILENO); //execute the subshell
                reap();
            }  

            else ///Basic command
                { //we don't parse again because we already parsed before calling the function
                int tmp;
                parse_command_subshell(line, args_subshell, &argsc_subshell, &tmp);
                launch_program(args_subshell, argsc_subshell, STDIN_FILENO, STDOUT_FILENO);
                reap();
                }      
       }
        _exit(0);


    } else if (pid > 0) {
        // We don't wait here, we let the caller (main shell) reap the child
    } else {
        perror("fork");
    }
  
}


//combining redirection and subshells in 1 function to call in the main file
void redirection_subshell(char line[]){
            char tmp[MAX_LINE];
            strcpy(tmp, line);

            char *parsed_args[MAX_ARGS];
            int parsed_argc;
            int subshell_loc;

            // we remove parentheses
            // we put the content of (...) into parsed_args[subshell_loc]

            parse_command_subshell(tmp, parsed_args, &parsed_argc, &subshell_loc);

            int is_outer_subshell_redir = 0;
            char *op = NULL;
            char *filename = NULL;

            if (subshell_loc == 0 && parsed_argc == 3) {
                // Candidate: (something) OP FILE
                if (strcmp(parsed_args[1], ">") == 0 ||
                    strcmp(parsed_args[1], ">>") == 0 ||
                    strcmp(parsed_args[1], "<") == 0) {

                    is_outer_subshell_redir = 1;
                    op       = parsed_args[1];
                    filename = parsed_args[2];
                }
            }

            if (is_outer_subshell_redir) { //handles the case where we have "(cmd) OP file" 

                int fd;
                if (strcmp(op, "<") == 0) {
                    fd = open(filename, O_RDONLY);
                } else if (strcmp(op, ">>") == 0) {
                    fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
                } else { 
                    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }

                if (fd == -1) {
                    perror("open failed");

                    return;

                } 

                else {
                    // Prepare inner subshell arguments
                    char *args_subshell[MAX_ARGS];
                    int argsc_subshell;

                    parse_command(parsed_args[0], args_subshell, &argsc_subshell);

                    char lwd[MAX_PROMPT_LEN - 6];
                    init_lwd(lwd);

                    if (strcmp(op, "<") == 0) {
                        // Redirect stdin of the subshell from file
                        subshsell_subroutine(parsed_args[0],
                                            args_subshell,
                                            argsc_subshell,
                                            lwd,
                                            fd,               // input from file
                                            STDOUT_FILENO);   // normal stdout
                    } else {
                        // Redirect stdout of the subshell to file
                        subshsell_subroutine(parsed_args[0],
                                            args_subshell,
                                            argsc_subshell,
                                            lwd,
                                            STDIN_FILENO,     // normal stdin
                                            fd);              // output to file
                    }
                    //we reap the subshell
                    reap();
                }
            } else {
                //case when we don't have a subshell
                char *args[MAX_ARGS];
                int argsc;
                parse_command(line, args, &argsc);
                launch_program_with_redirection(args, argsc,STDIN_FILENO,STDOUT_FILENO);                                               
                reap();
            }
}


//parsing commands with subshells debugger

void debug_parse_command_subshell_output(
    const char *label,
    char *args[],
    int argsc,
    int subshell_loc
) {
    fprintf(stderr, "\n====== DEBUG: %s ======\n", label);
    fprintf(stderr, "argsc       = %d\n", argsc);
    fprintf(stderr, "subshell_loc = %d\n", subshell_loc);

    for (int i = 0; i < argsc; i++) {
        if (args[i] == NULL)
            fprintf(stderr, "args[%d] = NULL\n", i);
        else
            fprintf(stderr, "args[%d] = '%s'\n", i, args[i]);
    }

    fprintf(stderr, "=============================\n\n");
}