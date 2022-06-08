#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"

/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44

/* 512 bytes = 128 * 4 
   sizeof block_size_t = 32 bits = 4 bytes.
   Each attribute in inode_disk is 4 bytes long.
   It has 4 attributes so there's 124 * 4 bytes left.
   That means we have room for an array of 124 block_sector_t values. */
#define DIRECT_BLOCKS 124

/* A 512 bytes block can hold up to 128 block_size_t values.
   The same value applies for pointers. */
#define INDIRECT_BLOCKS 128

/* TODO: 
   - Change inode_disk UNUSED length based on new inode size (size o list).
   - Change functios to work with lists instead of arrays.
     * byte_to_sector DONE
     * inode_create DONE
     * inode_close DONE
     * inode_open DONE
     * inode_write_at DONE
   - Create funcion to get SIZE sectors. If there's not enough space in freemap, free sectors, otherwise, add to file_sectors. */

/* On-disk inode.
   Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk
  {
    block_sector_t direct_blocks[DIRECT_BLOCKS];  /* Direct blocks (lvl 0). */
    block_sector_t indirect_block;                /* 1st level ind. block. */
    block_sector_t doubly_indirect_block;         /* 2nd level ind. block. */
    off_t length;                                 /* File size in bytes. */
    unsigned magic;                               /* Magic number. */
  };

bool inode_get_sectors (struct inode_disk *, off_t);

/*Returns the number of sectors to allocate for an inode SIZE
   bytes long.*/
static inline size_t
bytes_to_sectors (off_t size)
{
  return DIV_ROUND_UP (size, BLOCK_SECTOR_SIZE);
}

/* In-memory inode. */
struct inode
  {
    struct list_elem elem;              /* Element in inode list. */
    block_sector_t sector;              /* Sector number of disk location. */
    int open_cnt;                       /* Number of openers. */
    bool removed;                       /* True if deleted, false otherwise. */
    int deny_write_cnt;                 /* 0: writes ok, >0: deny writes. */
    struct inode_disk data;             /* Inode content. */
  };

/* Returns the block device sector that contains byte offset POS
   within INODE.
   Returns -1 if INODE does not contain data for a byte at offset
   POS. */
static block_sector_t
byte_to_sector (const struct inode *inode, off_t pos)
{
  ASSERT (inode != NULL);
  if (pos < inode->data.length) 
  {
    size_t block_index = pos / BLOCK_SECTOR_SIZE;

    if (block_index < DIRECT_BLOCKS)
      return inode->data.direct_blocks[block_index];
    else if (block_index < DIRECT_BLOCKS + INDIRECT_BLOCKS)
    {
      block_sector_t arr[INDIRECT_BLOCKS];
      block_read (fs_device, inode->data.indirect_block, arr);
      return arr[block_index];
    }
    else 
    {
      off_t offset = block_index - DIRECT_BLOCKS - INDIRECT_BLOCKS;
      off_t doubly_block = offset / INDIRECT_BLOCKS; // Index in lvl 1
      off_t doubly_off = offset % INDIRECT_BLOCKS; // Index in lvl 2
      block_sector_t arr1[INDIRECT_BLOCKS]; // Indirect block (1st lvl)
      block_sector_t arr2[INDIRECT_BLOCKS]; // Direct block (2nd lvl)
      block_read (fs_device, inode->data.indirect_block, arr1);
      block_read (fs_device, arr1[doubly_block], arr2);
      return arr2[doubly_off];
    }
  }
  else
    return -1;
}

