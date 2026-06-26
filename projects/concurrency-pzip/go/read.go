package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
)

func ReadPipe(filePaths []string) <-chan *Job {
	out := make(chan *Job, CHAN_SIZE)

	go func() {
		defer close(out)
		var counter uint64 = 0
		for _, path := range filePaths {
			counter = readFile(path, counter, out)
		}
	}()

	return out
}

func readFile(filePath string, counter uint64, out chan<- *Job) uint64 {
	file, err := os.Open(filePath)
	if err != nil {
		fmt.Fprintln(os.Stderr, "pzip: cannot open file")
		os.Exit(1)
	}
	defer file.Close()

	reader := bufio.NewReader(file)

	for {
		job := jobPool.Get().(*Job)
		n, err := job.Read(reader)

		if err != nil && err != io.EOF {
			fmt.Fprintf(os.Stderr, "pzip read error: %v\n", err)
			os.Exit(1)
		} else if err == io.EOF {
			job.Reset()
			jobPool.Put(job)
			break
		}

		if n > 0 {
			job.id = counter
			counter++
			out <- job
		} else {
			job.Reset()
			jobPool.Put(job)
		}
	}

	return counter
}
