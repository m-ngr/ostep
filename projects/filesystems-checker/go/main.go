package main

import (
	"fmt"
	"os"
	"xcheck/xv6fs"

	"golang.org/x/sys/unix"
)

func main() {
	if len(os.Args) != 2 {
		errorExit("Usage: xcheck <file_system_image>")
	}

	file, err := os.Open(os.Args[1])
	if err != nil {
		errorExit("image not found.")
	}
	defer file.Close()

	data := mmap(file)
	defer unix.Munmap(data)

	fs, err := xv6fs.NewFS(data)

	if err != nil {
		errorExit("couldn't init the FS Reader")
	}

	fsCheck(fs)
}

func mmap(file *os.File) []byte {
	stat, err := file.Stat()
	if err != nil {
		errorExit("Failed to stat file")
	}

	fileSize := stat.Size()
	if fileSize == 0 {
		errorExit("Cannot mmap an empty file")
	}

	data, err := unix.Mmap(
		int(file.Fd()),
		0,
		int(fileSize),
		unix.PROT_READ,
		unix.MAP_SHARED,
	)

	if err != nil {
		errorExit("mmap failed")
	}

	return data

}

func errorExit(msg string) {
	fmt.Fprintln(os.Stderr, msg)
	os.Exit(1)
}
