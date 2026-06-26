package main

import (
	"fmt"
	"os"
	"runtime"
)

const CHAN_SIZE = 1000
const CHUNK_SIZE = 4096 //4KB
const WRITER_SIZE = 4096

var cpu uint8 = uint8(runtime.NumCPU())

type Chunk[T any] struct {
	id   uint64
	data []T
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("pzip: file1 [file2 ...]")
		os.Exit(1)
	}

	WritePipe(CompressPipe(ReadPipe(os.Args[1:]), cpu))
}
