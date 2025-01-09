#include "../header/cache.h"
#include "../header/memory.h"
#include <stdio.h>
#include <stdlib.h>
// #define CACHE
// #define MEMORY_CHECK
// #define HIT
// #define MISS
// #define DATA_MEMORY

CacheLine inst_cache[CACHE_SIZE];
CacheLine data_cache[CACHE_SIZE];
int oldest_index = 0;

void init_cache() {
    for (int i = 0; i < CACHE_SIZE / sizeof(int); i++) {
        inst_cache[i].valid = false;
        inst_cache[i].dirty = false;
        inst_cache[i].sca_bit = false;
        inst_cache[i].tag = 0;
        for (int j = 0; j < BLOCK_SIZE/sizeof(int); j++) {
            inst_cache[i].data[j] = 0;
            data_cache[i].data[j] = 0;
        }
    }
}

// 메모리 블록을 캐시로 가져오는 함수
void fetch_from_memory(int memory_address, int line_index, bool isInstruction, ExecutionStats *stats) {
    CacheLine *target_cache = isInstruction ? inst_cache : data_cache;
    int *target_memory = isInstruction ? inst_memory : memory;
    int start_index = memory_address / sizeof(int);

    // 캐시 라인 데이터 초기화
    for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++) {
        target_cache[line_index].data[j] = 0;
    }
    
    // 메모리에서 캐시 라인으로 데이터 복사
    for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++) {
        target_cache[line_index].data[i] = target_memory[start_index + i];
#ifdef CACHE
        if(isInstruction){
            // printf("[fetch_from_memory]: inst_memory[0x%x] = 0x%x\n", start_index + i, target_memory[start_index + i]);
            // printf("[fetch_from_memory]: inst_cache[%d].data[%d] = 0x%x\n",line_index, i, target_cache[line_index].data[i]);
        }else{
            // printf("memory_address? 0x%x, start_index? 0x%x\n",memory_address, start_index);
            printf("[fetch_from_memory]: data_memory[0x%x] = 0x%x\n", start_index + i, target_memory[start_index + i]);
            printf("[fetch_from_memory]: data_cache[%d].data[%d] = 0x%x\n",line_index, i, target_cache[line_index].data[i]);
            // printf("cache miss나서 memory의 data block을 cache애 fetch완료\n");

        }
#endif
    }
    target_cache[line_index].dirty = false;
    target_cache[line_index].sca_bit = true;
    stats->memory_access_count++;
}



int find_victim(bool isInstruction) {
    CacheLine *target_cache = isInstruction ? inst_cache : data_cache;
    int checked = 0;
    while (checked < CACHE_SIZE) {
        if (target_cache[oldest_index].valid && target_cache[oldest_index].sca_bit) {
            target_cache[oldest_index].sca_bit = false;
            oldest_index = (oldest_index + 1) % CACHE_SIZE;
        } else {
            return oldest_index;
        }
        checked++;
    }
    return oldest_index;  // Return oldest if all lines had a second chance
}

