#ifndef __SAFE_QUEUE_H__
#define __SAFE_QUEUE_H__

#include <LPC210x.H>                       /* LPC210x definitions */

typedef struct {
    size_t head;
    size_t tail;
    size_t size;
    void** data;
} queue_t;

void* queue_read(queue_t *queue);

int queue_write(queue_t *queue, void* handle);

#endif // __SAFE_QUEUE_H__
