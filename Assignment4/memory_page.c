/*
    I decided to place all the pagetable code in one file since all the functionality acted
    on a pagetable struct however in retrospect this may have been a bad idea.
    Gist of this is that a PageTable struct is initialized through initPageTable with the required 
    settings(bit mask array, #levels, offset bits, and settings). these can be generated through
    helper methods in this file. Insert of new page is done throught the pageinserttable function
    which is a wrapper for the level type function. the pageinsert consecutively searches for the
    and creates a new mapping if needed in case of page miss.
 
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "memory_page.h"
#include "memory_trace_options.h"


/*
 Initialize the PageTable structure with default and passed in table settings
 */
PageTable *initPageTable(unsigned int *mask_set,int table_size,unsigned int offset,Memory_setting *msettings){
    
    PageTable *pagetable;
    pagetable = (PageTable *) malloc(sizeof(PageTable));
    if(!pagetable){
        fprintf(stderr, "Error Allocating Page Table");
        exit(EXIT_FAILURE);
    }
    pagetable->shift_array = getShift(mask_set, table_size);
    pagetable->mask_array = getMask(mask_set,pagetable->shift_array,table_size);
    pagetable->entry_counts = getEntryCounts(mask_set, table_size);
    pagetable->pageTableMask = getPageMask(pagetable->mask_array,table_size);
    pagetable->table_depth = table_size;
    pagetable->offsetBits = offset;
    pagetable->frame_asn = 0;
    pagetable->search_count = 0;
    pagetable->miss = 0;
    pagetable->hits = 0;
    pagetable->treeSize = sizeof(PageTable);
    pagetable->settings = msettings;
    
    pagetable->root = initLevel(pagetable, 0);
    return pagetable;
}

/*
    Allocates a new level based on the type of inner level or leaf.
    Grabs entry count number from pagetable structure in order to populate
    the inner array of pointers to maps or next levels.
    returns reference to newly allocated level.
*/

Level *initLevel(PageTable *pagetable,int current_depth){
    Level *lvl_ptr = malloc(sizeof(struct Level));
    //Checking for Errors
    if(!lvl_ptr){
        fprintf(stderr, "Error Allocating Level\n");
        exit(EXIT_FAILURE);
    }
    //Grab info to populate Level
    int index_size = pagetable->entry_counts[current_depth];
    lvl_ptr->depth = current_depth;
    lvl_ptr->page_table = pagetable;
    lvl_ptr->node_type = current_depth == (pagetable->table_depth)-1 ? IS_LEAF:IS_INNER;
    
    /*
        If the current level being allocated matches the total number levels - 1,
        then the current level is the map array is pointed to an allocated array.
        Example, level 0 would be a leaf if the total number of levels is 1

     */
    switch (lvl_ptr->node_type) {
        case IS_LEAF:
            if(!(lvl_ptr->map = (Map **) calloc(index_size,sizeof(Map*)))){
                fprintf(stderr, "Error Allocating Leaf Map\n");
                exit(EXIT_FAILURE);
            };
            lvl_ptr->page_table->treeSize+=(index_size * sizeof(Map*));

            break;
        case IS_INNER:
            if(!(lvl_ptr->next_level = (Level **) calloc(index_size, sizeof(Level*)))){
                fprintf(stderr, "Error Allocating Inner Level Array\n");
                exit(EXIT_FAILURE);
            };
            lvl_ptr->page_table->treeSize+=(index_size * sizeof(Level*));
            break;
        default:
            printf("Level type error %d\n",current_depth);
            break;
    }
    lvl_ptr->page_table->treeSize+=sizeof(struct Level);
    return lvl_ptr;
}
int pageInsertTable(PageTable *pagetable,unsigned int log_addr,unsigned int frame_number){
    if (!pagetable) {
        fprintf(stderr, "Error writing page. no table allocated\n");
        exit(EXIT_FAILURE);
    }
    pageInsert_Level(pagetable->root, log_addr, pagetable->frame_asn);
    return 0;
}
/*
    Main functionality of the program. this is a recursive function that searches for a
    page in the tree and allocates space for one if not found. functionality differs
    if it is a leaf or inner level type. Recursive search terminates when a leaf is found.
    once a lead is found, it attempts to locate the page. if a miss, allocate a mapping
    and increment the frames assigned. a further if statement delegates printing of
    the physical address translation.
 */
