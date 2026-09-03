package xv6fs

import (
	"errors"
	"xcheck/xv6"
)

type INum = uint16

var ErrInvalidAddr = errors.New("Invalid Block Address")

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

func (fs FS) Root() Inode {
	return fs.Inodes[1]
}

func (fs FS) WalkDirs(cb func(parent INum, dir INum, entries []DirEnt)) {
	walkDirs(&fs, 1, 1, cb)
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
		return nil, ErrInvalidAddr
	}

	inode := fs.Inodes[i]

	if inode.IsDir() == false {
		return nil, errors.New("Inode isn't a dir")
	}

	dirs := make([]DirEnt, 0, 2)

	for _, addr := range fs.GetValidAddrs(i) {
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

func (fs FS) GetValidAddrs(i INum) []BlockAddr {
	inode := fs.Inodes[i]
	addrs := inode.UsedDirectAddrs()

	indirAddr, used := inode.IndirectBlockAddr()

	if used {
		indirect, _ := fs.ReadAddrs(indirAddr)
		addrs = append(addrs, indirect...)
	}

	return addrs
}

func (fs FS) DataUsageMap() map[uint32]bool {
	m := make(map[uint32]bool, 0)

	for i, inode := range fs.Inodes {
		addrs := fs.GetValidAddrs(INum(i))

		for _, addr := range addrs {
			m[addr] = true
		}

		if a, ok := inode.IndirectBlockAddr(); ok {
			m[a] = true
		}
	}

	return m
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

func walkDirs(fs *FS, dir INum, parent INum, cb func(parent INum, dir INum, entries []DirEnt)) {
	ents, err := fs.ReadDirEnts(dir)

	if err != nil {
		panic(err)
	}

	cb(parent, dir, ents)

	for _, ent := range ents {
		if ent.DirName() == "." || ent.DirName() == ".." {
			continue
		}

		inode := fs.Inodes[ent.Inum]

		if inode.IsDir() {
			walkDirs(fs, ent.Inum, dir, cb)
		}
	}
}
