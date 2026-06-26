package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
)

func ReadPipe(filePaths []string) <-chan Chunk[byte] {
	out := make(chan Chunk[byte], CHAN_SIZE)

	go func() {
		defer close(out)
		var counter uint64 = 0
		for _, path := range filePaths {
			counter = readFile(path, counter, out)
		}
	}()

	return out
}

func readFile(filePath string, counter uint64, out chan<- Chunk[byte]) uint64 {
	file, err := os.Open(filePath)
	if err != nil {
		fmt.Fprintln(os.Stderr, "pzip: cannot open file")
		os.Exit(1)
	}
	defer file.Close()

	reader := bufio.NewReader(file)

	for {
		buff := make([]byte, CHUNK_SIZE)
		n, err := reader.Read(buff)

		if err != nil && err != io.EOF {
			fmt.Fprintf(os.Stderr, "pzip read error: %v\n", err)
			os.Exit(1)
		} else if err == io.EOF {
			break
		}

		if n > 0 {
			out <- Chunk[byte]{data: buff[:n], id: counter}
			counter++
		}
	}

	return counter
}
