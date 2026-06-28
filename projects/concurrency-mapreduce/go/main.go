package main

import (
	"bufio"
	"fmt"
	"map-reduce/mapreduce"
	"os"
	"strings"
)

func Map(ctx mapreduce.Emitter, filePath string) {
	file, err := os.Open(filePath)
	if err != nil {
		return
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		words := strings.FieldsFunc(line, func(r rune) bool {
			return r == ' ' || r == '\t' || r == '\n' || r == '\r'
		})

		for _, word := range words {
			ctx.Emit(word, "1")
		}
	}

	if err := scanner.Err(); err != nil {
		panic("Map failed to read file")
	}
}

func Reduce(iterator mapreduce.Getter, key string) {
	count := 0
	for {
		_, valExists := iterator.Next()
		if !valExists {
			break
		}
		count++
	}
	fmt.Printf("%s %d\n", key, count)
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: wordcount <input_file1> <input_file2> ...")
		os.Exit(1)
	}

	inputFiles := os.Args[1:]

	mapReducer := mapreduce.NewMapReducer(Map, 10, Reduce, 10, mapreduce.DefaultHashPartition)

	mapReducer.Run(inputFiles)
}