bool
inode_get_sectors (struct inode_disk *inode, off_t size) 
{
  if (size <= 0) return size == 0;

  off_t curr_pos = inode->length > 0 ? bytes_to_sectors (inode->length) : 0;
  int sectors = bytes_to_sectors (size);
  block_sector_t new_sectors[sectors];
  int num_sectors;

  bool requested_indirect1 = false;
  bool requested_indirect2 = false;

  block_sector_t blocks2[INDIRECT_BLOCKS];
  int b2_len = 0;

  if (curr_pos <= DIRECT_BLOCKS && curr_pos + sectors > DIRECT_BLOCKS)
  {
    if (!free_map_allocate (1, &inode->indirect_block))
      return false;
    else
      requested_indirect1 = true;
  }

  off_t tmp = DIRECT_BLOCKS + INDIRECT_BLOCKS;
  if (curr_pos + sectors > tmp)
  {
    if (curr_pos <= tmp)
    {
      if (!free_map_allocate (1, &inode->indirect_block))
      {
        if (requested_indirect1)
          free_map_release (inode->indirect_block, 1);

        return false;
      }
      else 
        requested_indirect2 = true;

      b2_len = curr_pos + sectors - tmp;
      b2_len += DIV_ROUND_UP (b2_len, INDIRECT_BLOCKS);
    }
    else
    {
      b2_len = size + curr_pos - tmp - INDIRECT_BLOCKS;
      if (sectors > b2_len)
        b2_len += DIV_ROUND_UP (b2_len, INDIRECT_BLOCKS);
      else
        b2_len = 0;
    }
  }

  for (int i = 0; i < b2_len; i++)
  {
    if (!free_map_allocate (1, &blocks2[i]))
    {
      for (int j = 0; j < i; j++)
        free_map_release (blocks2[j], 1);

      if (requested_indirect1)
        free_map_release (inode->indirect_block, 1);

      if (requested_indirect2)
        free_map_release (inode->doubly_indirect_block, 1);

      return false;
    }
  }

  for (int i = 0; i < sectors; i++)
  {
    if (!free_map_allocate(1, &new_sectors[i]))
    {
      for (int j = 0; j < i; j++)
        free_map_release (new_sectors[j], 1);
      
      return false;
    }
  }

  static char zeros[BLOCK_SECTOR_SIZE];
  for (int i = 0; i < sectors; i++)
    block_write (fs_device, new_sectors[i], zeros);

  for (int i = 0; i < b2_len; i++)
    block_write (fs_device, blocks2[i], zeros);
  

  /* Level 0 */
  if (curr_pos < DIRECT_BLOCKS && sectors > 0) 
  {
    num_sectors = DIRECT_BLOCKS - curr_pos;
    num_sectors = sectors < num_sectors ? sectors : num_sectors;

    for (int i = curr_pos; i < num_sectors; i++)
      inode->direct_blocks[i] = new_sectors[i];

    sectors -= num_sectors;
    curr_pos += num_sectors;
  }

  /* Level 1 */
  if (curr_pos < DIRECT_BLOCKS + INDIRECT_BLOCKS && sectors > 0)
  {
    block_sector_t arr[INDIRECT_BLOCKS];

    if (!requested_indirect1)
      block_read (fs_device, inode->indirect_block, arr);

    num_sectors = INDIRECT_BLOCKS + DIRECT_BLOCKS - curr_pos;
    num_sectors = sectors < num_sectors ? sectors : num_sectors;

    for (int i = curr_pos; i < num_sectors; i++)
      arr[i-DIRECT_BLOCKS] = new_sectors[i];

    block_write (fs_device, inode->indirect_block, arr);
    sectors -= num_sectors;
    curr_pos += num_sectors;
  }

  /* Level 2 */
  if (sectors > 0)
  {
    block_sector_t arr1[INDIRECT_BLOCKS];

    off_t cr_shift = curr_pos - INDIRECT_BLOCKS - DIRECT_BLOCKS;
    off_t cr_block = cr_shift / INDIRECT_BLOCKS;
    off_t cr_off   = cr_shift % INDIRECT_BLOCKS;

    if (!requested_indirect2)
    {
      block_sector_t arr2[INDIRECT_BLOCKS];
      block_read (fs_device, inode->doubly_indirect_block, arr1);
      block_read (fs_device, arr1[cr_block], arr2);

      num_sectors = INDIRECT_BLOCKS - cr_off - 1;
      num_sectors = sectors < num_sectors ? sectors : num_sectors;
      
      for (int i = cr_off+1; i < num_sectors; i++)
        arr2[i] = new_sectors[curr_pos+i-1];

      block_write (fs_device, arr1[cr_block], arr2);
      sectors -= num_sectors;
      curr_pos += num_sectors;
      cr_block++;
    }

    for (int i = cr_block; sectors > 0 && i < INDIRECT_BLOCKS; i++)
    {
      num_sectors = sectors < INDIRECT_BLOCKS ? sectors: INDIRECT_BLOCKS;
      block_sector_t arr2[INDIRECT_BLOCKS];
      arr1[i] = blocks2[b2_len--];

      for (int j = 0; j < INDIRECT_BLOCKS && b2_len >= 0; j++)
        arr2[j] = blocks2[b2_len--];

      block_write (fs_device, arr1[i], arr2);

      sectors -= num_sectors;
      curr_pos += num_sectors;
    }

    block_write (fs_device, inode->doubly_indirect_block, arr1);
  }
 
  inode->length += size;
  return true;
}

