#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_LINE_LENGTH 1024

// Converts total seconds into SRT format (HH:MM:SS,000)
void format_time(int total_seconds, char *buffer) {
    int hours = total_seconds / 3600;
    int minutes = (total_seconds % 3600) / 60;
    int seconds = total_seconds % 60;
    sprintf(buffer, "%02d:%02d:%02d,000", hours, minutes, seconds);
}

// Converts a timestamp string (MM:SS or HH:MM:SS) to total seconds
int parse_timestamp(const char *timestamp) {
    int h = 0, m = 0, s = 0;
    int parts = sscanf(timestamp, "%d:%d:%d", &h, &m, &s);
    
    if (parts == 2) { // MM:SS format
        return m * 60 + s;
    } else if (parts == 3) { // HH:MM:SS format
        return h * 3600 + m * 60 + s;
    }
    return 0;
}

// Function to check if a line is a valid timestamp
int is_timestamp(const char *line) {
    regex_t regex;
    int reti = regcomp(&regex, "^[0-9]{1,2}:[0-9]{2}(:[0-9]{2})?$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }

    reti = regexec(&regex, line, 0, NULL, 0);
    regfree(&regex);
    return !reti; // Returns 1 if it's a match, 0 otherwise
}

void youtube_to_srt(const char *input_file, const char *output_file) {
    FILE *infile = fopen(input_file, "r");
    if (!infile) {
        perror("Error opening input file");
        exit(1);
    }

    FILE *outfile = fopen(output_file, "w");
    if (!outfile) {
        perror("Error creating output file");
        fclose(infile);
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    char start_time[20], end_time[20];
    char subtitle_text[MAX_LINE_LENGTH] = "";
    int subtitle_number = 1;
    int start_seconds = 0;

    while (fgets(line, sizeof(line), infile)) {
        // Remove trailing newline
        line[strcspn(line, "\r\n")] = 0;

        if (is_timestamp(line)) { // If the line is a timestamp
            if (start_seconds > 0 && subtitle_text[0] != '\0') { // Process previous subtitle
                fprintf(outfile, "%d\n%s --> %s\n%s\n\n", subtitle_number, start_time, end_time, subtitle_text);
                subtitle_number++;
                subtitle_text[0] = '\0'; // Reset subtitle text
            }

            start_seconds = parse_timestamp(line);
            format_time(start_seconds, start_time);
            format_time(start_seconds + 2, end_time); // Default duration: 2s
        } else if (start_seconds > 0) {
            if (subtitle_text[0] != '\0') strcat(subtitle_text, " ");
            strcat(subtitle_text, line);
        }
    }

    // Process last subtitle if exists
    if (start_seconds > 0 && subtitle_text[0] != '\0') {
        fprintf(outfile, "%d\n%s --> %s\n%s\n\n", subtitle_number, start_time, end_time, subtitle_text);
    }

    fclose(infile);
    fclose(outfile);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    youtube_to_srt(argv[1], argv[2]);

    printf("Conversion completed successfully!\n");
    return 0;
}