int pageInsert_Level(Level *level,unsigned int log_addr, unsigned int frame_number){
    
    PageTable *pagetable;
    unsigned int pageNumber;
    unsigned int levelMask;
    unsigned int levelShift;
    unsigned int pageIndex;
    unsigned int pageMask;
    unsigned int offset;
    int type;
    int levelDepth;
    
    if (!level) {
        fprintf(stderr, "Error Inserting Address 0x%x. No allocated Level\n",log_addr);
        exit(EXIT_FAILURE);
    }
    
    /*
     Store page table properties.
     this might be troublesome if the table is long since
     these are stack variable and this is a recursive function.
     however, trees should really only at most 3 to 4 nodes deep.
     */
    pagetable = level->page_table;
    levelDepth = level->depth;

    levelMask = pagetable->mask_array[levelDepth];
    levelShift = pagetable->shift_array[levelDepth];
    pageIndex = logicalToPage(log_addr,levelMask,levelShift);
    type = level->node_type;
    pageMask = pagetable->pageTableMask;
    offset = pagetable->offsetBits;
    /*
        Different behavior depending if node is leaf or inner.
     */
    switch (type) {
        case IS_LEAF:
            //Detect if an error occured in allocating the map array
            if(!level->map){
                fprintf(stderr, "Map array not allocated for Address 0x%08x\n",log_addr);
                exit(EXIT_FAILURE);
            }
            pageNumber = logicalToPage(log_addr, pageMask, offset);

            //If the indexed map has not been assigned
            //Create a new mapping
            if(!(level->map[pageIndex])){
                level->map[pageIndex] = initMap(pageNumber,frame_number);
                pagetable->frame_asn++;
                pagetable->miss++;
                writeTranslation(pagetable->settings->outputfile, pageNumber, frame_number);
                pagetable->treeSize+=sizeof(struct Map);
            }
            else{
                //We have a hit!
                frame_number = level->map[pageIndex]->frameNumber;
                pagetable->hits++;
            }
            //Print translated address if option was set
            if(pagetable->settings->show_translation){
                unsigned int phy_addr;
                phy_addr = getPhysAddr(log_addr,pagetable,level->map[pageIndex]);
                writeTranslation(stdout, log_addr, phy_addr);
                //printf("%08x -> %08x\n",log_addr,phy_addr);
            }
            /*else{
                writeTranslation(pagetable, pageNumber, frame_number);
                //printf("%08x -> %08x\n",pageNumber,frame_number);
            }*/

            break;
            
        case  IS_INNER:
            if(!level->next_level){
                fprintf(stderr,"Level Array Not Allocated for Address 0x%8x\n", log_addr);
                exit(EXIT_FAILURE);
            }
            
            //Do we need to create a new page for this mapping?
            Level *subLevel = level->next_level[pageIndex];
            if(!(subLevel)){
                pageNumber = logicalToPage(log_addr, pageMask, offset);
                level->next_level[pageIndex] = initLevel(pagetable,levelDepth+1);
                pagetable->treeSize+=sizeof(struct Level);
                
                //Keep going if more levels are left
                pageInsert_Level(level->next_level[pageIndex], log_addr,frame_number);
            }
            else{
                //Inner level page hit. recursive call.
                pageInsert_Level(level->next_level[pageIndex], log_addr, frame_number);
            }
        default:
            break;
    }
    return 0;
}
/*
    Create a new mapping with the given page and availabe frame
 */
