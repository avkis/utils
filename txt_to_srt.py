import re
import sys

def youtube_to_srt(input_text):
    lines = input_text.strip().split('\n')
    srt_output = []
    subtitle_number = 1

    def format_time(seconds):
        hours = seconds // 3600
        minutes = (seconds % 3600) // 60
        seconds = seconds % 60
        return f"{hours:02}:{minutes:02}:{seconds:02}"

    # Parse lines
    i = 0
    while i < len(lines):
        if re.match(r'(?:2[0-3]|[01]\d|\d):[0-5]\d', lines[i]):  # Time format (e.g., 0:00)
            start_time = lines[i]
            start_time_parts = start_time.split(':')
            if len(start_time_parts) == 2:
                start_seconds = int(start_time_parts[0]) * 60 + int(start_time_parts[1])
            if len(start_time_parts) == 3:
                start_seconds = int(start_time_parts[0]) * 3600  + int(start_time_parts[1]) * 60 + int(start_time_parts[2])
            start_formatted = format_time(start_seconds)

            # Find the next time or end of the text
            j = i + 1
            subtitle_text = []
            while j < len(lines) and not re.match(r'(?:2[0-3]|[01]\d|\d):[0-5]\d', lines[j]):
                subtitle_text.append(lines[j])
                j += 1

            if j < len(lines):
                end_time = lines[j]
                end_time_parts = end_time.split(':')
            if len(end_time_parts) == 2:
                end_seconds = int(end_time_parts[0]) * 60 + int(end_time_parts[1])
            if len(end_time_parts) == 3:
                end_seconds = int(end_time_parts[0]) * 3600  + int(end_time_parts[1]) * 60 + int(end_time_parts[2])
            else:
                end_seconds = start_seconds + 2  # Assume each subtitle lasts 2 seconds if no end time

            end_formatted = format_time(end_seconds)

            # Add subtitle entry
            srt_output.append(f"{subtitle_number}")
            srt_output.append(f"{start_formatted} --> {end_formatted}")
            srt_output.append(' '.join(subtitle_text))
            srt_output.append('')

            subtitle_number += 1
            i = j
        else:
            i += 1

    return '\n'.join(srt_output)

def main():
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    # Read input text from file
    with open(input_file, 'r') as file:
        input_text = file.read()

    # Convert YouTube subtitles to SRT format
    srt_output = youtube_to_srt(input_text)

    # Write the result to output file
    with open(output_file, 'w') as file:
        file.write(srt_output)

if __name__ == "__main__":
    main()
