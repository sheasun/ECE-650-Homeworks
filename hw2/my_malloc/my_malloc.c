#include"my_malloc.h"

#include<pthread.h>


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//create new requested size space
block_t * create_space(size_t size){
    size_t requested = size + sizeof(block_t);
    block_t * new = sbrk(requested);
    new->size = size;
    new->next = NULL;
    new->prev = NULL;

    return new;
}

block_t * create_space_nolock(size_t size) {
    size_t requested = size + sizeof(block_t);
    pthread_mutex_lock(&lock);
    block_t * new = sbrk(requested);
    pthread_mutex_unlock(&lock);
    new->size = size;
    new->next = NULL;
    new->prev = NULL;

    return new;
}

//when the size of block is greater than requested size, split the current block
block_t * split_block(block_t * curr, size_t size, int no) {
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
        if(no == 0) {
            head = split;
        }
        else {
            head_no = split;
        }
    }
    if(split->next != NULL) {
        split->next->prev = split;
    }
    else {
        if(no == 0) {
            tail = split;
        }
        else {
            tail_no = split;
        }
    }

    return (void*)curr + sizeof(block_t);
}

block_t * update_block(block_t * curr, size_t size, int no) {
            if(curr->prev != NULL) {
                curr->prev->next = curr->next;
            }
            else {
                if(no == 0) {
                    head = curr->next;
                }
                else {
                    head_no = curr->next;
                }
            }
            if(curr->next != NULL) {
                curr->next->prev = curr->prev;
            }
            else {
                if(no == 0) {
                    tail = curr->prev;
                }
                else {
                    tail_no = curr->prev;
                }
            }
            curr->prev = NULL;
            curr->next = NULL;
            return (void*)curr + sizeof(block_t);
}

//best fit malloc
void *ts_malloc_lock(size_t size){
    pthread_mutex_lock(&lock);
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
        void * ans = split_block(best, size, 0);
        pthread_mutex_unlock(&lock);
        return ans;
    }
    else if(best->size == size) {
        void * ans = update_block(curr, size, 0);
        pthread_mutex_unlock(&lock);
        return ans;
    }
    }
    //else, create new space
    void * ans = (void*)create_space(size) + sizeof(block_t);
    pthread_mutex_unlock(&lock);
    return ans;
}

void ts_free_lock(void *ptr) {
    if(ptr == NULL) {
        return;
    }
    pthread_mutex_lock(&lock);
    block_t *curr = (block_t*)(ptr - sizeof(block_t));
    block_t * search = head;
    if(search == NULL) {
        head = tail = curr;
        pthread_mutex_unlock(&lock);
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
    pthread_mutex_unlock(&lock);
}

//
void *ts_malloc_nolock(size_t size) {
    block_t * curr = head_no;
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
        void * ans = split_block(best, size, 1);
        return ans;
    }
    else if(best->size == size) {
        void * ans = update_block(curr, size, 1);
        return ans;
    }
    }
    //else, create new space
    void * ans = (void*)create_space_nolock(size) + sizeof(block_t);
    return ans;
}

void ts_free_nolock(void *ptr) {
    if(ptr == NULL) {
        return;
    }
    block_t *curr = (block_t*)(ptr - sizeof(block_t));
    block_t * search = head_no;
    if(search == NULL) {
        head_no = tail_no = curr;
        return;
    }
    //curr on the left of the head
    else if(curr < head_no) {
        curr->next = head_no;
        head_no->prev = curr;
        head_no = curr;
    }
    //curr on the right of the tail
    else if(tail_no < curr) {
        tail_no->next = curr;
        curr->prev = tail_no;
        tail_no = curr;
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

    merge_blocks_nolock(curr);
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

void merge_blocks_nolock(block_t * curr) {
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
                tail_no = prev;
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
                tail_no = curr;
                }
            }
        }
        }
