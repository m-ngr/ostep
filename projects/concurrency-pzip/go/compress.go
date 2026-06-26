package main

import (
	"math"
	"sync"
)

func CompressPipe(in <-chan Chunk[byte], workers uint8) <-chan Chunk[Unit] {
	out := make(chan Chunk[Unit], CHAN_SIZE)
	var wg sync.WaitGroup
	wg.Add(int(workers))

	for range workers {
		go func() {
			defer wg.Done()
			for chunk := range in {
				out <- Chunk[Unit]{
					data: compress(chunk.data),
					id:   chunk.id,
				}
			}
		}()
	}

	go func() {
		wg.Wait()
		close(out)
	}()

	return out
}

func compress(data []byte) []Unit {
	if len(data) == 0 {
		return []Unit{}
	}

	var last byte = data[0]
	var count uint32 = 1
	result := make([]Unit, 0, len(data))

	for _, curr := range data[1:] {
		if curr == last && count < math.MaxUint32 {
			count++
			continue
		}

		result = append(result, NewUnit(count, last))
		last = curr
		count = 1
	}

	if count > 0 {
		result = append(result, NewUnit(count, last))
	}

	return result
}
