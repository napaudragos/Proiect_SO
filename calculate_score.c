#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USERNAME_MAX 32


typedef struct {
    int treasure_id;
    char username[USERNAME_MAX];
    float latitude;
    float longitude;
    char clue[256];
    int value;
} Treasure;

typedef struct UserScore {
    char username[USERNAME_MAX];
    int score;
    struct UserScore *next;
} UserScore;

UserScore* find_or_add(UserScore **head, const char *username) {
    UserScore *curr = *head;
    while (curr) {
        if (strcmp(curr->username, username) == 0)
            return curr;
        curr = curr->next;
    }
    UserScore *new_node = malloc(sizeof(UserScore));
    strcpy(new_node->username, username);
    new_node->score = 0;
    new_node->next = *head;
    *head = new_node;
    return new_node;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s treasures.dat\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    UserScore *scores = NULL;
    Treasure t;
    while (fread(&t, sizeof(Treasure), 1, f) == 1) {
        UserScore *us = find_or_add(&scores, t.username);
        us->score += t.value;
    }
    fclose(f);

    for (UserScore *curr = scores; curr; curr = curr->next) {
        printf("%s: %d\n", curr->username, curr->score);
    }
    // Eliberare memorie
    while (scores) {
        UserScore *tmp = scores;
        scores = scores->next;
        free(tmp);
    }
    return 0;
}