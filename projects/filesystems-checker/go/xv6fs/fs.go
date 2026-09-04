package xv6fs

import (
	"errors"
	"xcheck/xv6"
)

type INum = uint16

var ErrInvalidAddr = errors.New("Invalid Block Address")
var ErrInvalidINum = errors.New("Invalid Inode Number")
var ErrNotDir = errors.New("Inode isn't a dir")

const RootINum = 1

type WalkDirFunc = func(parent INum, dir INum, ents []DirEnt)

type FS struct {
	Reader BlockReader
	Super  SuperBlock
	bitmap []byte
	Inodes []Inode
}

func NewFS(data []byte) (*FS, error) {
	reader := NewBlockReader(data, xv6.BSIZE)
	super, bitmap, inodes, err := parseFS(reader)
	if err != nil {
		return nil, err
	}

	return &FS{
		Reader: reader,
		Super:  super,
		Inodes: inodes,
		bitmap: bitmap,
	}, nil
}

func (fs FS) WalkDirs(fn WalkDirFunc) {
	walkDirs(&fs, RootINum, RootINum, fn)
}

func (fs FS) IsValidAddr(a BlockAddr) bool {
	return (fs.Super.Bmapstart < a && a < fs.Super.Size)
}

func (fs FS) IsValidINum(i INum) bool {
	return (0 < i && i < INum(fs.Super.Ninodes))
}

func (fs FS) ReadAddrs(addr BlockAddr) ([]BlockAddr, error) {
	if !fs.IsValidAddr(addr) {
		return nil, ErrInvalidAddr
	}

	return fs.Reader.Read(addr).ToAddrs()
}

func (fs FS) ReadDirEnts(i INum) ([]DirEnt, error) {
	if !fs.IsValidINum(i) {
		return nil, ErrInvalidINum
	}

	inode := fs.Inodes[i]

	if inode.IsDir() == false {
		return nil, ErrNotDir
	}

	dirs := make([]DirEnt, 0, 2)

	for _, addr := range fs.getInodeAddrs(&inode) {
		d, err := fs.Reader.Read(addr).ToDirEnts()
		if err != nil {
			panic(err)
		}
		dirs = append(dirs, d...)
	}

	return dirs, nil
}

func (fs FS) IsAllocated(addr BlockAddr) (bool, error) {
	if !fs.IsValidAddr(addr) {
		return false, ErrInvalidAddr
	}

	byteIdx := addr / 8
	bitIdx := addr % 8

	bitmapByte := fs.bitmap[byteIdx]
	var mask byte = 0b01 << bitIdx
	bit := bitmapByte & mask

	return bit != 0, nil
}

func (fs FS) DataUsageMap() map[BlockAddr]bool {
	m := make(map[BlockAddr]bool, 0)

	for _, inode := range fs.Inodes {
		if !inode.IsUsed() {
			continue
		}
		addrs := fs.getInodeAddrs(&inode)

		for _, addr := range addrs {
			m[addr] = true
		}

		if a, ok := inode.IndirectBlockAddr(); ok {
			m[a] = true
		}
	}

	return m
}

func (fs FS) getInodeAddrs(inode *Inode) []BlockAddr {
	addrs := inode.DirectAddrs()

	indirAddr, used := inode.IndirectBlockAddr()

	if used {
		indirect, err := fs.ReadAddrs(indirAddr)
		if err != nil {
			panic(err)
		}
		addrs = append(addrs, indirect...)
	}

	return addrs
}

// ==============================================

func parseFS(reader BlockReader) (SuperBlock, []byte, []Inode, error) {
	super, err := reader.Read(xv6.SBSTART).ToSuperBlock()
	if err != nil {
		return SuperBlock{}, nil, nil, err
	}

	bitmap := reader.Read(super.Bmapstart)

	inodes := make([]Inode, 0, super.Ninodes)

	for i := super.Inodestart; len(inodes) < int(super.Ninodes); i++ {
		nodes, err := reader.Read(i).ToInodes()
		if err != nil {
			return SuperBlock{}, nil, nil, err
		}
		inodes = append(inodes, nodes...)
	}

	return super, bitmap, inodes, nil
}

func walkDirs(fs *FS, parent INum, dir INum, fn WalkDirFunc) {
	ents, err := fs.ReadDirEnts(dir)
	if err != nil {
		panic(err)
	}

	fn(parent, dir, ents)

	for _, ent := range ents {
		if ent.Inum == parent || ent.Inum == dir {
			continue
		}

		inode := fs.Inodes[ent.Inum]

		if inode.IsDir() {
			walkDirs(fs, dir, ent.Inum, fn)
		}
	}
}
