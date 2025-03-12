package main

import (
	"bufio"
	"fmt"
	"os"
	"regexp"
	"strconv"
	"strings"
)

// formatTime converts total seconds to SRT time format (HH:MM:SS,000)
func formatTime(totalSeconds int) string {
	hours := totalSeconds / 3600
	minutes := (totalSeconds % 3600) / 60
	seconds := totalSeconds % 60
	return fmt.Sprintf("%02d:%02d:%02d,000", hours, minutes, seconds)
}

// Converts a timestamp string (e.g., "1:00:05") to total seconds
func parseTimestamp(timestamp string) int {
	parts := strings.Split(timestamp, ":")
	if len(parts) == 2 { // Format MM:SS
		minutes, _ := strconv.Atoi(parts[0])
		seconds, _ := strconv.Atoi(parts[1])
		return minutes*60 + seconds
	} else if len(parts) == 3 { // Format HH:MM:SS
		hours, _ := strconv.Atoi(parts[0])
		minutes, _ := strconv.Atoi(parts[1])
		seconds, _ := strconv.Atoi(parts[2])
		return hours*3600 + minutes*60 + seconds
	}
	return 0
}

func youtubeToSrt(inputFile, outputFile string) error {
	file, err := os.Open(inputFile)
	if err != nil {
		return fmt.Errorf("error opening input file: %v", err)
	}
	defer file.Close()

	output, err := os.Create(outputFile)
	if err != nil {
		return fmt.Errorf("error creating output file: %v", err)
	}
	defer output.Close()

	scanner := bufio.NewScanner(file)
	reTime := regexp.MustCompile(`^\d{1,2}:\d{2}(:\d{2})?$`) // Matches MM:SS or HH:MM:SS
	var srtOutput []string
	subtitleNumber := 1
	var startTime, endTime string
	var subtitleText []string

	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())

		// Check if the line is a timestamp
		if reTime.MatchString(line) {
			// Process previous subtitle if exists
			if startTime != "" && len(subtitleText) > 0 {
				srtOutput = append(srtOutput, fmt.Sprintf("%d", subtitleNumber))
				srtOutput = append(srtOutput, fmt.Sprintf("%s --> %s", startTime, endTime))
				srtOutput = append(srtOutput, strings.Join(subtitleText, " "))
				srtOutput = append(srtOutput, "")
				subtitleNumber++
				subtitleText = nil
			}

			// Convert timestamp to total seconds
			startSeconds := parseTimestamp(line)
			startTime = formatTime(startSeconds)

			// Default end time (2s after start)
			endTime = formatTime(startSeconds + 2)
		} else if startTime != "" {
			// Collect subtitle text
			subtitleText = append(subtitleText, line)
		}
	}

	// Process last subtitle if exists
	if startTime != "" && len(subtitleText) > 0 {
		srtOutput = append(srtOutput, fmt.Sprintf("%d", subtitleNumber))
		srtOutput = append(srtOutput, fmt.Sprintf("%s --> %s", startTime, endTime))
		srtOutput = append(srtOutput, strings.Join(subtitleText, " "))
		srtOutput = append(srtOutput, "")
	}

	// Write to output file
	writer := bufio.NewWriter(output)
	for _, line := range srtOutput {
		_, _ = writer.WriteString(line + "\n")
	}
	writer.Flush()

	return nil
}

func main() {
	if len(os.Args) != 3 {
		fmt.Println("Usage: go run youtube_to_srt.go <input_file> <output_file>")
		os.Exit(1)
	}

	inputFile := os.Args[1]
	outputFile := os.Args[2]

	err := youtubeToSrt(inputFile, outputFile)
	if err != nil {
		fmt.Println("Error:", err)
		os.Exit(1)
	}

	fmt.Println("Conversion completed successfully!")
}
