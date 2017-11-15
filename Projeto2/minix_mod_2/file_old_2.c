/*
 *  linux/fs/minix/file.c
 *
 *  Copyright (C) 1991, 1992 Linus Torvalds
 *
 *  minix regular file handling primitives
 */

#include "minix.h"
#include <linux/aio.h>
#include <linux/linkage.h>
#include <linux/wait.h>
#include <linux/kdev_t.h>
#include <linux/dcache.h>
#include <linux/path.h>
#include <linux/stat.h>
#include <linux/cache.h>
#include <linux/list.h>
#include <linux/list_lru.h>
#include <linux/llist.h>
#include <linux/radix-tree.h>
#include <linux/rbtree.h>
#include <linux/init.h>
#include <linux/pid.h>
#include <linux/bug.h>
#include <linux/mutex.h>
#include <linux/rwsem.h>
#include <linux/capability.h>
#include <linux/semaphore.h>
#include <linux/fiemap.h>
#include <linux/rculist_bl.h>
#include <linux/atomic.h>
#include <linux/shrinker.h>
#include <linux/migrate_mode.h>
#include <linux/uidgid.h>
#include <linux/lockdep.h>
#include <linux/percpu-rwsem.h>
#include <linux/blk_types.h>

#include <asm/byteorder.h>
#include <uapi/linux/fs.h>
#include <linux/kernel.h>
#include <uapi/linux/uio.h>
#include <linux/types.h>

#include <linux/export.h>
#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/capability.h>
#include <linux/kernel_stat.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/mman.h>
#include <linux/pagemap.h>
#include <linux/file.h>
#include <linux/uio.h>
#include <linux/hash.h>
#include <linux/writeback.h>
#include <linux/backing-dev.h>
#include <linux/pagevec.h>
#include <linux/blkdev.h>
#include <linux/security.h>
#include <linux/cpuset.h>
#include <linux/hardirq.h> // for BUG_ON(!in_atomic()) only 
#include <linux/hugetlb.h>
#include <linux/memcontrol.h>
#include <linux/cleancache.h>
#include <linux/rmap.h>

#include <trace/events/filemap.h>


ssize_t my_son(struct kiocb *iocb, struct iov_iter *from);

const struct file_operations minix_file_operations = {
	
	.llseek			= generic_file_llseek,
	.read_iter		= generic_file_read_iter,
	//.write_iter		= generic_file_write_iter,
	.write_iter		= my_son,
	.mmap			= generic_file_mmap,
	.fsync			= generic_file_fsync,
	.splice_read	= generic_file_splice_read,
};

static int minix_setattr(struct dentry *dentry, struct iattr *attr)
{
	printk("file %s line 28\n", __PRETTY_FUNCTION__);
	struct inode *inode = d_inode(dentry);
	int error;

	error = setattr_prepare(dentry, attr);
	if (error)
		return error;

	if ((attr->ia_valid & ATTR_SIZE) &&
	    attr->ia_size != i_size_read(inode)) {
		error = inode_newsize_ok(inode, attr->ia_size);
		if (error)
			return error;

		truncate_setsize(inode, attr->ia_size);
		minix_truncate(inode);
	}

	setattr_copy(inode, attr);
	mark_inode_dirty(inode);
	return 0;
}

ssize_t my_son(struct kiocb *iocb, struct iov_iter *from)
{
	printk("file %s line 178\n", __PRETTY_FUNCTION__);

	//printk("file %d line 180\n", from->type);
	//printk("file %u line 181\n", from->iov_offset);
	printk("file %u count  \n", from->count);
	printk("file %s line 183\n", from->iov->iov_base);
	printk("file %d len\n", from->iov->iov_len);
	printk("file %d pos\n", iocb->ki_pos);
	
	return generic_file_write_iter(iocb,from); // Como se fosse o Super do java!

}

const struct inode_operations minix_file_inode_operations = {
	.setattr	= minix_setattr,
	.getattr	= minix_getattr,
};