/* List of open inodes, so that opening a single inode twice
   returns the same `struct inode'. */
static struct list open_inodes;

/* Initializes the inode module. */
void
inode_init (void)
{
  list_init (&open_inodes);
}

/* Initializes an inode with LENGTH bytes of data and
   writes the new inode to sector SECTOR on the file system
   device.
   Returns true if successful.
   Returns false if memory or disk allocation fails. */
bool
inode_create (block_sector_t sector, off_t length)
{
  struct inode_disk *disk_inode = NULL;
  bool success = false;

  ASSERT (length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  ASSERT (sizeof *disk_inode == BLOCK_SECTOR_SIZE); // FIX THIS

  disk_inode = calloc (1, sizeof *disk_inode);
  if (disk_inode != NULL)
    {
      // size_t sectors = bytes_to_sectors (length);
      /* Disk_inode->length = 0; */
      disk_inode->magic = INODE_MAGIC;
      if (inode_get_sectors (disk_inode, length)) 
        {
          block_write (fs_device, sector, disk_inode);
          // if (sectors > 0)
          //   {
          //     static char zeros[BLOCK_SECTOR_SIZE];
          //     size_t i;
              
          //     for (i = 0; i < sectors; i++) 
          //       block_write_inode (disk_inode, zeros);
          //   }
          success = true;
        }
      free (disk_inode);
    }
  return success;
}

/* Reads an inode from SECTOR
   and returns a `struct inode' that contains it.
   Returns a null pointer if memory allocation fails. */
struct inode *
inode_open (block_sector_t sector)
{
  struct list_elem *e;
  struct inode *inode;

  /* Check whether this inode is already open. */
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e))
    {
      inode = list_entry (e, struct inode, elem);
      if (inode->sector == sector)
        {
          inode_reopen (inode);
          return inode;
        }
    }

  /* Allocate memory. */
  inode = malloc (sizeof *inode);
  if (inode == NULL)
    return NULL;

  /* Initialize. */
  list_push_front (&open_inodes, &inode->elem);
  inode->sector = sector;
  inode->open_cnt = 1;
  inode->deny_write_cnt = 0;
  inode->removed = false;
  block_read (fs_device, inode->sector, &inode->data);
  return inode;
}

/* Reopens and returns INODE. */
struct inode *
inode_reopen (struct inode *inode)
{
  if (inode != NULL)
    inode->open_cnt++;
  return inode;
}

/* Returns INODE's inode number. */
block_sector_t
inode_get_inumber (const struct inode *inode)
{
  return inode->sector;
}

/* Closes INODE and writes it to disk.
   If this was the last reference to INODE, frees its memory.
   If INODE was also a removed inode, frees its blocks. */
void
inode_close (struct inode *inode)
{
  /* Ignore null pointer. */
  if (inode == NULL)
    return;

  /* Release resources if this was the last opener. */
  if (--inode->open_cnt == 0)
    {
      /* Remove from inode list and release lock. */
      list_remove (&inode->elem);

      /* Deallocate blocks if removed. */
      if (inode->removed)
        {
          free_map_release (inode->sector, 1);
          for (off_t b = 0; b < inode->data.length; b += BLOCK_SECTOR_SIZE)
            free_map_release (byte_to_sector (inode, b), 1);
        }

      free (inode);
    }
}

