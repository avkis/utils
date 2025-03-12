import re
import sys

def format_time(total_seconds):
    """Convert total seconds to SRT time format (HH:MM:SS,000)"""
    hours = total_seconds // 3600
    minutes = (total_seconds % 3600) // 60
    seconds = total_seconds % 60
    return f"{hours:02}:{minutes:02}:{seconds:02},000"

def parse_timestamp(timestamp):
    """Convert a timestamp (MM:SS or HH:MM:SS) into total seconds"""
    parts = list(map(int, timestamp.split(":")))
    if len(parts) == 2:  # Format MM:SS
        return parts[0] * 60 + parts[1]
    elif len(parts) == 3:  # Format HH:MM:SS
        return parts[0] * 3600 + parts[1] * 60 + parts[2]
    return 0

def youtube_to_srt(input_file, output_file):
    """Convert YouTube subtitle format to SRT format"""
    with open(input_file, "r", encoding="utf-8") as infile:
        lines = [line.strip() for line in infile.readlines()]

    re_time = re.compile(r"^\d{1,2}:\d{2}(:\d{2})?$")  # Matches MM:SS or HH:MM:SS
    srt_output = []
    subtitle_number = 1
    start_time, end_time = None, None
    subtitle_text = []

    for line in lines:
        if re_time.match(line):  # If it's a timestamp
            # Process previous subtitle if exists
            if start_time and subtitle_text:
                srt_output.append(f"{subtitle_number}")
                srt_output.append(f"{start_time} --> {end_time}")
                srt_output.append(" ".join(subtitle_text))
                srt_output.append("")
                subtitle_number += 1
                subtitle_text = []

            # Convert timestamp to total seconds
            start_seconds = parse_timestamp(line)
            start_time = format_time(start_seconds)
            end_time = format_time(start_seconds + 2)  # Default duration: 2s

        elif start_time:
            subtitle_text.append(line)

    # Process last subtitle if exists
    if start_time and subtitle_text:
        srt_output.append(f"{subtitle_number}")
        srt_output.append(f"{start_time} --> {end_time}")
        srt_output.append(" ".join(subtitle_text))
        srt_output.append("")

    # Write to output file
    with open(output_file, "w", encoding="utf-8") as outfile:
        outfile.write("\n".join(srt_output) + "\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python youtube_to_srt.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    youtube_to_srt(input_file, output_file)
    print("Conversion completed successfully!")
