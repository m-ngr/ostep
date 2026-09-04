package xv6fs

import (
	"bytes"
	"encoding/binary"
	"fmt"
)

func Map[T, U any](input []T, convert func(T) U) []U {
	output := make([]U, len(input))
	for i, val := range input {
		output[i] = convert(val)
	}
	return output
}

func Filter[T any](s []T, keep func(T) bool) []T {
	result := make([]T, 0, len(s)) // Pre-allocate capacity
	for _, val := range s {
		if keep(val) {
			result = append(result, val)
		}
	}
	return result
}

func BytesToStruct[T any](b []byte) (T, error) {
	var target T

	err := binary.Read(bytes.NewReader(b), binary.LittleEndian, &target)
	if err != nil {
		return target, err
	}

	return target, nil
}

func BytesToSlice[T any](b []byte) ([]T, error) {
	var zero T
	structSize := binary.Size(zero)
	if structSize <= 0 {
		return nil, fmt.Errorf("type %T is not fixed-size", zero)
	}
	count := len(b) / structSize

	if count == 0 {
		return nil, nil
	}

	target := make([]T, count)

	err := binary.Read(bytes.NewReader(b[:count*structSize]), binary.LittleEndian, &target)
	if err != nil {
		return nil, err
	}

	return target, nil
}