/* Marks INODE to be deleted when it is closed by the last caller who
   has it open. */
void
inode_remove (struct inode *inode)
{
  ASSERT (inode != NULL);
  inode->removed = true;
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t
inode_read_at (struct inode *inode, void *buffer_, off_t size, off_t offset)
{
  uint8_t *buffer = buffer_;
  off_t bytes_read = 0;
  uint8_t *bounce = NULL;

  while (size > 0)
    {
      /* Disk sector to read, starting byte offset within sector. */
      block_sector_t sector_idx = byte_to_sector (inode, offset);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually copy out of this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break;

      if (sector_ofs == 0 && chunk_size == BLOCK_SECTOR_SIZE)
        {
          /* Read full sector directly into caller's buffer. */
          block_read (fs_device, sector_idx, buffer + bytes_read);
        }
      else
        {
          /* Read sector into bounce buffer, then partially copy
             into caller's buffer. */
          if (bounce == NULL)
            {
              bounce = malloc (BLOCK_SECTOR_SIZE);
              if (bounce == NULL)
                break;
            }
          block_read (fs_device, sector_idx, bounce);
          memcpy (buffer + bytes_read, bounce + sector_ofs, chunk_size);
        }

      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_read += chunk_size;
    }
  free (bounce);

  return bytes_read;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
   Returns the number of bytes actually written, which may be
   less than SIZE if end of file is reached or an error occurs.
   (Normally a write at end of file would extend the inode, but
   growth is not yet implemented.) */
off_t
inode_write_at (struct inode *inode, const void *buffer_, off_t size,
                off_t offset)
{
  const uint8_t *buffer = buffer_;
  off_t bytes_written = 0;
  uint8_t *bounce = NULL;

  if (inode->deny_write_cnt)
    return 0;

  if (size + offset > inode->data.length)
    if (!inode_get_sectors (&inode->data, size + offset - inode->data.length))
      return false;

  while (size > 0)
    {
      /* Sector to write, starting byte offset within sector. */
      block_sector_t sector_idx = byte_to_sector (inode, offset);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length (inode) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually write into this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break; // FIX THIS

      if (sector_ofs == 0 && chunk_size == BLOCK_SECTOR_SIZE)
        {
          /* Write full sector directly to disk. */
          block_write (fs_device, sector_idx, buffer + bytes_written);
        }
      else
        {
          /* We need a bounce buffer. */
          if (bounce == NULL)
            {
              bounce = malloc (BLOCK_SECTOR_SIZE);
              if (bounce == NULL)
                break;
            }

          /* If the sector contains data before or after the chunk
             we're writing, then we need to read in the sector
             first.  Otherwise we start with a sector of all zeros. */
          if (sector_ofs > 0 || chunk_size < sector_left)
            block_read (fs_device, sector_idx, bounce);
          else
            memset (bounce, 0, BLOCK_SECTOR_SIZE);
          memcpy (bounce + sector_ofs, buffer + bytes_written, chunk_size);
          block_write (fs_device, sector_idx, bounce);
        }

      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_written += chunk_size;
    }
  free (bounce);

  return bytes_written;
}

/* Disables writes to INODE.
   May be called at most once per inode opener. */
void
inode_deny_write (struct inode *inode)
{
  inode->deny_write_cnt++;
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
}

/* Re-enables writes to INODE.
   Must be called once by each inode opener who has called
   inode_deny_write() on the inode, before closing the inode. */
void
inode_allow_write (struct inode *inode)
{
  ASSERT (inode->deny_write_cnt > 0);
  ASSERT (inode->deny_write_cnt <= inode->open_cnt);
  inode->deny_write_cnt--;
}

/* Returns the length, in bytes, of INODE's data. */
off_t
inode_length (const struct inode *inode)
{
  return inode->data.length;
}
