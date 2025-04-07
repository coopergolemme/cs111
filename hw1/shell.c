/**
 * @file shell.c
 * @brief This file contains the implementation of a simple shell program.
 *
 * This shell program is part of the coursework for CS111 at Tufts University.
 * It provides basic functionalities to execute commands and manage processes.
 *
 * @author Cooper Golemme
 * @date 2025-01-25
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define MAX_ARGS 256
#define INPUT_BUFFER_SIZE 256

int main()
{
    char input[INPUT_BUFFER_SIZE];

    while (true)
    {
        fprintf(stdout, "jsh$ ");

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("fgets error: EOF reached\n");
            return -1;
        }

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0)
        {
            break;
        }

        char *command_segment = strtok(input, "|");
        char *commands[MAX_ARGS];
        int num_commands = 0;

        // Gets commands and arguments. Commands are separated by the
        // "|" character.
        while (command_segment != NULL)
        {
            commands[num_commands++] = command_segment;
            command_segment = strtok(NULL, "|");
        }

        // Pipefds is array of file descriptors for pipes. The number of file
        // descriptors is 2 * (num_commands - 1) because each command has an
        // input and output pipe, except for the first and last commands.
        int pipefds[2 * (num_commands - 1)];

        // Create pipes for each command except the last one.
        for (int j = 0; j < num_commands - 1; j++)
        {
            if (pipe(pipefds + j * 2) < 0)
            {
                perror("pipe");
                exit(1);
            }
        }

        // Fork a child process for each command.
        for (int i = 0; i < num_commands; i++)
        {
            int rc = fork();
            if (rc < 0)
            {
                perror("fork");
                exit(1);
            }
            else if (rc == 0) // child process
            {
                // Redirect input for all commands except the first one.
                if (i > 0)
                {
                    if (dup2(pipefds[(i - 1) * 2], 0) < 0)
                    {
                        perror("dup2");
                        exit(1);
                    }
                }
                // Redirect output for all commands except the last one.
                if (i < num_commands - 1)
                {
                    if (dup2(pipefds[i * 2 + 1], 1) < 0)
                    {
                        perror("dup2");
                        exit(1);
                    }
                }
                // Close all pipe file descriptors
                for (int j = 0; j < 2 * (num_commands - 1); j++)
                {
                    close(pipefds[j]);
                }

                char *args[MAX_ARGS];
                char *args_tok = strtok(commands[i], " ");
                int k = 0;
                while (args_tok != NULL)
                {
                    args[k++] = args_tok;
                    args_tok = strtok(NULL, " ");
                }
                args[k] = NULL;
                execvp(args[0], args);

                // execvp only returns if there is an error
                fprintf(stdout, "jsh error: Command not found: %s\n", args[0]);

                // status 127 indicates command not found
                exit(127);
            }
        }

        // Close each of the file descriptors
        for (int j = 0; j < 2 * (num_commands - 1); j++)
        {
            close(pipefds[j]);
        }

        int status;
        int last_command_status = 0;

        // Wait for each command child process
        for (int i = 0; i < num_commands; i++)
        {
            wait(&status);
            // If the last command fails, update the last command status, else assume we assume it returned with status 0
            if (i == 0 && WEXITSTATUS(status) != 0)
            {
                last_command_status = WEXITSTATUS(status);
            }
        }

        // only print the status of the final command
        fprintf(stdout, "jsh status: %d\n", last_command_status);
    }

    return 0;
}