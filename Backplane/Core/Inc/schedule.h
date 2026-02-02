#pragma once
#include "main.h"

#define NUM_PUMPS 8
#define MAX_TOKENS 64
#define LINE_MAX 256
#define BUFFER_SIZE 1024

uint16_t get_next(uint16_t i);
int buffer_push(uint8_t byte);
int buffer_pop(uint8_t *out);

int split_tokens(char *s, char *argv[], int max_tokens);
int parse_i32(const char *s, int32_t *out);
int parse_f32(const char *s, float *out);
void handle_line(char *line, uint8_t card);
void usb_protocol_poll(uint8_t card);
