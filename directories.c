#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

int main (int argc, char **argv){
    if (argc < 2) {
        printf("Usage: %s path", argv[0]);
        exit(EXIT_FAILURE);
    }

    DIR *d = opendir(argv[1]);
    if (d==NULL){
        printf("Directory does not exits");
        exit(EXIT_FAILURE);
    }
    struct dirent* current;
    struct stat info;
    do {
        current = readdir(d);
        char *path = malloc(sizeof(char)*(sizeof(argv[1]) + strlen(current->d_name) + 3));
        strcpy(path, argv[1]);
        strcat(path, "/");
        strcat(path, current->d_name);

        stat(path, &info);
        printf("Name: %s\n", current->d_name);
        printf("Inode number: %i\n", current->d_ino);
        printf("File size: %lld bytes\n", (long long) info.st_size);
        printf("User ID: %i\n", info.st_uid);
        printf("\n");
    } while (current != NULL);
    closedir(d);
    return 0;
}