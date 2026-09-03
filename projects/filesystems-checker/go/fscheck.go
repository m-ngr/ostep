package main

import (
	"errors"
	"xcheck/xv6fs"
)

func fsCheck(fs *xv6fs.FS) {
	checkInodes(fs)
	validateRoot(fs)
	validateDirs(fs, 1, 1)
	checkFsUsage(fs)
	checkDoubleUsage(fs)
	checkLinks(fs)
}

func checkInodes(fs *xv6fs.FS) {
	for _, inode := range fs.Inodes {
		switch {
		case inode.IsUsed() == false:
			continue
		case inode.IsFile(), inode.IsDir(), inode.IsDevice():
			validateAddress(fs, &inode)
		default:
			errorExit("ERROR: bad inode.")
		}
	}
}

func validateAddress(fs *xv6fs.FS, inode *xv6fs.Inode) {
	for _, addr := range inode.UsedDirectAddrs() {
		if !fs.IsValidAddr(addr) {
			errorExit("ERROR: bad direct address in inode.")
		}
	}

	indirect, used := inode.IndirectBlockAddr()
	if used == false {
		return
	}

	indAddrs, err := fs.ReadAddrs(indirect)

	if err != nil {
		errorExit("ERROR: bad indirect address in inode.")
	}

	for _, addr := range indAddrs {
		if !fs.IsValidAddr(addr) {
			errorExit("ERROR: bad indirect address in inode.")
		}
	}
}

func validateRoot(fs *xv6fs.FS) {
	dirs, err := fs.ReadDirEnts(1)

	if err != nil {
		errorExit("ERROR: root directory does not exist.")
	}

	dir, err := findDirEnt(dirs, "..")

	if err != nil {
		errorExit("ERROR: root directory does not exist.")

	}

	if dir.Inum != 1 {
		errorExit("ERROR: root directory does not exist.")
	}
}

func validateDirs(fs *xv6fs.FS, iNum xv6fs.INum, parent xv6fs.INum) {
	dirs, err := fs.ReadDirEnts(iNum)

	if err != nil {
		errorExit("ERROR: directory not properly formatted.")
	}

	parentDir, err := findDirEnt(dirs, "..")

	if err != nil {
		errorExit("ERROR: directory not properly formatted.")
	}

	if parentDir.Inum != parent {
		errorExit("ERROR: directory not properly formatted.")
	}

	currDir, err := findDirEnt(dirs, ".")

	if err != nil {
		errorExit("ERROR: directory not properly formatted.")
	}

	if currDir.Inum != iNum {
		errorExit("ERROR: directory not properly formatted.")
	}

	for _, ent := range dirs {
		if ent.DirName() == "." || ent.DirName() == ".." {
			continue
		}
		inode := fs.Inodes[ent.Inum]
		if inode.IsDir() {
			validateDirs(fs, ent.Inum, iNum)
		}
	}
}

func checkDoubleUsage(fs *xv6fs.FS) {
	m := make(map[uint32]bool, 0)
	for _, inode := range fs.Inodes {
		addrs := inode.UsedDirectAddrs()
		for _, addr := range addrs {
			if m[addr] == true {
				errorExit("ERROR: direct address used more than once.")
			}
			m[addr] = true
		}

		indirect, used := inode.IndirectBlockAddr()

		if !used {
			continue
		}

		if m[indirect] == true {
			errorExit("ERROR: indirect address used more than once.")
		}

		m[indirect] = true

		indBlock, _ := fs.ReadAddrs(indirect)

		for _, addr := range indBlock {
			if m[addr] == true {
				errorExit("ERROR: indirect address used more than once.")
			}
			m[addr] = true
		}
	}
}

func checkFsUsage(fs *xv6fs.FS) {
	usageMap := fs.DataUsageMap()
	for i := range fs.Super.Nblocks {
		addr := fs.Super.Bmapstart + 1 + i
		isUsed := usageMap[addr]
		bitmap, _ := fs.IsAllocated(addr)

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

func checkAddrsAlloc(fs *xv6fs.FS, i xv6fs.INum) {
	addrs := fs.GetValidAddrs(i)

	for _, addr := range addrs {
		if used, err := fs.IsAllocated(addr); err != nil || used == false {
			errorExit("ERROR: address used by inode but marked free in bitmap.")
		}
	}
}

func findDirEnt(dirs []xv6fs.DirEnt, name string) (xv6fs.DirEnt, error) {
	for _, dir := range dirs {
		if dir.DirName() == name {
			return dir, nil
		}
	}

	return xv6fs.DirEnt{}, errors.New("Not Found")
}

func checkLinks(fs *xv6fs.FS) {
	links := MapDirs(fs)

	for i, inode := range fs.Inodes {
		if i == 0 {
			continue
		}

		linkCount := links[xv6fs.INum(i)]

		if inode.IsUsed() && linkCount == 0 {
			errorExit("ERROR: inode marked use but not found in a directory.")
		}

		if !inode.IsUsed() && linkCount != 0 {
			errorExit("ERROR: inode referred to in directory but marked free.")
		}

		if inode.IsFile() && inode.Nlink != linkCount {
			errorExit("ERROR: bad reference count for file.")
		}

		if inode.IsDir() && linkCount != 1 {
			errorExit("ERROR: directory appears more than once in file system.")
		}

		if inode.IsDir() && inode.Nlink != 1 {
			errorExit("ERROR: directory appears more than once in file system.")
		}
	}
}

// TODO: Refactor into FS itself
func MapDirs(fs *xv6fs.FS) map[xv6fs.INum]uint16 {
	m := make(map[xv6fs.INum]uint16)
	m[1] = 1
	mapDirs(fs, 1, m)
	return m
}

func mapDirs(fs *xv6fs.FS, i xv6fs.INum, m map[xv6fs.INum]uint16) {
	ents, err := fs.ReadDirEnts(i)
	if err != nil {
		panic(err)
	}

	for _, ent := range ents {
		if ent.DirName() == "." || ent.DirName() == ".." {
			continue
		}

		inode := fs.Inodes[ent.Inum]

		m[ent.Inum] += 1

		if inode.IsDir() {
			mapDirs(fs, ent.Inum, m)
		}
	}
}
