#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
int out_fd = -1;

void handle_SIGTERM(int sig) {
    dprintf(out_fd, "Monitor shutting down...\n");
    exit(0);
}

void handle_SIGUSR1(int sig) {
    FILE *f = fopen("prm.txt", "r");
    if (!f) return;
    char args[3][100];
    int count = 0;
    while (fgets(args[count], sizeof(args[count]), f) && count < 3) {
        args[count][strcspn(args[count], "\n")] = 0;
        count++;
    }
    fclose(f);
    unlink("prm.txt");
    if (count == 0) return;

    char *exec_args[5] = {"./treasure_manager", NULL, NULL, NULL, NULL};
    for (int i = 0; i < count; i++) {
        exec_args[i + 1] = args[i];
    }

    pid_t pid = fork();
    if (pid == 0) {
        if (out_fd != -1) {
            dup2(out_fd, STDOUT_FILENO);
            dup2(out_fd, STDERR_FILENO);
        }
        execvp(exec_args[0], exec_args);
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <write_fd>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    out_fd = atoi(argv[1]);
    struct sigaction sa_usr1, sa_term;
    sa_usr1.sa_handler = handle_SIGUSR1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;
    sigaction(SIGUSR1, &sa_usr1, NULL);

    sa_term.sa_handler = handle_SIGTERM;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0;
    sigaction(SIGTERM, &sa_term, NULL);

    while (1) pause();
    return 0;
}