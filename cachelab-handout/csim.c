#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h> 
#include <stdint.h>

// Cache Line Definition
typedef struct Cache_Line {
    int valid;
    int tag;
    uint32_t last_used;
} Cache_Line;

// Global Variables
Cache_Line** cache; 
int hits, misses, evictions;

// Parse Command Line Arguments
void Parse_Args
(int argc, char** argv, int* S, int* E, int* b, int* v, char (*trace_file)[]) {
    int c;
    while ((c = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch (c) {
            case 'h':
                printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <tracefile>\n");
                printf("Options:\n");
                printf("  -h         Print this help message.\n");
                printf("  -v         Optional verbose flag.\n");
                printf("  -s <num>   Number of set index bits.\n");
                printf("  -E <num>   Number of lines per set.\n");
                printf("  -b <num>   Number of block offset bits.\n");
                printf("  -t <file>  Trace file.\n");
                exit(0);
            case 'v':
                *v = 1;
                break;
            case 's':
                *S = 1 << atoi(optarg);
                break;
            case 'E':
                *E = atoi(optarg);
                break;
            case 'b':
                *b = atoi(optarg);
                break;
            case 't':
                if (strlen(optarg) >= 128) {
                    printf("trace file name is too long\n");
                    exit(1);
                }
                strcpy(*trace_file, optarg);
                break;
            default:
                printf("Unknown argument.\n");
                printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <tracefile>\n");
                exit(1);
        }
    }
}

// Initialize Cache and return pointer to cache
Cache_Line** Init_Cache(int S, int E) {
    Cache_Line** cache = (Cache_Line**)malloc(sizeof(Cache_Line*) * S);

    int i, j;
    for (i = 0; i < S; i++) {
        cache[i] = (Cache_Line*)malloc(sizeof(Cache_Line) * E);
        for (j = 0; j < E; j++) {
            cache[i][j].valid = 0;
            cache[i][j].tag = 0;
            cache[i][j].last_used = 0;
        }
    }
    return cache;
}

// Free Cache
void Free_Cache(Cache_Line** cache, int S) {
    int i;
    for (i = 0; i < S; i++) {
        free(cache[i]);
    }
    free(cache);
}

// Update Cache with address
void Update_Cache(uint64_t address, int S, int E, int b, int v) {
    int index = (address >> b) & (S - 1);
    int tag = (address >> b) / S;

    // Update last used
    for (int i = 0; i < E; i++) {
        if (cache[index][i].valid) {
            ++cache[index][i].last_used;
        }
    }

    // Check if hit
    for (int i = 0; i < E; ++i) {
        if (cache[index][i].valid && cache[index][i].tag == tag) {
            cache[index][i].last_used = 0;
            ++hits;
            return;
        }
    }

    // Misses, check if eviction is needed
    ++misses;
    for (int i = 0; i < E; ++i) {
        if (!cache[index][i].valid) {
            cache[index][i].valid = 1;
            cache[index][i].tag = tag;
            cache[index][i].last_used = 0;
            return;
        }
    }

    // Eviction needed
    int max_last_used_index = 0;
    int max_last_used = cache[index][0].last_used;
    for (int i = 1; i < E; ++i) {
        if (cache[index][i].last_used > max_last_used) {
            max_last_used_index = i;
            max_last_used = cache[index][i].last_used;
        }
    }
    cache[index][max_last_used_index].tag = tag;
    cache[index][max_last_used_index].last_used = 0;
    ++evictions;
    
    // Update last used
    for (int i = 0; i < E; ++i) {
        ++cache[index][i].last_used;
    }

    return;
}

// Read Trace File and Update Cache
void Read_Trace
(char* trace_file, int S, int E, int b, int v) {
    FILE* trace_fp = fopen(trace_file, "r");
    if (trace_fp == NULL) {
        printf("Error: cannot open trace file\n");
        exit(1);
    }

    char line[128];
    char op;
    uint64_t address;
    int size;
    while (fgets(line, 128, trace_fp) != NULL) {
        sscanf(line, " %c %lx,%d", &op, &address, &size);
        if (v) {
            printf("%c %lx,1", op, address);
        }
        int cur_hits = hits, cur_misses = misses, cur_evictions = evictions;
        switch (op) {
            case 'I':
                continue;
            case 'L':
            case 'S':
                Update_Cache(address, S, E, b, v);
                break;
            case 'M':
                Update_Cache(address, S, E, b, v);
                Update_Cache(address, S, E, b, v);
                break;
            default:
                printf("Error: unknown operation\n");
                exit(1);
        }
        if (v) {
            if (cur_misses != misses) {
                printf(" miss");
            }
            if (cur_evictions != evictions) {
                printf(" eviction");
            }
            if (cur_hits != hits) {
                printf(" hit");
            }
            printf("\n");
        }
    }

    return;
}

int main(int argc, char* argv[])
{
    int v, S, E, b;
    v = 0;
    char trace_file[128];
    Parse_Args(argc, argv, &S, &E, &b, &v, &trace_file);
    cache = Init_Cache(S, E);
    hits = misses = evictions = 0;
    Read_Trace(trace_file, S, E, b, v);
    Free_Cache(cache, S);
    printSummary(hits, misses, evictions);
    return 0;
}
