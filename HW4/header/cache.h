#ifndef CACHE_H
#define CACHE_H

#include "executionStats.h"
#include <stdbool.h>

#define CACHE_SIZE 256 // 캐시 라인 수 2^8
#define BLOCK_SIZE 64 // 각 캐시 라인의 크기 (바이트)
#define ADDRESS_BITS 32 // 주소 크기 (비트)
#define TAG_BITS    18
#define INDEX_BITS  8
#define OFFSET_BITS 6


typedef struct {
    bool valid;
    unsigned int tag; // 태그: 메모리 주소의 상위 비트, 캐시 라인이 해당 메모리 주소와 연관되어 있는지 확인
    int data[BLOCK_SIZE];
    bool dirty; // 수정여부
    bool sca_bit;
} CacheLine;

extern CacheLine inst_cache[CACHE_SIZE];// for 16KB cache
extern CacheLine data_cache[CACHE_SIZE];
ExecutionStats stats;

void init_cache();
void fetch_from_memory(int memory_address, int line_index, bool isInstruction, ExecutionStats *stats);
int find_victim(bool isInstruction);
int read_from_cache(int address, bool isInstruction, ExecutionStats *stats);
void write_to_cache(int address, int data, bool isInstruction, ExecutionStats *stats);
void flush_cache(bool isInstruction);
void flush_line_to_memory(int line_index, bool isInstruction, ExecutionStats *stats);
bool cache_full();

#endif