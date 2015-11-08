
#include <ctype.h>
#include "memory_trace_options.h"

#define IS_LEAF 0
#define IS_INNER 1
#ifndef Assignment4_memory_page_h
#define Assignment4_memory_page_h

typedef struct PageTable{
    unsigned int *mask_array;
    unsigned int *shift_array;
    int *entry_counts;
    int table_depth;
    unsigned int search_count;
    unsigned int frame_asn;
    unsigned int hits;
    unsigned int miss;
    unsigned int offsetBits;
    unsigned int pageTableMask;
    unsigned long int treeSize;
    Memory_setting *settings;
    
    struct Level *root;
}PageTable;

typedef struct Map{
    unsigned int pageNumber;
    unsigned int frameNumber;
}Map;
typedef struct Level{
    int depth;
    int node_type;
    PageTable *page_table;
    Map **map;
    struct Level **next_level;
}Level;
unsigned int *getMask(unsigned int *level_bits, unsigned int *shiftArray,int length);
unsigned int *getShift(unsigned int *level_bits,int length);
int *getEntryCounts(unsigned int *level_bits,int length);

unsigned int logicalToPage(unsigned int log_Addr,unsigned int level_mask,unsigned shift);
PageTable *initPageTable(unsigned int *mask_set,int table_size,unsigned int offset,Memory_setting *msettings);
Level *initLevel(PageTable *pagetable,int current_depth);
Map *initMap(unsigned int page_number,unsigned int frame_number);
Map *pageLookup(PageTable *pagetable, unsigned int log_addr);
int pageInsertTable(PageTable *pagetable,unsigned int log_addr,unsigned int frame_number);
int pageInsert_Level(Level *level,unsigned int log_addr, unsigned int frame_number);
unsigned int getPageMask(unsigned int *mask_ary,int length);
unsigned int getPhysAddr(unsigned int log_addr,PageTable *table,Map *map);
void writeTranslation (FILE *outfile,unsigned int trans_a,unsigned int trans_b);
#endif
