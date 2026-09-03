package xv6fs

import "xcheck/xv6"

type Inode struct {
	xv6.Dinode
}

func NewInode(di xv6.Dinode) Inode {
	return Inode{Dinode: di}
}

func (n Inode) IsUsed() bool {
	return n.Type != 0
}

func (n Inode) IsDir() bool {
	return n.Type == xv6.T_DIR
}

func (n Inode) IsFile() bool {
	return n.Type == xv6.T_FILE
}

func (n Inode) IsDevice() bool {
	return n.Type == xv6.T_DEV
}

func (n Inode) DirectAddrs() []BlockAddr {
	return n.Addrs[:xv6.NDIRECT]
}

func (n Inode) UsedDirectAddrs() []BlockAddr {
	return Filter(n.DirectAddrs(), func(a BlockAddr) bool {
		return a > 0
	})
}

func (n Inode) IndirectBlockAddr() (BlockAddr, bool) {
	addr := n.Addrs[xv6.NDIRECT]
	return addr, addr > 0
}
