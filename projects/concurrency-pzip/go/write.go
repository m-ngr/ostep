package main

import (
	"fmt"
	"os"
)

type JobStream struct {
	channel <-chan *Job
	nextId  uint64
	cache   map[uint64]*Job
}

func NewJobStream(ch <-chan *Job) *JobStream {
	return &JobStream{
		channel: ch,
		nextId:  0,
		cache:   make(map[uint64]*Job),
	}
}

func (s *JobStream) next() (*Job, bool) {
	if v, ok := s.cache[s.nextId]; ok == true {
		delete(s.cache, s.nextId)
		s.nextId++
		return v, true
	}

	for job := range s.channel {
		if job.id == s.nextId {
			s.nextId++
			return job, true
		} else {
			s.cache[job.id] = job
		}
	}

	return nil, false
}

func WritePipe(in <-chan *Job) {
	writer := NewUnitWriter(os.Stdout)
	defer writer.Flush()
	stream := NewJobStream(in)

	for {
		job, ok := stream.next()
		if !ok {
			break
		}

		if err := writer.Write(job.outputChunk); err != nil {
			fmt.Fprintf(os.Stderr, "pzip write error: %v\n", err)
			os.Exit(1)
		}

		job.Reset()
		jobPool.Put(job)
	}
}
