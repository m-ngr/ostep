package main

import (
	"errors"
	"fmt"
)

func fsCheck(fs *Xv6FS) {
	checkInodes(fs)
	validateRoot(fs)
	ValidateDirs(fs, 1, 1)
	checkFsUsage(fs)
	checkDoubleUsage(fs)
	CheckLinks(fs)
	// log(fs)
}

func log(fs *Xv6FS) {
	for i := range fs.super.NInodes {
		inode, _ := fs.Inode(i)
		fmt.Println(i, ":", inode)
	}
}

func checkDoubleUsage(fs *Xv6FS) {
	m := make(map[uint32]bool, 0)
	for i := range fs.super.NInodes {
		inode, _ := fs.Inode(i)
		addrs := inode.Addrs[:nDirect]
		for _, addr := range addrs {
			if addr != 0 && m[addr] == true {
				errorExit("ERROR: direct address used more than once.")
			}
			m[addr] = true
		}

		indirect := inode.Addrs[nDirect]

		if indirect == 0 {
			continue
		}

		if m[indirect] == true {
			errorExit("ERROR: indirect address used more than once.")
		}

		m[indirect] = true

		indBlock := fs.Block(indirect).AsValidAddrs()

		for _, addr := range indBlock {
			if addr != 0 && m[addr] == true {
				errorExit("ERROR: indirect address used more than once.")
			}
			m[addr] = true
		}
	}
}

func checkInodes(fs *Xv6FS) {
	for i := range fs.super.NInodes {
		inode, _ := fs.Inode(i)
		switch inode.Type {
		case T_UNUSED:
			continue
		case T_DEV, T_DIR, T_FILE:
			validateAddress(fs, &inode)
		default:
			errorExit("ERROR: bad inode.")
		}
	}
}

func checkFsUsage(fs *Xv6FS) {
	usageMap := fs.DataUsageMap()
	for i := range fs.super.NBlocks {
		addr := 59 + i
		isUsed := usageMap[addr]
		bitmap, _ := fs.IsUsed(i)

		if bitmap == isUsed {
			continue
		}

		if bitmap == true {
			// fmt.Println("i:", i, "addr:", addr)
			// fmt.Println(fs.BitMap())
			// fmt.Println(usageMap)
			errorExit("ERROR: bitmap marks block in use but it is not in use.")
		} else {
			errorExit("ERROR: address used by inode but marked free in bitmap.")
		}
	}
}

func validateAddress(fs *Xv6FS, inode *Inode) {
	for _, addr := range inode.Addrs[:nDirect] {
		if addr == 0 || (58 < addr && addr < 1000) {
			continue
		}
		errorExit("ERROR: bad direct address in inode.")
	}

	addr := inode.Addrs[nDirect]
	if addr == 0 || (58 < addr && addr < 1000) {
		indirect := fs.Block(addr).AsAddrs()
		for _, addr := range indirect {
			if addr == 0 || (58 < addr && addr < 1000) {
				continue
			}
			errorExit("ERROR: bad indirect address in inode.")
		}
	} else {
		errorExit("ERROR: bad indirect address in inode.")
	}
}

func validateAddrs(fs *Xv6FS, inode *Inode) {
	addrs := fs.GetValidAddrs(inode)

	for _, addr := range addrs {
		if used, err := fs.IsUsed(addr - 59); err != nil || used == false {
			errorExit("ERROR: address used by inode but marked free in bitmap.")
		}
	}
}

func validateRoot(fs *Xv6FS) {
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

func ValidateDirs(fs *Xv6FS, iNum uint16, parent uint16) {
	dirs, err := fs.ReadDirEnts(uint32(iNum))

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
		inode, err := fs.Inode(uint32(ent.Inum))
		if err != nil {
			errorExit("ERROR: directory not properly formatted.")
		}
		if inode.Type == T_DIR {
			ValidateDirs(fs, ent.Inum, iNum)
		}
	}
}

func findDirEnt(dirs []DirEnt, name string) (DirEnt, error) {
	for _, dir := range dirs {
		if dir.DirName() == name {
			return dir, nil
		}
	}

	return DirEnt{}, errors.New("Not Found")
}

func MapDirs(fs *Xv6FS) map[uint32]uint32 {
	m := make(map[uint32]uint32)
	m[1] = 1
	mapDirs(fs, 1, m)
	return m
}

func mapDirs(fs *Xv6FS, dirINum uint16, m map[uint32]uint32) {
	ents, err := fs.ReadDirEnts(uint32(dirINum))
	if err != nil {
		panic(err)
	}

	for _, ent := range ents {
		if ent.DirName() == "." || ent.DirName() == ".." {
			continue
		}
		inode, err := fs.Inode(uint32(ent.Inum))
		if err != nil {
			panic(err)
		}

		m[uint32(ent.Inum)] += 1

		if inode.Type == T_DIR {
			mapDirs(fs, ent.Inum, m)
		}
	}
}

func CheckLinks(fs *Xv6FS) {
	links := MapDirs(fs)

	for i := range fs.super.NInodes {
		if i < 1 {
			continue
		}
		inode, _ := fs.Inode(i)

		if inode.Type != T_UNUSED && links[i] == 0 {
			errorExit("ERROR: inode marked use but not found in a directory.")
		}

		if inode.Type == T_UNUSED && links[i] != 0 {
			errorExit("ERROR: inode referred to in directory but marked free.")
		}

		if inode.Type == T_FILE && inode.NLink != uint16(links[i]) {
			errorExit("ERROR: bad reference count for file.")
		}

		if inode.Type == T_DIR && links[i] != 1 {
			errorExit("ERROR: directory appears more than once in file system.")
		}

		if inode.Type == T_DIR && inode.NLink != 1 {
			errorExit("ERROR: directory appears more than once in file system.")
		}
	}
}
