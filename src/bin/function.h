// function.h

#ifndef FUNCTION_H
#define FUNCTION_H

#include <stdio.h>

void sigint_handler(int signo);
void sigquit_handler(int signo);
int change_directory(const char *dirname);
int make_directory(const char *dirname);
int remove_directory(const char *dirname);
int remove_file(const char *file_path);
int move_file(const char *source_path, const char *destination_path);
int list();
void print_working_directory();
void concatenate(const char *filename, FILE *output_file);
void copy_file(const char *source_path, const char *destination_path);
int tokenized(char buff[], char* arg[], char delim[]);
int checkInput(char buf[]);
void executesLine(int is_bg, char *argv[]);
void doPipe(char* arg1[], char* arg2[]);
void doRedirection(int flag, int is_bg, char *argv[], char* input, char * output);
int parse(const char *command, char **arguments, int *background);

#endif
