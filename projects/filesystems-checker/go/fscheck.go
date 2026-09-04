package main

import (
	"errors"
	"xcheck/xv6fs"
)

func fsCheck(fs *xv6fs.FS) {
	checkInodes(fs)
	checkDirTree(fs)
	checkDataAllocation(fs)
	checkLinks(fs)
}

func checkInodes(fs *xv6fs.FS) {
	used := make(map[xv6fs.BlockAddr]bool, 0)

	for _, inode := range fs.Inodes {
		switch {
		case inode.IsUsed() == false:
			continue
		case inode.IsFile(), inode.IsDir(), inode.IsDevice():
			validateAddress(fs, &inode, used)
		default:
			errorExit("ERROR: bad inode.")
		}
	}
}

func validateAddress(fs *xv6fs.FS, inode *xv6fs.Inode, used map[xv6fs.BlockAddr]bool) {
	for _, addr := range inode.DirectAddrs() {
		if !fs.IsValidAddr(addr) {
			errorExit("ERROR: bad direct address in inode.")
		}

		if used[addr] == true {
			errorExit("ERROR: direct address used more than once.")
		}

		used[addr] = true
	}

	indirect, ok := inode.IndirectBlockAddr()
	if !ok {
		return
	}

	if used[indirect] == true {
		errorExit("ERROR: indirect address used more than once.")
	}

	used[indirect] = true

	indAddrs, err := fs.ReadAddrs(indirect)

	if err != nil {
		errorExit("ERROR: bad indirect address in inode.")
	}

	for _, addr := range indAddrs {
		if !fs.IsValidAddr(addr) {
			errorExit("ERROR: bad indirect address in inode.")
		}

		if used[addr] == true {
			errorExit("ERROR: indirect address used more than once.")
		}
		used[addr] = true
	}
}

func checkDirTree(fs *xv6fs.FS) {
	dirs, err := fs.ReadDirEnts(xv6fs.RootINum)

	if err != nil {
		errorExit("ERROR: root directory does not exist.")
	}

	dir, err := findDirEnt(dirs, "..")

	if err != nil || dir.Inum != 1 {
		errorExit("ERROR: root directory does not exist.")
	}

	validateDirs(fs)
}

func validateDirs(fs *xv6fs.FS) {
	fs.WalkDirs(func(parent xv6fs.INum, dir xv6fs.INum, ents []xv6fs.DirEnt) {
		parentEnt, err := findDirEnt(ents, "..")

		if err != nil || parentEnt.Inum != parent {
			errorExit("ERROR: directory not properly formatted.")
		}

		dirEnt, err := findDirEnt(ents, ".")

		if err != nil || dirEnt.Inum != dir {
			errorExit("ERROR: directory not properly formatted.")
		}
	})
}

func checkDataAllocation(fs *xv6fs.FS) {
	usageMap := fs.DataUsageMap()

	for i := range fs.Super.Nblocks {
		addr := fs.Super.Bmapstart + 1 + i
		isUsed := usageMap[addr]
		bitmap, err := fs.IsAllocated(addr)
		if err != nil {
			panic(err)
		}

		if bitmap == isUsed {
			continue
		}

		if bitmap == true {
			errorExit("ERROR: bitmap marks block in use but it is not in use.")
		} else {
			errorExit("ERROR: address used by inode but marked free in bitmap.")
		}
	}
}

func checkLinks(fs *xv6fs.FS) {
	refs := make(map[xv6fs.INum]uint16)
	refs[xv6fs.RootINum] = 1

	fs.WalkDirs(func(parent xv6fs.INum, dir xv6fs.INum, ents []xv6fs.DirEnt) {
		for _, ent := range ents {
			if ent.Inum == parent || ent.Inum == dir {
				continue
			}

			refs[ent.Inum] += 1
		}
	})

	for i, inode := range fs.Inodes {
		if i == 0 {
			continue
		}

		refCount := refs[xv6fs.INum(i)]

		if inode.IsUsed() && refCount == 0 {
			errorExit("ERROR: inode marked use but not found in a directory.")
		}

		if !inode.IsUsed() && refCount != 0 {
			errorExit("ERROR: inode referred to in directory but marked free.")
		}

		if inode.IsFile() && inode.Nlink != refCount {
			errorExit("ERROR: bad reference count for file.")
		}

		if inode.IsDir() && (refCount != 1 || inode.Nlink != 1) {
			errorExit("ERROR: directory appears more than once in file system.")
		}
	}
}

// =====================================================================

func findDirEnt(dirs []xv6fs.DirEnt, name string) (xv6fs.DirEnt, error) {
	for _, dir := range dirs {
		if dir.DirName() == name {
			return dir, nil
		}
	}

	return xv6fs.DirEnt{}, errors.New("Not Found")
}
