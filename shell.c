#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>   // Needed for file control options
#include <signal.h>  // Needed for signal handling

#define MAX_CMD_LENGTH 1024 // Maximum length of the command line
#define MAX_ARGS 64         // Maximum number of arguments for a command

// Forward declaration for our main execution function
int run_command_segment(char* segment);

/**
 * parse_args - Parses a command string into arguments.
 */
void parse_args(char *cmd, char **args, char **input_file, char **output_file) {
    int i = 0;
    *input_file = NULL;
    *output_file = NULL;

    char *token = strtok(cmd, " ");
    while (token != NULL && i < MAX_ARGS - 1) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) *input_file = token;
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) *output_file = token;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

/**
 * main - The main function for the shell.
 * This version uses a simple fgets loop, perfect for a non-interactive backend.
 */
int main() {
    char cmd_line[MAX_CMD_LENGTH];

    signal(SIGINT, SIG_IGN); // Parent shell ignores Ctrl+C

    // Set unbuffered output for stdout and stderr to ensure real-time updates
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // Print the initial prompt when the shell starts
    printf("simple_shell> ");

    while (fgets(cmd_line, sizeof(cmd_line), stdin) != NULL) {
        cmd_line[strcspn(cmd_line, "\n")] = 0;

        if (strlen(cmd_line) == 0) {
            printf("simple_shell> ");
            continue;
        }

        char* and_pos = strstr(cmd_line, "&&");
        if (and_pos != NULL) {
            *and_pos = '\0';
            if (run_command_segment(cmd_line) == 0) {
                run_command_segment(and_pos + 2);
            }
        } else {
            char* or_pos = strstr(cmd_line, "||");
            if (or_pos != NULL) {
                *or_pos = '\0';
                if (run_command_segment(cmd_line) != 0) {
                    run_command_segment(or_pos + 2);
                }
            } else {
                run_command_segment(cmd_line);
            }
        }
        
        // Print the prompt after each command execution
        printf("simple_shell> ");
    }

    printf("\nShell terminated.\n");
    return 0;
}

/**
 * run_command_segment - Executes a single command segment.
 */
int run_command_segment(char* segment) {
    int background = 0;
    char* ampersand = strrchr(segment, '&');
    if (ampersand != NULL) {
        background = 1;
        *ampersand = '\0';
    }

    char segment_copy_for_builtins[MAX_CMD_LENGTH];
    strcpy(segment_copy_for_builtins, segment);
    char* builtin_args[MAX_ARGS];
    char* temp_in, *temp_out;
    parse_args(segment_copy_for_builtins, builtin_args, &temp_in, &temp_out);

    if (builtin_args[0] == NULL) return 0;
    if (strcmp(builtin_args[0], "exit") == 0) exit(0);
    if (strcmp(builtin_args[0], "cd") == 0) {
        if (builtin_args[1] == NULL) {
            fprintf(stderr, "cd: expected argument\n");
            return 1;
        }
        if (chdir(builtin_args[1]) != 0) {
            perror("cd");
            return 1;
        }
        return 0;
    }

    char* pipe_pos = strchr(segment, '|');
    if (pipe_pos != NULL) {
        *pipe_pos = '\0';
        char* cmd1_str = segment;
        char* cmd2_str = pipe_pos + 1;
        char *args1[MAX_ARGS], *args2[MAX_ARGS];
        char *in1, *out1, *in2, *out2;
        parse_args(cmd1_str, args1, &in1, &out1);
        parse_args(cmd2_str, args2, &in2, &out2);
        int pipefd[2];
        if (pipe(pipefd) == -1) { perror("pipe"); return 1; }
        pid_t pid1, pid2;
        if ((pid1 = fork()) == 0) {
            signal(SIGINT, SIG_DFL);
            close(pipefd[0]); dup2(pipefd[1], STDOUT_FILENO); close(pipefd[1]);
            execvp(args1[0], args1);
            perror(args1[0]); exit(1);
        }
        if ((pid2 = fork()) == 0) {
            signal(SIGINT, SIG_DFL);
            close(pipefd[1]); dup2(pipefd[0], STDIN_FILENO); close(pipefd[0]);
            execvp(args2[0], args2);
            perror(args2[0]); exit(1);
        }
        close(pipefd[0]); close(pipefd[1]);
        int status1, status2;
        waitpid(pid1, &status1, 0); waitpid(pid2, &status2, 0);
        return WEXITSTATUS(status2);
    }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        char *args[MAX_ARGS];
        char *input_file, *output_file;
        char segment_copy[MAX_CMD_LENGTH];
        strcpy(segment_copy, segment);
        parse_args(segment_copy, args, &input_file, &output_file);
        if (input_file) {
            int fd = open(input_file, O_RDONLY);
            if (fd == -1) { perror("open"); exit(1); }
            dup2(fd, STDIN_FILENO); close(fd);
        }
        if (output_file) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) { perror("open"); exit(1); }
            dup2(fd, STDOUT_FILENO); close(fd);
        }
        execvp(args[0], args);
        perror(args[0]); exit(1);
    } else {
        if (!background) {
            int status;
            waitpid(pid, &status, 0);
            return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
        } else {
            printf("Started background process: [%d]\n", pid);
            return 0;
        }
    }
}