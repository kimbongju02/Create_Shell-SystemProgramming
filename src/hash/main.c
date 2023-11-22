#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>

#include <fcntl.h>
#include <signal.h> 
#include <string.h>

#define MAX_BUFFER_SIZE 1024


int change_directory(const char *dirname) {
    if (chdir(dirname) != 0) {
        perror("Error changing directory");
        return -1;
    }
    return 0;
}

int make_directory(const char *dirname){
    if(mkdir(dirname, 0777)!=0){
        perror("Eror create directory");
        return -1;
    }
    return 0;
}

int remove_directory(const char *dirname) {
    if (rmdir(dirname) != 0) {
        perror("Error removing directory");
        return -1;
    }
    return 0;
}

int remove_file(const char *file_path) {
    
}
/*
if (unlink(file_path) != 0) {
        perror("Error removing file");
        return -1;
    }
return 0;

*/

/*
                char *path = argument[1];
                struct stat file_info;

                if (stat(path, &file_info) == 0) {
                    // file
                    if (S_ISREG(file_info.st_mode)) {
                         if (remove_file(path) == 0) {
                            printf("file deletion successful: %s\n", path);
                        }
                    // directory
                    } else if (S_ISDIR(file_info.st_mode)) {
                        if (remove_directory(path) == 0) {
                            printf("directory deletion successful: %s\n", path);
                        }
                    // not file and directory
                    } else {
                        printf("%s not file and directory.\n", path);
                    }
                } else {
                    perror("read file error");
                }
*/

int move_file(const char *source_path, const char *destination_path){
    if (rename(source_path, destination_path) != 0) {
        perror("Error moving file");
        return -1;
    }
    return 0;
}

int list(){
    DIR *dir;
    struct dirent *entry;

    dir = opendir(".");
    if (dir == NULL) {
        perror("Fail open Directory");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return 0;
}

int file_redirection(char* command){
    int result = system(command);

    if (result == -1) {
        perror("File Redirection Fail");
        return -1;
    }

    return 0;
}

void sigint_handler(int signo){
    printf("\nCtrl-C push!\n");
}

void sigquit_handler(int signo) {
    printf("\nCtrl-z push\n");
}

// pwd
void print_working_directory(){
    char *pwd;
    pwd = getcwd(NULL,0);
    if(pwd != NULL){
        printf("%s\n",pwd);
        free(pwd);
    }
    else{
        perror("getcwd");
        exit(EXIT_FAILURE);
    }
}
// cat
void concatenate(const char *filename, FILE *output_file) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int ch;
    while ((ch = fgetc(file)) != EOF) {
        putchar(ch);

        // 리다이렉션 파일이 열려있는 경우 해당 파일에도 쓰기
        if (output_file != NULL) {
            fputc(ch, output_file);
        }
    }

    fclose(file);
}
void copy_file(const char *source_path, const char *destination_path) {
    FILE *source_file, *destination_file;
    char buffer[1024];
    size_t bytes_read, bytes_written;

    // open source file
    source_file = fopen(source_path, "rb");
    if (source_file == NULL) {
        perror("Error opening source file");
        return;
    }

    // check destination path is directory
    struct stat dest_stat;
    if (stat(destination_path, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode)) {
        // destination path is directory
        // create a new file path add source file name
        char dest_filename[1024];
        snprintf(dest_filename, sizeof(dest_filename), "%s/%s", destination_path, basename((char *)source_path));
        destination_path = dest_filename;
    }

    // open destination file 
    destination_file = fopen(destination_path, "wb");
    if (destination_file == NULL) {
        perror("Error opening destination file");
        fclose(source_file);
        return;
    }

    // copy file
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
        bytes_written = fwrite(buffer, 1, bytes_read, destination_file);
        if (bytes_written != bytes_read) {
            perror("Error writing to destination file");
            fclose(source_file);
            fclose(destination_file);
            return;
        }
    }

    fclose(source_file);
    fclose(destination_file);
}

int parse(const char *command, char **arguments,int *background) {
    char *token;
    int count = 0;
    char command_copy[strlen(command) + 1];
    strcpy(command_copy, command);
    token = strtok(command_copy, " ");

    
    while (token != NULL) {
        arguments[count] = (char *)malloc(strlen(token) + 1);
        strcpy(arguments[count], token);
        if(strcmp(token,"&")==0){
            *background = 1;
        }
        count++;
        token = strtok(NULL, " ");
    }

    return count;
}

int main(int argc, char *argv[]) {

    char input[MAX_BUFFER_SIZE];
    struct passwd *user_info;
    user_info = getpwuid(getuid());
    int background = 0;
    int jobs_count = 0;

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

        int count = parse(input,argument, &background);

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
            //pwd
            if (strcmp(argument[0], "pwd")==0){
                print_working_directory();
                continue;
            }

            // cd
            if (strcmp(argument[0], "cd")==0){
                change_directory(argument[1]);
                continue;
            }

            // mkdir
            if(strcmp(argument[0],"mkdir")==0){
                make_directory(argument[1]);
                continue;
            }

            // rmdir
            if(strcmp(argument[0],"rmdir")==0){
                remove_directory(argument[1]);
                continue;
            }

            // cp
            if(strcmp(argument[0], "cp")==0){
                copy_file(argument[1], argument[2]);
                continue;
            }

            // rm
            if(strcmp(argument[0], "rm")==0){
                remove_file(argument[1]);
                continue;
            }

            if (strcmp(argument[0], "ls") == 0) {
                list();
                continue;
            }

            // cp
            if(strcmp(argument[0], "mv")==0){
                move_file(argument[1], argument[2]);
                continue;
            }

            //cat
            if(strcmp(argument[0], "cat")==0){
                concatenate(argument[1], output_file);
                continue;
            }

            // doogunwo version redirection >
            const char *input_filename = argv[1];
            const char *output_filename = (argc == 3) ? argv[2] : NULL;
            if(output_file != NULL){
                output_file = fopen(output_filename,"w");
                if(output_file == NULL){
                    perror("Error opening output file");
                    exit(EXIT_FAILURE);
                }
            }
            if (output_file != NULL) {
                fclose(output_file);
            }

            /**/
            // file redirection >
            if (argument[1]!= NULL && strcmp(argument[1], ">") == 0) {
                char command[100];
                strcpy(command, argument[0]);
                
                if (argument[2] != NULL) {
                    strcat(command, " ");
                    strcat(command, argument[1]);
                    strcat(command, " ");
                    strcat(command, argument[2]);
                }
                else{
                    perror("use 'a > b'");
                }
                file_redirection(command);
                continue;
            }

           
        
        } 
    }

    return 0;
}
