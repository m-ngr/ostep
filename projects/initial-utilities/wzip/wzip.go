package main

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"io"
	"os"
)

type WzipWriter struct {
	out *bufio.Writer
}

func NewWzipWriter(w io.Writer) *WzipWriter {
	return &WzipWriter{
		out: bufio.NewWriter(w),
	}
}

func (w WzipWriter) Write(count uint32, b byte) {
	var buf [5]byte
	binary.LittleEndian.PutUint32(buf[0:4], count)
	buf[4] = b
	w.out.Write(buf[:])
}

func (w WzipWriter) Flush() {
	w.out.Flush()
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("wzip: file1 [file2 ...]")
		os.Exit(1)
	}

	writer := NewWzipWriter(os.Stdout)
	defer writer.Flush()

	var last byte = 0
	var count int32 = 0
	firstChar := true

	for _, arg := range os.Args[1:] {
		file, err := os.Open(arg)
		if err != nil {
			fmt.Fprintln(os.Stderr, "wzip: cannot open file")
			os.Exit(1)
		}

		err = func() error {
			defer file.Close()
			reader := bufio.NewReader(file)

			for {
				curr, err := reader.ReadByte()
				if err != nil {
					if err == io.EOF {
						break
					}
					return err
				}

				if firstChar {
					last = curr
					count = 1
					firstChar = false
					continue
				}

				if curr == last {
					count++
				} else {
					writer.Write(uint32(count), last)
					last = curr
					count = 1
				}
			}
			return nil
		}()

		if err != nil {
			fmt.Fprintln(os.Stderr, "wzip: error reading file")
			os.Exit(1)
		}

	}

	if count > 0 {
		writer.Write(uint32(count), last)
	}
}
