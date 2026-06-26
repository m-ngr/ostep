package main

import (
	"bufio"
	"encoding/binary"
	"io"
	"unsafe"
)

type Unit [5]byte

func (u Unit) getByte() byte {
	return u[4]
}

func (u Unit) getCount() uint32 {
	return binary.LittleEndian.Uint32(u[:])
}

func (u Unit) decompose() (uint32, byte) {
	return u.getCount(), u.getByte()
}

func (u *Unit) setCount(newCount uint32) {
	binary.LittleEndian.PutUint32(u[:], newCount)
}

func (u *Unit) setByte(newbyte byte) {
	u[4] = newbyte
}

func (u Unit) equal(u2 Unit) bool {
	return u.getByte() == u2.getByte()
}

func (u *Unit) add(delta uint32) {
	u.setCount(u.getCount() + delta)
}

func NewUnit(count uint32, b byte) Unit {
	var buf Unit
	binary.LittleEndian.PutUint32(buf[0:4], count)
	buf[4] = b
	return buf
}

type UnitWriter struct {
	writer *bufio.Writer
	last   *Unit
}

func NewUnitWriter(w io.Writer) *UnitWriter {
	return &UnitWriter{
		writer: bufio.NewWriterSize(w, WRITER_SIZE),
		last:   nil,
	}
}

func (w *UnitWriter) Flush() {
	if w.last != nil {
		w.writer.Write(w.last[:])
	}
	w.writer.Flush()
}

func (w *UnitWriter) Write(data []Unit) error {
	ready, err := w.join(data)

	if err != nil {
		return err
	}

	if len(ready) == 0 {
		return nil
	}

	b := unsafe.Slice((*byte)(unsafe.Pointer(&ready[0])), len(ready)*5)

	_, err = w.writer.Write(b)
	return err
}

func (w *UnitWriter) join(data []Unit) ([]Unit, error) {
	if len(data) == 0 {
		return data, nil
	}

	if w.last != nil {
		if w.last.equal(data[0]) {
			data[0].add(w.last.getCount())
		} else {
			_, err := w.writer.Write(w.last[:])
			if err != nil {
				return data, err
			}
		}
	}

	w.last = &(data[len(data)-1])
	return data[0 : len(data)-1], nil
}
