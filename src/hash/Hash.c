#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>

#define MAX_BUFFER_SIZE 1024

int change_directory(const char *dirname) {
    if (chdir(dirname) != 0) {
        perror("Error changing directory");
        return -1;
    }
    return 0;
}

int parse(const char *command, char **arguments) {
    char *token;
    int count = 0;
    char command_copy[strlen(command) + 1];
    strcpy(command_copy, command);
    token = strtok(command_copy, " ");
    while (token != NULL) {
        arguments[count] = (char *)malloc(strlen(token) + 1);
        strcpy(arguments[count], token);
        count++;
        token = strtok(NULL, " ");
    }

    return count;
}

int main() {

    char input[MAX_BUFFER_SIZE];
    struct passwd *user_info;
    user_info = getpwuid(getuid());
    
    char *argument[9];

    while (1) {
        //호스트 이름 얻기
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
        // 프롬프트 표시

        printf("%s@%s:%s> ", user_info->pw_name, host_name, initial_cwd);
        fflush(stdout);

        // 사용자 입력 받기
        if (fgets(input, MAX_BUFFER_SIZE, stdin) == NULL) {
            perror("Error reading input");
            exit(EXIT_FAILURE);

        }

        // 새 줄 문자 제거
        input[strcspn(input, "\n")] = '\0';

        // "exit" 입력 시 프로그램 종료
        if (strcmp(input, "exit") == 0) {
            printf("Exiting Hash shell.\n");
            break;
        }

        //---------------------------- 명령어

        
        int count = parse(input,argument);
        
        if (strcmp(argument[0], "cd")==0){
            change_directory(argument[1]);
            continue;
        }

        //----------------------------- 명령어 

        // 입력된 명령어 실행
        if (strlen(input) > 0) {
            int pid = fork();

            if (pid == -1) {

                perror("Error creating child process");
                exit(EXIT_FAILURE);

            } else if (pid == 0) {

                // 자식 프로세스에서 명령어 실행
                execlp(input, input, (char *)NULL);

                // execlp가 실패할 경우 실행
                perror("Error executing command");
                exit(EXIT_FAILURE);

            } else {

                // 부모 프로세스는 자식 프로세스의 종료를 기다림
                wait(NULL);
                
            }
        }
    }

    return 0;
}