Map *initMap(unsigned int page_number,unsigned int frame_number){
    Map *map_ptr = (Map *) malloc(sizeof(struct Map));
    if(!map_ptr){
        fprintf(stderr, "Error Allocating Map\n");
        exit(EXIT_FAILURE);
    }
    map_ptr->pageNumber = page_number;
    map_ptr->frameNumber = frame_number;
    return map_ptr;
}

/*
    Return the page to frame mapping for a given logical address
 */
Map *pageLookup(PageTable *pagetable, unsigned int log_addr){
    Map *mapping = NULL;
    Level *currentLevel;
    if(!pagetable){
        fprintf(stderr, "No pagetable allocated\n");
        exit(EXIT_FAILURE);
    }
    currentLevel = pagetable->root;
    int depth = 0;
    while (currentLevel) {
        int index = logicalToPage(log_addr,pagetable->mask_array[depth], pagetable->shift_array[depth]);
        if(currentLevel->node_type==IS_LEAF){
            return currentLevel->map[index];
        }
        else{
            currentLevel = currentLevel->next_level[index];
        }
        depth++;
    }
    return mapping;
}
/*
    helper function. all it does it OR all the masks to aid with certain things.
    result is used in determining the page number and the offset index.
    EX: level 0:4 bits level 2:8 bits level 3: 8 bits
        returns the number 0xFFFFF000.
    The NOT of this can be used to pull the offset index.
    0x00000FFF
 */
unsigned int getPageMask(unsigned int *mask_ary,int length){
    int i  = 0;
    unsigned int mask=0;
    while(i<length){
        mask = mask_ary[i++]|mask;
    }
    return mask;
}
/*
    returns the physcial address of a logical address.
    multiplies the frame number by the page size and adds the offset bits.
    
 */
unsigned int getPhysAddr(unsigned int log_addr,PageTable *table,Map *map){
    unsigned int p_addr;
    //bit magic
    p_addr = map->frameNumber*(pow(2,table->offsetBits))+((~table->pageTableMask)&log_addr);
    return p_addr;
    
}
/*
 Creates array of shift quantity needed for the bit mask at each level
 */
unsigned int *getShift(unsigned int *level_bits, int length){
    unsigned int *shift_ary_ptr = (unsigned int *)malloc(length *sizeof(unsigned int));
    int i = 0;
    int bit_count = 32;
    while(i < length){
        bit_count-=level_bits[i];
        shift_ary_ptr[i] = bit_count;
        i++;
    }
    return shift_ary_ptr;
}
/*
 Create an array of the index counts required for each level in the page table;
 */
int *getEntryCounts(unsigned int *level_bits,int length){
    int *entry_count_ptr = (int *)malloc(length *sizeof(int));
    int i = 0;
    while(i <length){
        entry_count_ptr[i] = (int) pow(2, level_bits[i]);
        i++;
    }
    return entry_count_ptr;
}
/*
 This line essentially creates n number of 1's in a 32 bit field
 and shifts them over the required amount for the given level
 using the value in shiftArray. Each mask is populated into an
 array that is eventually returned.
 */

unsigned int *getMask(unsigned int *level_bits,unsigned int *shiftArray,int length)
{
    unsigned int *mask_array = (unsigned int*) malloc(length * sizeof(unsigned int));
    int i = 0;
    while (i < length) {
        unsigned int mask = ((1U<<level_bits[i])-1)<<shiftArray[i];
        mask_array[i]=mask;
        i++;
    }
    return mask_array;
}
/*
 Generates the page number for a given level using the shift and mask values.
 */
unsigned int logicalToPage(unsigned int log_Addr,unsigned int level_mask,unsigned shift){
    unsigned int masked_addr = log_Addr&level_mask;
    unsigned int frame_number = masked_addr>>shift;
    return frame_number;
}

/*
    Write to proper location based settings.
    If the output file option was never given the file pointer should be null and 
    the if statement false;
 */
void writeTranslation(FILE *outfile,unsigned int trans_a,unsigned trans_b){
    if(outfile){
        fprintf(outfile,"%08x -> %08x\n",trans_a,trans_b);
    }

}
