#include <stdio.h>
#include <stdlib.h>

#if 0
int main()
{
    printf("Hello world!\n");
    return 0;
}
#endif

#include "lfs.h"


static unsigned char BlockCache[1024*32] = {0xff};


static int block_read(const struct lfs_config *c, lfs_block_t block,
                      lfs_off_t off, void *buffer, lfs_size_t size)
{
  memcpy( (uint8_t *)buffer,
         &BlockCache[block * c->block_size + off], (uint32_t)size);
  return 0;
}

static int block_prog(const struct lfs_config *c, lfs_block_t block,
                      lfs_off_t off, const void *buffer, lfs_size_t size)
{
  memcpy(&BlockCache[block * c->block_size + off],
                (uint8_t *)buffer, (uint32_t)size);
  return 0;
}

static int block_erase(const struct lfs_config *c, lfs_block_t block)
{
  memset(&BlockCache[block * c->block_size],
                0xFF, c->block_size);
  return 0;
}

static int block_sync(const struct lfs_config *config)
{

    FILE * fp = NULL;
    fp = fopen("block.bin", "wb");
    fwrite(BlockCache, sizeof(BlockCache), 1, fp);
    fclose(fp);

    return 0;
}

// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;

// configuration of the filesystem is provided by this struct
const struct lfs_config cfg = {
    // block device operations
    .read  = block_read,
    .prog  = block_prog,
    .erase = block_erase,
    .sync  = block_sync,

    // block device configuration
    .read_size = 16,
    .prog_size = 16,
    .block_size = 1024,
    .block_count = 128,
    .lookahead = 128,
};

// entry point
int main(void) {
    // mount the filesystem
    int err = lfs_mount(&lfs, &cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    // read current count
    uint32_t boot_count = 5;
    lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs, &file);

    // release any resources we were using
    lfs_unmount(&lfs);

    // print the boot count
    printf("boot_count: %d\n", boot_count);
}
