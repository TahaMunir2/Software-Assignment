#include "s3.h"

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory 
    char lwd[MAX_PROMPT_LEN-6]; 

    init_lwd(lwd);///Implement this function: initializes lwd with the cwd (using getcwd)

    //Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];

    //args for subshell
    char *args_subshell[MAX_ARGS];


    ///Stores the number of arguments
    int argsc;

    //argsc for subshell
    int argsc_subshell;

    //int subshell index in the argument array
    int subshell_loc;

    while (1) {

        read_command_line(line); ///Notice the additional parameter (required for prompt construction)

        if(command_with_batching(line)) {
            seperate_batchs(line);

        }

        else if(command_with_pipelining(line)) {
            pipeline(line);
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

        else if(is_cd(line)){///Implement this function
            parse_command_subshell(line, args, &argsc, &subshell_loc); //
            run_cd(args, argsc, lwd); ///Implement this function
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


        //Command with redirection
        else if (command_with_redirection(line)) {
            redirection_subshell(line);
        }

       else {

            if (is_subshell(line) == 1){ //the whole line is a subshell
                parse_command_subshell(line, args, &argsc, &subshell_loc ); //removes the brackets
                debug_parse_command_subshell_output("after parse_command_subshell", args, argsc, subshell_loc);
                subshsell_subroutine(args[subshell_loc], args_subshell, argsc_subshell, lwd, STDIN_FILENO, STDOUT_FILENO); //execute the subshell
                reap();
            }  

            else ///Basic command
                {
                parse_command(line, args, &argsc);
                launch_program(args, argsc, 0, 1);
                reap();
                }      
       }
    }

    return 0;
}