package xv6fs

import (
	"bytes"
	"xcheck/xv6"
)

type BlockReader struct {
	data      []byte
	blockSize uint32
}

type Block []byte
type BlockAddr = uint32
type SuperBlock xv6.Superblock
type DirEnt xv6.Dirent

func (d DirEnt) DirName() string {
	return string(bytes.TrimRight(d.Name[:], "\x00"))
}

func NewBlockReader(data []byte, blockSize uint32) BlockReader {
	return BlockReader{
		data:      data,
		blockSize: blockSize,
	}
}

func (r BlockReader) Read(i BlockAddr) Block {
	s := i * r.blockSize
	e := s + r.blockSize
	if e > BlockAddr(len(r.data)) {
		return nil
	}
	return r.data[s:e]
}

func (b Block) ToSuperBlock() (SuperBlock, error) {
	sb, err := BytesToStruct[xv6.Superblock](b)
	return SuperBlock(sb), err
}

func (b Block) ToInodes() ([]Inode, error) {
	di, err := BytesToSlice[xv6.Dinode](b)
	if err != nil {
		return nil, err
	}
	return MapSlice(di, func(d xv6.Dinode) Inode { return NewInode(d) }), nil
}

func (b Block) ToDirEnts() ([]DirEnt, error) {
	de, err := BytesToSlice[xv6.Dirent](b)
	if err != nil {
		return nil, err
	}

	dirents := MapSlice(
		Filter(de, func(e xv6.Dirent) bool { return e.Inum > 0 }),
		func(e xv6.Dirent) DirEnt { return DirEnt(e) },
	)
	return dirents, nil
}

func (b Block) ToAddrs() ([]BlockAddr, error) {
	addrs, err := BytesToSlice[BlockAddr](b)
	if err != nil {
		return nil, err
	}
	return Filter(addrs, func(a BlockAddr) bool { return a > 0 }), nil
}
