package main

import (
	"fmt"
	"os"
)

type ChunkStream struct {
	channel <-chan Chunk[Unit]
	nextId  uint64
	cache   map[uint64][]Unit
}

func NewChunkStream(ch <-chan Chunk[Unit]) *ChunkStream {
	return &ChunkStream{
		channel: ch,
		nextId:  0,
		cache:   make(map[uint64][]Unit),
	}
}

func (s *ChunkStream) next() ([]Unit, bool) {
	if v, ok := s.cache[s.nextId]; ok == true {
		delete(s.cache, s.nextId)
		s.nextId++
		return v, true
	}

	for chunk := range s.channel {
		if chunk.id == s.nextId {
			s.nextId++
			return chunk.data, true
		} else {
			s.cache[chunk.id] = chunk.data
		}
	}

	return nil, false
}

func WritePipe(in <-chan Chunk[Unit]) {
	writer := NewUnitWriter(os.Stdout)
	defer writer.Flush()
	stream := NewChunkStream(in)

	for {
		data, ok := stream.next()
		if !ok {
			break
		}

		if err := writer.Write(data); err != nil {
			fmt.Fprintf(os.Stderr, "pzip write error: %v\n", err)
			os.Exit(1)
		}
	}
}
