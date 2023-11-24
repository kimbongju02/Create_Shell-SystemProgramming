#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h> 
#include <string.h>

#include "../bin/function.h"
#define MAX_BUFFER_SIZE 1024
#define MAXARG 7

int main(int argc, char *argv[]) {

    char input[MAX_BUFFER_SIZE];
    struct passwd *user_info;
    user_info = getpwuid(getuid());
    int background = 0;
    int jobs_count = 0;
    
    // start redirection and pipe variable
    int num_args = 0, i=0, num_arg=0;
    char* arg[MAXARG];
    char* s;
    char* saveptr;
    int pid, status;
    
    int num = 0;
    int j = 0;
    int m = 0;
    int brpoint = 0;
    char *pipe_arg1[MAXARG];
    char *pipe_arg2[MAXARG];
    char input_file[20] = "";
    char output[20] = "";
    int is_bg = 0;
    static const char delim[] = " \t\n";
    //end redirection and pipe variable

    FILE *output_file = NULL; // 리다이렉션을 위한 파일 cat t1.txt > t2.txt 의 경우 t1.txt를 
    // 이 변수에 담는다.
    
    while (1) {
        char *argument[9] = {NULL};
        
        struct sigaction act_c;
        act_c.sa_handler = sigint_handler;
        sigfillset(&(act_c.sa_mask));
        sigaction(SIGINT, &act_c, NULL);

        struct sigaction act_z;
        act_z.sa_handler = sigquit_handler;
        sigfillset(&(act_z.sa_mask));
        sigaction(SIGQUIT, &act_z, NULL);        

        // get hostname
        char host_name[MAX_BUFFER_SIZE];
        if (gethostname(host_name, MAX_BUFFER_SIZE) != 0) {
            perror("Error getting host name");
            exit(EXIT_FAILURE);
        }

        char initial_cwd[MAX_BUFFER_SIZE];
        if (getcwd(initial_cwd, sizeof(initial_cwd)) == NULL) {
            perror("Error getting current working directory");
            exit(EXIT_FAILURE);
        }

        // show prompts
        printf("%s@%s:%s> ", user_info->pw_name, host_name, initial_cwd);
        fflush(stdout);

        // input user
        if (fgets(input, MAX_BUFFER_SIZE, stdin) == NULL) {
            perror("Error reading input");
            exit(EXIT_FAILURE);
        }

        // remove new line characters
        input[strcspn(input, "\n")] = '\0';

        // enter "exit" exit program
        if (strcmp(input, "exit") == 0) {
            printf("Exiting Hash shell.\n");
            break;
        }

        int count = parse(input, argument, &background);

        if(background==1){// background run
            int pid = fork();
            if (pid < 0) {
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                // child process
                int exec_result = execvp(argument[0], argument);
                if (exec_result == -1) {
                    exit(EXIT_FAILURE);
                }
            } else {
                // parent process
                printf("[%d]       %d\n", ++jobs_count, pid);
                background = 0;
            }
        }

        //---------------------------- Internal Implementation Commands
        else{

            if(argument[0] == NULL){
                continue;
            }
            // redirection and pipe
            if(strchr(input, '>') != NULL || strchr(input, '<') != NULL || strchr(input, '|') != NULL){
                num_args =0;
                if (strchr(input, ';') != NULL) {
                    num_args = tokenized(input, argument, ";");
                } else {
                    num_args = 1;
                    argument[0] = input;
                    argument[1] = (char *)0;
                }

                for (i = 0; i<num_args; i++) {
                num = 0;
                brpoint = 0;
                m = 0;
                is_bg = 0;
                s = strtok_r(argument[i], delim, &saveptr);
                
                    while(s) {
                        if (strcmp(s, "|") == 0) {
                            m = 1;
                            brpoint = num;
                            for (j =0; j < brpoint; j++) {
                                pipe_arg1[j] =  arg[j];
                            }
                            pipe_arg1[brpoint] = (char *) 0;
                        } else if (strcmp(s, "<") == 0) {
                            m = 2;
                            brpoint = num;
                        } else if (strcmp(s, ">") == 0) {
                            if (m == 2)
                                m = 4;
                            else
                                m = 3;
                        } else {
                            if(m < 2)
                                arg[num++] = s;
                            else if(m ==2)
                                strcpy(input_file, s);
                            else
                                strcpy(output, s);
                        }
                        
                        s = strtok_r(NULL, delim, &saveptr);
                        
                    }
                    arg[num] = (char *)0;
                    
                    if (m == 0) {
                        executesLine(is_bg, arg);
                    }
                    else if (m == 1) {
                        for (j =0; j < num-brpoint; j++) {
                            pipe_arg2[j]= arg[j+brpoint];
                        }
                        pipe_arg2[num-brpoint] = (char *) 0;
                        doPipe(pipe_arg1, pipe_arg2);
                    } else if(m >= 2) {
                        doRedirection(m, is_bg, arg, input_file, output);
                    }       
                }
            }

            //pwd
            else if (strcmp(argument[0], "pwd")==0){
                print_working_directory();
            }

            // cd
            else if (strcmp(argument[0], "cd")==0){
                change_directory(argument[1]);
            }

            // mkdir
            else if(strcmp(argument[0],"mkdir")==0){
                make_directory(argument[1]);
            }

            // rmdir
            else if(strcmp(argument[0],"rmdir")==0){
                remove_directory(argument[1]);
            }

            // cp
            else if(strcmp(argument[0], "cp")==0){
                copy_file(argument[1], argument[2]);
            }

            // rm
            else if(strcmp(argument[0], "rm")==0){
                remove_file(argument[1]);
            }

            else if (strcmp(argument[0], "ls") == 0) {
                list();
            }

            // cp
            else if(strcmp(argument[0], "mv")==0){
                move_file(argument[1], argument[2]);
            }

            //cat
            else if(strcmp(argument[0], "cat")==0){
                concatenate(argument[1], output_file);
            }

            else{
                printf("not command\n");
            }
        } 
    }
    return 0;
}
