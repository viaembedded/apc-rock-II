/*
 * alloc_sb.c --- Allocate the superblock and block group descriptors for a
 * newly initialized filesystem.  Used by mke2fs when initializing a filesystem
 *
 * Copyright (C) 1994, 1995, 1996, 2003 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Library
 * General Public License, version 2.
 * %End-Header%
 */

#include <common.h>
//#include <ext_common.h>
//#include <ext4fs.h>
#include <malloc.h>
#include <stddef.h>
#include <linux/stat.h>
#include <linux/time.h>


#include "ext2_fs.h"
#include "ext2fs.h"

/*
 * This function reserves the superblock and block group descriptors
 * for a given block group.  It currently returns the number of free
 * blocks assuming that inode table and allocation bitmaps will be in
 * the group.  This is not necessarily the case when the flex_bg
 * feature is enabled, so callers should take care!  It was only
 * really intended for use by mke2fs, and even there it's not that
 * useful.  In the future, when we redo this function for 64-bit block
 * numbers, we should probably return the number of blocks used by the
 * super block and group descriptors instead.
 *
 * See also the comment for ext2fs_super_and_bgd_loc()
 */
int ext2fs_reserve_super_and_bgd(ext2_filsys fs,
				 dgrp_t group,
				 ext2fs_block_bitmap bmap)
{
	blk_t	super_blk, old_desc_blk, new_desc_blk;
	int	j, old_desc_blocks, num_blocks;

	num_blocks = ext2fs_super_and_bgd_loc(fs, group, &super_blk,
					      &old_desc_blk, &new_desc_blk, 0);//how many data blocks


	if (fs->super->s_feature_incompat & EXT2_FEATURE_INCOMPAT_META_BG)
		old_desc_blocks = fs->super->s_first_meta_bg;
	else
		old_desc_blocks =
			fs->desc_blocks + fs->super->s_reserved_gdt_blocks;//how many block the group descriptions will take and reserve

	if (super_blk || (group == 0))
		ext2fs_mark_block_bitmap(bmap, super_blk);//mark the superblock in block bitmap

	if (old_desc_blk) {
		if (fs->super->s_reserved_gdt_blocks && fs->block_map == bmap)
			fs->group_desc[group].bg_flags &= ~EXT2_BG_BLOCK_UNINIT;
		for (j=0; j < old_desc_blocks; j++)
			if (old_desc_blk + j < fs->super->s_blocks_count)
				ext2fs_mark_block_bitmap(bmap,
							 old_desc_blk + j);//mar the group descriptions in the block bitmap
	}
	if (new_desc_blk)
		ext2fs_mark_block_bitmap(bmap, new_desc_blk);

	return num_blocks;//how many data blocks
}
