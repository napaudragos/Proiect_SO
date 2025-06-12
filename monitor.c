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
    while (fgets(args[count], sizeof(args[count]), f) && count < 3) { //se citeste maxim 3 linii
        args[count][strcspn(args[count], "\n")] = 0;
        count++;
    }
    fclose(f);
    unlink("prm.txt"); //sterge programul dupa ce a citit din el
    if (count == 0) return;

    char *exec_args[5] = {"./treasure_manager", NULL, NULL, NULL, NULL};
    for (int i = 0; i < count; i++) {
        exec_args[i + 1] = args[i];
    }

    pid_t pid = fork();
    if (pid == 0) {
        if (out_fd != -1) { //verifica daca out_fd este valid
            dup2(out_fd, STDOUT_FILENO); //redirectioneaza stdout catre out_fd , care va fi scris in fisierul treasure_hub
            dup2(out_fd, STDERR_FILENO); //redirectioneaza stderr catre out_fd , care va fi scris in fisierul treasure_hub
        }
        execvp(exec_args[0], exec_args); //execita manageerul cu argumentele date
        exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0); //blocheaza procesul parinte pana la terminarea procesului copil
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <write_fd>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    out_fd = atoi(argv[1]); //converteste argumentul din string in int pt a-l folosi ca file descriptor (ca sa stie unde sa scrie in hub)

    struct sigaction sa_usr1, sa_term;

    sa_usr1.sa_handler = handle_SIGUSR1; //spune sistemului ca pentru semnalul SIGUSR1 sa execute functia handle_SIGUSR1
    sigemptyset(&sa_usr1.sa_mask); //nu blocheaza alte semnale in timpul executiei handler-ului, masca e goala
    sa_usr1.sa_flags = 0; //seteaza optiuni speciale
    sigaction(SIGUSR1, &sa_usr1, NULL); //inregistreaza handler-ul pentru SIGUSR1

    sa_term.sa_handler = handle_SIGTERM; //spune sistemului ca pentru semnalul SIGTERM sa execute functia handle_SIGTERM
    sigemptyset(&sa_term.sa_mask); //nu blocheaza alte semnale in timpul executiei handler-ului, masca e goala
    sa_term.sa_flags = 0; //seteaza optiuni speciale
    sigaction(SIGTERM, &sa_term, NULL); //inregistreaza handler-ul pentru SIGTERM

    while (1) pause();
    return 0;
}