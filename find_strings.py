import os
import re
import json

def find_substrings(directory, substr):
    mui_strings = []

    # Regex to find strings with substr
    string_pattern = re.compile(r'"[^"]*' + re.escape(substr) + '[^"]*"|\'[^\']*' + re.escape(substr) + '[^\']*\'' )

    # Walk through the directory recursively
    for root, _, files in os.walk(directory):
        for file in files:
            # Check if the file is a .tsx or .jsx file
            if file.endswith('.tsx') or file.endswith('.jsx'):
                file_path = os.path.join(root, file)
                # Open and read the file
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    # Find all matches of the regex pattern in the file content
                    matches = string_pattern.findall(content)
                    mui_strings.extend(matches)

    return mui_strings

def save_to_json(data, filename):
    # Save the data to a JSON file
    with open(filename, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=4)

def main(directory, output_file, substring):
    # Find all strings containing substring
    substrings = find_substrings(directory, substring)
    # Sort the found strings in alphabetical order
    substrings = sorted(substrings)
    # Save the sorted results to a JSON file
    save_to_json(substrings, output_file)
    print(f"Found {len(substrings)} {substring} strings.")
    print(f"Results saved to {output_file}")

if __name__ == "__main__":
    import argparse

    # Set up command-line arguments
    parser = argparse.ArgumentParser(description="Find and save strings containing '@mui' in .tsx and .jsx files.")
    parser.add_argument("directory", type=str, help="Directory to search for .tsx and .jsx files recursively")
    parser.add_argument("output_file", type=str, help="Output JSON file to save the results")

    args = parser.parse_args()

    # Run the main function with the provided arguments
    main(args.directory, args.output_file)

