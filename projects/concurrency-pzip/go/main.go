package main

import (
	"fmt"
	"os"
	"runtime"
)

const CHAN_SIZE = 1000
const CHUNK_SIZE = 4096
const WRITER_SIZE = 4096

var cpu uint8 = uint8(runtime.NumCPU())

func main() {
	if len(os.Args) < 2 {
		fmt.Println("pzip: file1 [file2 ...]")
		os.Exit(1)
	}

	WritePipe(CompressPipe(ReadPipe(os.Args[1:]), cpu))
}
