package main

import (
	"io"
	"math"
	"sync"
)

var jobPool = sync.Pool{
	New: func() any {
		return NewJob(CHUNK_SIZE)
	},
}

type Job struct {
	id          uint64
	inputChunk  []byte
	outputChunk []Unit
}

func NewJob(chunkSize int) *Job {
	return &Job{
		inputChunk:  make([]byte, 0, chunkSize),
		outputChunk: make([]Unit, 0, chunkSize),
	}
}

func (j *Job) Reset() {
	j.id = 0
	j.inputChunk = j.inputChunk[:0]
	j.outputChunk = j.outputChunk[:0]
}

func (j *Job) Read(r io.Reader) (int, error) {
	buf := j.inputChunk[0:cap(j.inputChunk)]
	n, err := r.Read(buf)
	j.inputChunk = buf[0:n]
	return n, err
}

func (j *Job) Compress() {
	j.outputChunk = j.outputChunk[:0]

	if len(j.inputChunk) == 0 {
		return
	}

	var last byte = j.inputChunk[0]
	var count uint32 = 1

	for _, curr := range j.inputChunk[1:] {
		if curr == last && count < math.MaxUint32 {
			count++
			continue
		}

		j.outputChunk = append(j.outputChunk, NewUnit(count, last))
		last = curr
		count = 1
	}

	if count > 0 {
		j.outputChunk = append(j.outputChunk, NewUnit(count, last))
	}

}
