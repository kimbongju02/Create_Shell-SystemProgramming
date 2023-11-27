
// function.c

#include "function.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>

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

    printf("File deletion successful: %s\n", file_path);
    return 0;
}

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
        return;
    }
}
// cat
void concatenate(const char *filename, FILE *output_file) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error opening file");
        return;
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

// start redirection and pipe
int tokenized(char buff[], char* arg[] ,char delim[]){
    char* saveptr;
    char* s;
    int num = 0;
    
    s = strtok_r(buff, delim, &saveptr);
    while(s) {
        arg[num++] = s;
        s = strtok_r(NULL, delim, &saveptr);
    }
    
    arg[num] = (char *)0;
    
    return num;
}
int checkInput(char buf[]){
    if(strcmp(buf, "")!=0 && strcmp(buf, "\t")!=0 && strcmp(buf, " ")!=0)
        return 0;
    return 1;
}
void executesLine(int isbg, char *argv[]){
    int status, pid;

    if ((pid=fork()) == -1)
        perror("fork failed");
    else if (pid != 0) {
        if(isbg==0)
             pid = wait(&status);
        else {
             printf("[1] %d\n", getpid());
             waitpid(pid, &status, WNOHANG);    
        }
    } else {
        execvp(argv[0], argv);
    }
}
void doPipe(char* arg1[], char* arg2[]){
    int fd[2];
    if(pipe(fd) == -1) {
        perror("Pipe failed");
        exit(1);
    }

    if(fork() == 0){            
        close(STDOUT_FILENO);  
        dup2(fd[1], 1);        
        close(fd[0]);    
        close(fd[1]);

        execvp(arg1[0], arg1);
        perror("parent execvp failed");
        exit(1);
    }

    if(fork() == 0) {           
        close(STDIN_FILENO); 
        dup2(fd[0], 0);      
        close(fd[1]);  
        close(fd[0]);

        execvp(arg2[0], arg2);
        perror("child execvp failed");
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);
    wait(0);
    wait(0);
}
void doRedirection(int flag, int is_bg, char *argv[], char* input, char * output){
    int input_fd, output_fd;
    int status, pid;
    
    if ((pid=fork()) == -1)
        perror("fork failed");
    else if (pid != 0) {
        if(is_bg==0)
            pid = wait(&status);
        else {
            printf("[1] %d\n", getpid());
            waitpid(pid, &status, WNOHANG);
        }
    } else {
        if (flag == 2) {
            if((input_fd = open(input, O_RDONLY))==-1){
                perror(argv[0]);
                exit(2);
            }
            dup2(input_fd, 0);
            close(input_fd);
            execvp(argv[0], argv);
        } else if (flag == 3) {
            output_fd = open(output, O_CREAT|O_TRUNC|O_WRONLY, 0600);
            dup2(output_fd, 1);
            close(output_fd);
            execvp(argv[0], argv);
        } else {
            if (input != NULL && output != NULL) {
                input_fd = open(input, O_RDONLY);
                dup2(input_fd, 0);
                close(input_fd);
                
                output_fd = open(output, O_CREAT|O_TRUNC|O_WRONLY, 0600);
                dup2(output_fd, 1);
                close(output_fd);
                execvp(argv[0], argv);
            }
        }
    }
}
// end redirection and pipe

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
