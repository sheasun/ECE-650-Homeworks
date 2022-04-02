#include"my_malloc.h"

unsigned long data_segment_size = 0;

block_t * head = NULL;
block_t * tail = NULL;


//when the size of block is greater than requested size, split the current block
block_t * split_block(block_t * curr, size_t size) {
    void * temp = (void*)curr + sizeof(block_t) + size;
    block_t * split = (block_t*)temp;
    size_t subsize = curr->size - sizeof(block_t) - size;
    curr->size = size;
    //update the size of curr
    split->size = subsize;
    split->next = curr->next;
    split->prev = curr->prev;

    curr->next = NULL;
    curr->prev = NULL;

    if(split->prev != NULL) {
        split->prev->next = split;
    }
    else {
        head = split;
    }
    if(split->next != NULL) {
        split->next->prev = split;
    }
    else {
        tail = split;
    }

    return (void*)curr + sizeof(block_t);
}

//create new requested size space
block_t * create_space(size_t size){
    size_t requested = size + sizeof(block_t);
    block_t * new = sbrk(requested);
    new->size = size;
    new->next = NULL;
    new->prev = NULL;

    data_segment_size += requested;
    return new;
}

block_t * update_block(block_t * curr, size_t size) {
            if(curr->prev != NULL) {
                curr->prev->next = curr->next;
            }
            else {
                head = curr->next;
            }
            if(curr->next != NULL) {
                curr->next->prev = curr->prev;
            }
            else {
                tail = curr->prev;
            }
            curr->prev = NULL;
            curr->next = NULL;
            return (void*)curr + sizeof(block_t);
}

//first fit malloc
void *ff_malloc(size_t size) {
    if(size == 0) {
        return NULL;
    }
    block_t * curr = head;
    while(curr != NULL) {
        if(curr->size > size + sizeof(block_t)) {
            void * ans = split_block(curr, size);
            return ans;
        }
        else if(curr->size == size) {
            void * ans = update_block(curr, size);
            return ans;
            }
        if(curr->next != NULL) {
            curr = curr->next;
        }
        else {
            break;
        }
    }
    void * ans = (void*)create_space(size) + sizeof(block_t);
    return ans;
}

//best fit malloc
void *bf_malloc(size_t size){
    block_t * curr = head;
    block_t * best = NULL;
    while(curr != NULL) {
        if(curr->size == size) {
            best = curr;
            break;
        }
        else if(curr->size > sizeof(block_t) + size) {
            if(best == NULL || best->size > curr->size) {
                best = curr;
            }
        }
        if(curr->next != NULL) {
            curr = curr->next;
        }
        else {
            break;
        }
    }
    //if found the best fit block, split it
    if(best != NULL) {
    if(best->size > size + sizeof(block_t)) {
        void * ans = split_block(best, size);
        return ans;
    }
    else if(best->size == size) {
        void * ans = update_block(curr, size);
        return ans;
    }
    }
    //else, create new space
    void * ans = (void*)create_space(size) + sizeof(block_t);
    return ans;
}

unsigned long get_data_segment_size(){
    return data_segment_size;
}

unsigned long get_data_segment_free_space_size(){
    unsigned long free = 0;
    block_t * curr = head;
    while(curr != NULL) {
        free +=  curr->size + sizeof(block_t);
        curr = curr->next;
    }
  return free;
}

void merge_blocks(block_t * curr) {
    block_t * prev = curr->prev;
    if(curr->prev != NULL) {
        void * end = (void*)prev + prev->size + sizeof(block_t);
        if(end == (void*)curr) {
            prev->size += curr->size + sizeof(block_t);
            prev->next = curr->next;
            if(curr->next != NULL) {
                curr->next->prev = prev;
            }
            else {
                tail = prev;
            }
            curr = prev;
        }
    }
    block_t * next = curr->next;
    if(curr->next != NULL) {
        void * end = (void*)curr + curr->size + sizeof(block_t);
        if(end == (void*)next) {
            curr->size += next->size + sizeof(block_t);
            curr->next = next->next;
            if(curr->next != NULL) {
                curr->next->prev = curr;
            }
            else {
                tail = curr;
                }
            }
        }
        }

void ff_free(void *ptr) {
    if(ptr == NULL) {
        return;
    }
    block_t *curr = (block_t*)(ptr - sizeof(block_t));
    block_t * search = head;
    if(search == NULL) {
        head = tail = curr;
        return;
    }
    //curr on the left of the head
    else if(curr < head) {
        curr->next = head;
        head->prev = curr;
        head = curr;
    }
    //curr on the right of the tail
    else if(tail < curr) {
        tail->next = curr;
        curr->prev = tail;
        tail = curr;
    }
    //curr is in the linked list
    else {
        while(search < curr) {
            search = search->next;
        }
        curr->next = search;
        curr->prev = search->prev;
        search->prev = curr;
        curr->prev->next = curr;
    }

    merge_blocks(curr);
}

void bf_free(void *ptr) {
    ff_free(ptr);
}