int read_from_cache(int address, bool isInstruction, ExecutionStats *stats){
    CacheLine *target_cache = isInstruction ? inst_cache : data_cache;

    uint32_t offset_mask = (1 << OFFSET_BITS) - 1;  // 6비트 마스크
    uint32_t index_mask = (1 << INDEX_BITS) - 1;    // 8비트 마스크

    unsigned int offset = address & offset_mask;
    unsigned int line_index = (address >> OFFSET_BITS) & index_mask;
    unsigned int tag = address >> (OFFSET_BITS + INDEX_BITS);
    
    if (line_index >= CACHE_SIZE) {
        printf("Cache index out of bounds: %u\n", line_index);
        return -1;
    }

    if (target_cache[line_index].valid && target_cache[line_index].tag == tag) {// cache HIT
#ifdef HIT
        if(isInstruction){
            printf("inst_cache HIT\n");
        }else{
            printf("data_cache HIT\n");
        }
        printf("[HIT] target_csache[%d].data[0x%lx] = 0x%x\n",line_index,offset / sizeof(int),target_cache[line_index].data[offset / sizeof(int)]);
#endif
        if(isInstruction){
            stats->inst_cache_hit_count++;
        }else{
            stats->data_cache_hit_count++;
        }
        target_cache[line_index].sca_bit = true;
        return target_cache[line_index].data[offset / sizeof(int)];
    } else { // cache MISS
#ifdef MISS
        if(isInstruction){
            printf("inst_cache MISS\n");
        }else{
            printf("data_cache MISS\n");
        }
#endif
        int victim_index = find_victim(isInstruction);
        if (target_cache[victim_index].valid && target_cache[victim_index].dirty) {
            flush_line_to_memory(victim_index, isInstruction, stats);
        }
        if(isInstruction){
            fetch_from_memory(address & ~(BLOCK_SIZE - 1), victim_index, true, stats);
            stats->inst_cache_miss_count++;
        }else{
            fetch_from_memory(address & ~(BLOCK_SIZE - 1), victim_index, false, stats);
            stats->data_cache_miss_count++;
        }
        target_cache[victim_index].tag = tag;
        target_cache[victim_index].valid = true;
        target_cache[victim_index].sca_bit = true;
#ifdef DATA_MEMORY
        if(isInstruction){
            printf("[read_from_cache]: inst_cache[%d].data[%lu] = 0x%x\n",victim_index, offset / sizeof(int), target_cache[victim_index].data[offset / sizeof(int)]);
        }else{
            printf("[read_from_cache]: data_cache[%d].data[%lu] = 0x%x\n",victim_index, offset / sizeof(int), target_cache[victim_index].data[offset / sizeof(int)]);
        }
        printf("[MISS] target_cache[%d].data[0x%lx] = 0x%x\n",victim_index,offset / sizeof(int),target_cache[victim_index].data[offset / sizeof(int)]);
#endif
        return target_cache[victim_index].data[offset / sizeof(int)];
    }
}


void write_to_cache(int address, int data, bool isInstruction, ExecutionStats *stats) {
    CacheLine *target_cache = isInstruction ? inst_cache : data_cache;

    uint32_t offset_mask = (1 << OFFSET_BITS) - 1;  // 6비트 마스크
    uint32_t index_mask = (1 << INDEX_BITS) - 1;    // 8비트 마스크

    unsigned int offset = address & offset_mask;
    unsigned int line_index = (address >> OFFSET_BITS) & index_mask;
    unsigned int tag = address >> (OFFSET_BITS + INDEX_BITS);

    if (line_index >= CACHE_SIZE) {
        printf("Error: line_index out of bounds\n");
        return;
    }

    if (target_cache[line_index].valid && target_cache[line_index].tag == tag) {
        // Cache Hit
        if(isInstruction){
            stats->inst_cache_hit_count++;
        }else{
            stats->data_cache_hit_count++;
        }
        target_cache[line_index].data[offset / sizeof(int)] = data;
        target_cache[line_index].dirty = true;
    } else {
        // Cache Miss
        if(isInstruction){
            stats->inst_cache_miss_count++;
        }else{
            stats->data_cache_miss_count++;
        }
        if (target_cache[line_index].valid && target_cache[line_index].dirty) {
            flush_line_to_memory(line_index, isInstruction, stats);
        }
        fetch_from_memory(address & ~(BLOCK_SIZE - 1), line_index, isInstruction, stats);
        target_cache[line_index].data[offset / sizeof(int)] = data;
        target_cache[line_index].tag = tag;
        target_cache[line_index].valid = true;
        target_cache[line_index].dirty = true;
    }
    target_cache[line_index].sca_bit = true;
}


void flush_line_to_memory(int line_index, bool isInstruction, ExecutionStats *stats) {
    CacheLine *target_cache = isInstruction ? inst_cache : data_cache;
    int memory_address = (target_cache[line_index].tag << (INDEX_BITS + OFFSET_BITS)) | (line_index << OFFSET_BITS);
    int memory_index = memory_address / sizeof(int);
#ifdef MEMORY_CHECK
    printf("Flushing cache line %d to memory address 0x%x\n", line_index, memory_address);
#endif
    for (int i = 0; i < BLOCK_SIZE / sizeof(int); i++) {
        memory[memory_index + i] = target_cache[line_index].data[i];
    }
    target_cache[line_index].dirty = false;
    stats->memory_access_count++;
}

void flush_cache(bool isInstruction) {
    CacheLine *target_cache = isInstruction ? inst_cache : data_cache;
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (target_cache[i].valid && target_cache[i].dirty) {
            flush_line_to_memory(i, isInstruction, &stats);
            target_cache[i].dirty = false;
        }
    }
}

bool cache_full() {
    CacheLine *data_cache;
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!data_cache[i].valid) {
            return false;
        }
    }
    return true;
}