#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define USERNAME_MAX 32


typedef struct {
    int treasure_id;
    char username[USERNAME_MAX];
    float latitude;
    float longitude;
    char clue[256];
    int value;
} Treasure;

int main() {
    mkdir("hunt1", 0755);
    mkdir("hunt2", 0755);
    Treasure t1 = {1, "alice", 10.0, 20.0, "clue1", 100};
    Treasure t2 = {2, "bob", 11.0, 21.0, "clue2", 200};
    FILE *f = fopen("hunt1/treasures.dat", "wb");
    fwrite(&t1, sizeof(Treasure), 1, f);
    fwrite(&t2, sizeof(Treasure), 1, f);
    fclose(f);

    Treasure t3 = {3, "alice", 12.0, 22.0, "clue3", 150};
    Treasure t4 = {4, "carol", 13.0, 23.0, "clue4", 300};
    f = fopen("hunt2/treasures.dat", "wb");
    fwrite(&t3, sizeof(Treasure), 1, f);
    fwrite(&t4, sizeof(Treasure), 1, f);
    fclose(f);
    return 0;
}