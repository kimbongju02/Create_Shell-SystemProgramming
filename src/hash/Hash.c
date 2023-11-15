#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>

#include <fcntl.h>
#include <signal.h>

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
    if (unlink(file_path) != 0) {
        perror("Error removing file");
        return -1;
    }
    return 0;
}

int move_file(const char *source_path, const char *destination_path){
    if (rename(source_path, destination_path) != 0) {
        perror("Error moving file");
        return -1;
    }
    return 0;
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

int function(const char *argument){

}

int main() {

    char input[MAX_BUFFER_SIZE];
    struct passwd *user_info;
    user_info = getpwuid(getuid());
    int background = 0;
    int jobs_count = 0;
    char *argument[9];

    while (1) {
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

            function(argument[0]);
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
            }

            // cp
            if(strcmp(argument[0], "mv")==0){
                move_file(argument[1], argument[2]);
                continue;
            }
        } 
    }

    return 0;
}