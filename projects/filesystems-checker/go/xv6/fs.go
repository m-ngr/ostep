// Package xv6 contains data structures and constants for the xv6 file system.
// This package is ported from the original xv6 C codebase (mkfs.c/fs.h).
package xv6

/*
# VX6 FS:
## Layout:
- Block 0: empty (unused)
- Block 1: Super Block
- Block 2-31: Log
- Block 32-57: Inode Table
- Block 58: Bitmap
- Block 59-999: Data
## Notes:
- Disk image is little-endian
- FS size: 1000 Blocks
- Block size: 512 bytes
- Inode size: 64 bytes
- Inode[0]: not used
- Inode[1]: root inode
*/

// File System Disk Layout Constants
const (
	FSSIZE     = 1000 // Size of file system image (blocks)
	NBLOCKS    = 941  // Number of data blocks
	NINODES    = 200  // Number of inodes
	NLOG       = 30   // Number of log blocks
	LOGSTART   = 2    // Block number of first log block
	INODESTART = 32   // Block number of first inode block
	BMAPSTART  = 58   // Block number of first free map block

	SBSTART   = 1  // Superblock start block
	DATASTART = 59 // First data block
	NMETA     = 59 // Total metadata blocks

	BSIZE = 512 // Block size in bytes

	NDIRECT   = 12
	NINDIRECT = BSIZE / 4 // 128 uint32 addresses per indirect block
	MAXFILE   = NDIRECT + NINDIRECT

	// Inode types
	T_DIR  = 1
	T_FILE = 2
	T_DEV  = 3

	DIRSIZ = 14 // Max chars per directory entry filename
)

// Superblock contains file system layout metadata.
type Superblock struct {
	Size       uint32 // Size of file system image (blocks)
	Nblocks    uint32 // Number of data blocks
	Ninodes    uint32 // Number of inodes
	Nlog       uint32 // Number of log blocks
	Logstart   uint32 // Block number of first log block
	Inodestart uint32 // Block number of first inode block
	Bmapstart  uint32 // Block number of first free map block
}

// Dinode represents an on-disk inode structure (64 bytes: 8 per 512B block).
type Dinode struct {
	Type  uint16              // File type
	Major uint16              // Major device number (for T_DEV inodes only)
	Minor uint16              // Minor device number (for T_DEV inodes only)
	Nlink uint16              // Number of links to inode in file system
	Size  uint32              // Size of file (bytes)
	Addrs [NDIRECT + 1]uint32 // Data block addresses
}

// Dirent represents a directory entry structure (16 bytes: 32 per 512B block).
type Dirent struct {
	Inum uint16
	Name [DIRSIZ]byte
}
