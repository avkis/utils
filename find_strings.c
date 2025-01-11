#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex>
#include <linux/limits.h>

#define MAX_STRINGS 1000
#define MAX_STRING_LENGTH 1024

// Function prototypes
void find_strings(const char *directory, const char *search_str, char strings[][MAX_STRING_LENGTH], int *count);
void save_to_json(const char strings[][MAX_STRING_LENGTH], int count, const char *output_file);
void sort_strings(char strings[][MAX_STRING_LENGTH], int count);
int string_contains(const char *str, regex_t *regex);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <search_string> <directory> <output_file>\n", argv[0]);
        return 1;
    }

    const char *search_str = argv[1];
    const char *directory = argv[2];
    const char *output_file = argv[3];

    char (*strings)[MAX_STRING_LENGTH] = malloc(MAX_STRINGS * MAX_STRING_LENGTH);
    if (strings == NULL) {
        perror("malloc");
        return 1;
    }

    int count = 0;
    find_strings(directory, search_str, strings, &count);
    sort_strings(strings, count);
    save_to_json(strings, count, output_file);

    printf("Found %d strings containing '%s'.\n", count, search_str);
    printf("Results saved to %s\n", output_file);

    free(strings);
    return 0;
}

void find_strings(const char *directory, const char *search_str, char strings[][MAX_STRING_LENGTH], int *count) {
    DIR *dir = opendir(directory);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    struct stat statbuf;
    regex_t regex;

    // Compile the regex to find strings containing the search string
    char regex_pattern[MAX_STRING_LENGTH];
    snprintf(regex_pattern, sizeof(regex_pattern), "%s", search_str);
    if (regcomp(&regex, regex_pattern, REG_EXTENDED)) {
        fprintf(stderr, "Could not compile regex\n");
        closedir(dir);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        if (stat(path, &statbuf) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            find_strings(path, search_str, strings, count);
        } else if (S_ISREG(statbuf.st_mode)) {
            if (strstr(entry->d_name, ".tsx") || strstr(entry->d_name, ".jsx")) {
                FILE *file = fopen(path, "r");
                if (!file) {
                    perror("fopen");
                    continue;
                }

                char line[MAX_STRING_LENGTH];
                while (fgets(line, sizeof(line), file)) {
                    if (string_contains(line, &regex)) {
                        // Replace single quotes with double quotes
                        for (char *p = line; *p; p++) {
                            if (*p == '\'') *p = '"';
                        }

                        // Remove newline character
                        line[strcspn(line, "\n")] = '\0';
                        strncpy(strings[(*count)++], line, MAX_STRING_LENGTH - 1);

                        if (*count >= MAX_STRINGS) {
                            fprintf(stderr, "Reached maximum number of strings (%d)\n", MAX_STRINGS);
                            fclose(file);
                            closedir(dir);
                            regfree(&regex);
                            return;
                        }
                    }
                }

                fclose(file);
            }
        }
    }

    closedir(dir);
    regfree(&regex);
}

void save_to_json(const char strings[][MAX_STRING_LENGTH], int count, const char *output_file) {
    FILE *file = fopen(output_file, "w");
    if (!file) {
        perror("fopen");
        return;
    }

    fprintf(file, "[\n");
    for (int i = 0; i < count; i++) {
        fprintf(file, "    \"%s\"%s", strings[i], (i == count - 1) ? "" : ",\n");
    }
    fprintf(file, "]\n");

    fclose(file);
}

void sort_strings(char strings[][MAX_STRING_LENGTH], int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (strcmp(strings[i], strings[j]) > 0) {
                char temp[MAX_STRING_LENGTH];
                strncpy(temp, strings[i], MAX_STRING_LENGTH);
                strncpy(strings[i], strings[j], MAX_STRING_LENGTH);
                strncpy(strings[j], temp, MAX_STRING_LENGTH);
            }
        }
    }
}

int string_contains(const char *str, regex_t *regex) {
    return regexec(regex, str, 0, NULL, 0) == 0;
}
