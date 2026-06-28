package mapreduce

import "sync"

func DefaultHashPartition(key string, numPartitions int) int {
	var hash uint64 = 5381
	for i := 0; i < len(key); i++ {
		hash = hash*33 + uint64(key[i])
	}
	return int(hash % uint64(numPartitions))
}

type Storage struct {
	partitions  []Partition
	partitioner Partitioner
}

func NewStorage(size uint, partitioner Partitioner) *Storage {
	if size == 0 {
		panic("Storage size cannot be zero")
	}

	if partitioner == nil {
		partitioner = DefaultHashPartition
	}

	arr := make([]Partition, size)
	for i := range arr {
		arr[i] = *NewPartition()
	}

	return &Storage{
		partitioner: partitioner,
		partitions:  arr,
	}
}

func (s *Storage) Emit(key string, value string) {
	i := s.partitioner(key, len(s.partitions))
	s.partitions[i].Emit(key, value)
}

func (s *Storage) GetValueIterator(p uint, key string) *Iterator[string] {
	return s.partitions[p].GetValueIterator(key)
}

func (s *Storage) GetKeys(partition uint) []string {
	return s.partitions[partition].GetKeys()
}

// =========================================================

type Partition struct {
	lock sync.Mutex
	data map[string][]string
}

func NewPartition() *Partition {
	return &Partition{
		data: make(map[string][]string),
	}
}

func (p *Partition) Emit(key string, value string) {
	p.lock.Lock()
	p.data[key] = append(p.data[key], value)
	p.lock.Unlock()
}

func (p *Partition) GetValueIterator(key string) *Iterator[string] {
	return &Iterator[string]{
		data: p.data[key],
		next: 0,
	}
}

func (p *Partition) GetKeys() []string {
	keys := make([]string, 0, len(p.data))
	for k := range p.data {
		keys = append(keys, k)
	}
	return keys
}

// =========================================================

type Iterator[T any] struct {
	data []T
	next int
}

func (it *Iterator[T]) Next() (T, bool) {
	var empty T

	if it.next >= len(it.data) {
		return empty, false
	} else {
		v := it.data[it.next]
		it.next++
		return v, true
	}
}
