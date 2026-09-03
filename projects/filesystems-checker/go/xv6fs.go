package main

import (
	"bytes"
	"encoding/binary"
	"errors"
	"unsafe"
)

const blockSize = 512
const nDirect = 12
const nIndirect = blockSize / 4
const inodeSize = 64

type Block []byte

func (b Block) AsAddrs() []uint32 {
	if len(b) < 4 {
		return []uint32{}
	}
	return unsafe.Slice((*uint32)(unsafe.Pointer(&b[0])), len(b)/4)
}

func (b Block) AsValidAddrs() []uint32 {
	all := b.AsAddrs()
	var valid []uint32

	for _, addr := range all {
		if addr != 0 {
			valid = append(valid, addr)
		}
	}

	return valid
}

func (b Block) AsDirEnts() []DirEnt {
	dirEntSize := int(unsafe.Sizeof(DirEnt{}))
	if len(b) < dirEntSize {
		return []DirEnt{}
	}
	return unsafe.Slice((*DirEnt)(unsafe.Pointer(&b[0])), len(b)/dirEntSize)
}

func (b Block) AsValidDirEnts() []DirEnt {
	all := b.AsDirEnts()
	var valid []DirEnt

	for _, dir := range all {
		if dir.Inum != 0 {
			valid = append(valid, dir)
		}
	}

	return valid
}

type InodeType uint16

const (
	T_UNUSED InodeType = 0
	T_DIR    InodeType = 1
	T_FILE   InodeType = 2
	T_DEV    InodeType = 3
)

var ErrOutOfBound = errors.New("XV6 FS: Out Of Bound")

type SuperBlock struct {
	Size       uint32 // Size of file system image (blocks)
	NBlocks    uint32 // Number of data blocks
	NInodes    uint32 // Number of inodes
	NLog       uint32 // Number of log blocks
	LogStart   uint32 // Block number of first log block
	InodeStart uint32 // Block number of first inode block
	BmapStart  uint32 // Block number of first free map block
}

type Inode struct {
	Type  InodeType           // File type
	Major uint16              // Major device number (for T_DEV inodes only)
	Minor uint16              // Minor device number (for T_DEV inodes only)
	NLink uint16              // Number of links to inode in file system
	Size  uint32              // Size of file (bytes)
	Addrs [nDirect + 1]uint32 // Data block addresses
}

type DirEnt struct {
	Inum uint16
	Name [14]byte
}

func (d DirEnt) DirName() string {
	return string(bytes.TrimRight(d.Name[:], "\x00"))
}

type Xv6FS struct {
	data  []byte
	super SuperBlock
}

func NewXv6FS(data []byte) (*Xv6FS, error) {
	fs := &Xv6FS{data: data}
	sb, err := fs.SuperBlock()
	if err != nil {
		return &Xv6FS{}, err
	}
	fs.super = sb
	return fs, nil
}

func (fs Xv6FS) DataUsageMap() map[uint32]bool {
	m := make(map[uint32]bool, 0)
	for i := range fs.super.NInodes {
		inode, _ := fs.Inode(i)
		addrs := fs.GetValidAddrs(&inode)
		// fmt.Println(i, ":", inode, ":", addrs)
		for _, addr := range addrs {
			m[addr] = true
		}
		indirect := inode.Addrs[nDirect]
		if indirect != 0 {
			m[indirect] = true
		}
	}
	return m
}

func (fs Xv6FS) Block(i uint32) Block {
	s := i * blockSize
	e := s + blockSize
	if e > uint32(cap(fs.data)) {
		return Block{}
	}
	return fs.data[s:e]
}

func (fs *Xv6FS) SuperBlock() (SuperBlock, error) {
	var sb SuperBlock

	reader := bytes.NewReader(fs.Block(1))

	if err := binary.Read(reader, binary.LittleEndian, &sb); err != nil {
		return SuperBlock{}, err
	}

	fs.super = sb

	return sb, nil
}

func (fs Xv6FS) BitMap() []byte {
	return fs.Block(fs.super.BmapStart)
}

func (fs Xv6FS) Inode(i uint32) (Inode, error) {
	if i >= fs.super.NInodes {
		return Inode{}, ErrOutOfBound
	}
	base := fs.super.InodeStart * blockSize
	s := base + i*inodeSize
	data := fs.data[s : s+inodeSize]
	reader := bytes.NewReader(data)
	var inode Inode

	if err := binary.Read(reader, binary.LittleEndian, &inode); err != nil {
		return Inode{}, err
	}

	return inode, nil
}

func (fs Xv6FS) Data(i uint32) ([]byte, error) {
	if i >= fs.super.NBlocks {
		return []byte{}, ErrOutOfBound
	}
	return fs.Block(fs.super.BmapStart + 1 + i), nil
}

func (fs Xv6FS) Log(i uint32) ([]byte, error) {
	if i >= fs.super.NLog {
		return []byte{}, ErrOutOfBound
	}
	return fs.Block(fs.super.LogStart + i), nil
}

func (fs Xv6FS) IsUsed(i uint32) (bool, error) {
	i += 59
	if i >= fs.super.NBlocks {
		return false, ErrOutOfBound
	}
	byteIdx := i / 8
	bitIdx := i % 8

	bitmapByte := fs.BitMap()[byteIdx]
	var mask byte = 0b01 << bitIdx
	bit := bitmapByte & mask

	return bit != 0, nil
}

func (fs Xv6FS) ReadDirEnts(iNum uint32) ([]DirEnt, error) {
	inode, err := fs.Inode(iNum)

	if err != nil {
		return nil, err
	}

	if inode.Type != T_DIR {
		return nil, errors.New("Inode isn't a dir")
	}

	dirs := make([]DirEnt, 0, 2)

	for _, addr := range fs.GetValidAddrs(&inode) {
		d := fs.Block(addr).AsValidDirEnts()
		dirs = append(dirs, d...)
	}

	return dirs, nil
}

func (fs Xv6FS) GetValidAddrs(inode *Inode) []uint32 {
	addrs := make([]uint32, 0, nDirect)

	for _, a := range inode.Addrs[:nDirect] {
		if a == 0 {
			continue
		}
		addrs = append(addrs, a)
	}

	indirAddr := inode.Addrs[nDirect]

	if indirAddr != 0 {
		indirect := fs.Block(indirAddr).AsValidAddrs()

		addrs = append(addrs, indirect...)
	}

	return addrs
}
