package mapreduce

import (
	"slices"
	"sync"
)

type Emitter interface {
	Emit(key string, value string)
}

type Getter interface {
	Next() (string, bool)
}

type Mapper func(ctx Emitter, filePath string)

type Reducer func(ctx Getter, key string)

type Partitioner func(key string, numPartitions int) int

type MapReducer struct {
	mapper        Mapper
	reducer       Reducer
	mappersCount  uint
	reducersCount uint
	storage       *Storage
}

func NewMapReducer(mapper Mapper, mappersCount uint, reducer Reducer, reducersCount uint, part Partitioner) *MapReducer {

	if mapper == nil || reducer == nil {
		panic("mapper and reducer functions cannot be nil")
	}

	return &MapReducer{
		mapper:        mapper,
		mappersCount:  mappersCount,
		reducer:       reducer,
		reducersCount: reducersCount,
		storage:       NewStorage(reducersCount, part),
	}
}

func (m *MapReducer) Run(files []string) {
	var filesLock sync.Mutex
	var mappersWg sync.WaitGroup
	mappersWg.Add(int(m.mappersCount))

	for range m.mappersCount {
		go func() {
			defer mappersWg.Done()

			for {
				filesLock.Lock()
				if len(files) == 0 {
					filesLock.Unlock()
					break
				}
				path := files[0]
				files = files[1:]
				filesLock.Unlock()
				m.mapper(m.storage, path)
			}
		}()
	}

	mappersWg.Wait()
	//========================================
	var reducersWg sync.WaitGroup
	reducersWg.Add(int(m.reducersCount))

	for p := range m.reducersCount {
		go func() {
			defer reducersWg.Done()

			keys := m.storage.GetKeys(p)
			slices.Sort(keys)
			for _, key := range keys {
				m.reducer(m.storage.GetValueIterator(p, key), key)
			}
		}()
	}

	reducersWg.Wait()
}
