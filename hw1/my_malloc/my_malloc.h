#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stddef.h>

struct block_tag {
    size_t size;
    struct block_tag * next;
    struct block_tag * prev;
};

typedef struct block_tag block_t;


//first fit malloc/free
void *ff_malloc(size_t size);
void ff_free(void *ptr);

//best fit malloc/free
void *bf_malloc(size_t size);
void bf_free(void *ptr);

unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size(); //in bytes

//create new requested size space
block_t * create_space(size_t size);

//when the size of block is larger than requested size, split the current block
block_t * split_block(block_t * curr, size_t size);

block_t * update_block(block_t * curr, size_t size);

//find the most suitable block in bf_malloc
block_t * find_best(block_t * curr, size_t size);

void merge_blocks(block_t * ptr);
