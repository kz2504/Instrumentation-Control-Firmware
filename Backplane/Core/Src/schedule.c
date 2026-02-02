#include "main.h"
#include "schedule.h"

#define BUFFER_SIZE 1024

uint16_t head = 0;
uint16_t tail = 0;

uint8_t buffer[BUFFER_SIZE];

uint16_t get_next(uint16_t i) {
    return (uint16_t)((i + 1u) % BUFFER_SIZE);
}

int buffer_push(uint8_t byte) { //1 if push success, 0 if buffer full
    uint16_t next = get_next(head);
    if (next == tail) {
    	return 0;
    }
    buffer[head] = byte;
    head = next;
    return 1;
}

int buffer_pop(uint8_t *out) { //1 if pop success, 0 if buffer empty
	if (tail == head){
		return 0;
	}
	*out = buffer[tail];
	tail = get_next(tail);
	return 1;
}

