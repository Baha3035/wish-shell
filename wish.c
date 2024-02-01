#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

int main(int argc, char* argv[]){
    char PATH[] = "/bin/";
    // strcat(PATH, "/");
    char buffer[BUFFER_SIZE];

    printf("wish> ");

    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL)
    {
        // Remove newline character from the end of the input
        buffer[strcspn(buffer, "\n")] = '\0';

        printf("buffer %s\n", buffer);

        char* token;
        char* args[32];
        int i = 0;
        // strtok() returns a pointer
        token = strtok(buffer, " ");
        while (token != NULL && i < 32) {
            printf("token %s\n", token);
            args[i++] = token;
            token = strtok(NULL, " ");
        }

        args[i] = NULL; // Null-terminate the arguments list

        pid_t pid = fork();

        if (pid < 0){
            // Fork failed
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0){
            // Child process
            char bin_command_path[BUFFER_SIZE];
            char user_bin_command_path[BUFFER_SIZE];
            snprintf(bin_command_path, BUFFER_SIZE, "%s%s", PATH, args[0]);
            // printf("Bin command path %s\n",bin_command_path);
            snprintf(user_bin_command_path, BUFFER_SIZE, "%s%s", PATH, args[0]);
            if (access(bin_command_path, X_OK) == 0){
                execv(bin_command_path, args);
                // If execv returns it must've failed
                perror("execv");
                exit(EXIT_FAILURE);
            } else if (access(user_bin_command_path, X_OK) == 0){
                execv(user_bin_command_path, args);
                // If execv returns it must've failed
                perror("execv");
                exit(EXIT_FAILURE);
            } else {
                perror("command not found");
                exit(EXIT_FAILURE);
            }
        } else {
            // Parent process
            int status;
            waitpid(pid, &status, 0);
            printf("wish> ");
        }

        // char* arr[] = {buffer, "-l", NULL};
        // execv(PATH, arr);
        /* code */
    }
    
    return 0;
}