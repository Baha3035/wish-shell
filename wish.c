#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> // For open() and O_WRONLY

#define BUFFER_SIZE 4096
#define MAX_PATHS 10
#define MAX_PATH_LENGTH 256
#define MAX_ARGS 32

void execute_command(char *args[], char *PATH[], int outputRedirect, char *filename, char *error_message) {
    pid_t pid = fork();

    if (pid < 0) {
        // Fork failed
        write(STDERR_FILENO, error_message, strlen(error_message));
    } else if (pid == 0) {
        // Child process
        int path_success = 0;
        for (int j = 0; j < MAX_PATHS && PATH[j] != NULL; j++) {
            char command_path[MAX_PATH_LENGTH];
            snprintf(command_path, MAX_PATH_LENGTH, "%s%s", PATH[j], args[0]);
            if (access(command_path, X_OK) == 0) {
                path_success = 1;

                if (outputRedirect) {
                    if (filename != NULL) {
                        // Open the file for writing, truncate it if it exists or create it if it doesn't
                        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd < 0) {
                            // Failed to open file
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            exit(0);
                        }

                        // Redirect stdout and stderr to the file
                        dup2(fd, STDOUT_FILENO);
                        dup2(fd, STDERR_FILENO);

                        // Close the file descriptor as it's no longer needed
                        close(fd);
                    } else {
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        exit(0);
                    }
                }

                execv(command_path, args);
                // If execv returns, it must've failed
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
        }
        if (!path_success) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(0);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}

void handle_cd(char *args[], char *error_message) {
    if (args[1] == NULL) {
        write(STDERR_FILENO, error_message, strlen(error_message));
    } else {
        if (chdir(args[1]) != 0) {
            // chdir failed
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    }
}

void handle_path(char *args[], char *PATH[], char *error_message) {
    // Clear existing PATH
    memset(PATH, 0, MAX_PATHS * sizeof(char *));
    // Set new PATH
    for (int j = 1; args[j] != NULL && j < MAX_PATHS; j++) {
        // Append '/' to the end of the directory path
        size_t len = strlen(args[j]);
        if (len > 0 && args[j][len - 1] != '/') {
            strcat(args[j], "/");
        }

        char *absolute_path = strcat(realpath(args[j], NULL), "/");
        if (absolute_path != NULL) {
            PATH[j - 1] = absolute_path;
        } else {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    }
}

int main(int argc, char *argv[]) {
    char error_message[] = "An error has occurred\n";
    char *PATH[MAX_PATHS] = {"/bin/"};
    char buffer[BUFFER_SIZE];

    if (argc > 2) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 1;
    }

    FILE *fp = (argc == 2) ? fopen(argv[1], "r") : stdin;

    if (fp == NULL) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return 1;
    }

    while (1) {
        if (argc == 1) {
            printf("wish> ");
        }

        if (fgets(buffer, BUFFER_SIZE, (argc == 2) ? fp : stdin) == NULL) {
            break;
        }

        if (strcmp(buffer, "\n") == 0) {
            continue;
        }

        // Remove newline character from the end of the input
        buffer[strcspn(buffer, "\n")] = '\0';

        char *token;
        char *args[MAX_ARGS] = {NULL}; // Initialize args with NULL pointers
        int arg_count = 0;

        // strtok() returns a pointer
        token = strtok(buffer, " \t\n");
        while (token != NULL && arg_count < MAX_ARGS - 1) {
            args[arg_count++] = token;
            token = strtok(NULL, " \t\n");
        }

        // Check for redirection
        int multipleFiles = 0;
        int outputRedirect = 0;
        char *filename;
        for (int j = 0; args[j] != NULL; j++)
        {
            if ((strcmp(args[j], ">") == 0) && (strcmp(args[0], ">") != 0))
            {
                if (args[j + 2] != NULL)
                {
                    multipleFiles = 1;
                }
                else
                {
                    outputRedirect = 1;
                    filename = args[j + 1];
                    args[j] = NULL;
                }
            }
            else
            {
                char *pos = strchr(args[j], '>');

                if (pos)
                {
                    if (args[j + 1] != NULL)
                    {
                        multipleFiles = 1;
                    }
                    else
                    {
                        char *arg = strtok(args[j], ">");
                        args[j] = arg;
                        outputRedirect = 1;
                        arg = strtok(NULL, ">");
                        filename = arg;
                    }
                }
            }
        }

        if (multipleFiles) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(0);
        }

        if (strcmp(args[0], "exit") == 0) {
            if (arg_count > 1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
            } else {
                exit(0);
            }
            break;
        } else if (strcmp(args[0], "cd") == 0) {
            handle_cd(args, error_message);
        } else if (strcmp(args[0], "path") == 0) {
            handle_path(args, PATH, error_message);
        } else {
            execute_command(args, PATH, outputRedirect, filename, error_message);
        }
    }

    if (argc == 2) {
        fclose(fp);
    }

    return 0;
}
