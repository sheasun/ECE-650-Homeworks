#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

struct block_tag {
    size_t size;
    struct block_tag * next;
    struct block_tag * prev;
};

typedef struct block_tag block_t;

//Thread Safe malloc/ free: locking version
void *ts_malloc_lock(size_t size);
void ts_free_lock(void *ptr);

//Thread Safe malloc/ free: non-locking version
void *ts_malloc_nolock(size_t size);
void ts_free_nolock(void *ptr);

//create new space: locking version
block_t * create_space(size_t size);

//create new space: non-locking version
block_t * create_space_nolock(size_t size);

block_t * split_block(block_t * curr, size_t size, int no);
//block_t * split_block_nolock(block_t * curr, size_t size);

void merge_blocks(block_t * curr);
void merge_blocks_nolock(block_t * curr);

block_t * head = NULL;
block_t * tail = NULL;

__thread block_t * head_no = NULL;
__thread block_t * tail_no = NULL;
