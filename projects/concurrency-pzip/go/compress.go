package main

import (
	"sync"
)

func CompressPipe(in <-chan *Job, workers uint8) <-chan *Job {
	out := make(chan *Job, CHAN_SIZE)
	var wg sync.WaitGroup
	wg.Add(int(workers))

	for range workers {
		go func() {
			defer wg.Done()
			for job := range in {
				job.Compress()
				out <- job
			}
		}()
	}

	go func() {
		wg.Wait()
		close(out)
	}()

	return out
}
