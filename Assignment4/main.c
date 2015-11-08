
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "memory_trace_options.h"
#include "memory_page.h"
#include <math.h>
#include "byutr.h"
int limit(int num_proc,unsigned int *count);
int main(int argc, char * argv[]) {
    
    p2AddrTr trace;
    PageTable *table;
    char *syssort;
    int offset_size;
    Memory_setting settings = {0,DONT_SHOW_TRANSLATION,NULL,NULL,NULL};
    int tableSize = get_settings(argc,argv,&settings);
    if(tableSize<1){
        printf("exit no args:\n");
        exit(EXIT_FAILURE);
    }
    //retrieve and check for valid settings
    if((offset_size = valid_level_settings(settings.level_settings, tableSize))<1){
        fprintf(stderr, "Invalid Level Settings\n");
        exit(EXIT_FAILURE);
    }
    //initiate table struct
    table = initPageTable(settings.level_settings,tableSize,offset_size,&settings);
    
    //read in addresses.
    unsigned int count = 0;
    while(!feof(settings.tracefile)&&limit(settings.number_to_process,&count)){
        if(NextAddress(settings.tracefile, &trace)){
            unsigned int next_addr = (unsigned int)trace.addr;
            pageInsertTable(table, next_addr, table->frame_asn);

        }
        count++;
    }
    //set up the summary printout
    unsigned int table_hit = table->hits;
    double rate = ((double)table_hit/(double)count);
    printf("Page Size: 0x%x\n",(unsigned int)pow(2,table->offsetBits));
    printf("Hits:%u (%4.2f)   Misses:%u (%4.2f) ",table->hits,rate,table->miss,1-rate);
    printf(" #Addresses:%u\n",count);
    printf("Tree Size: %lu Bytes\n",table->treeSize);
    
    //lazy sort by pagenumbers...
    if(settings.outputfile){
        fclose(settings.outputfile);
        asprintf(&syssort,"sort -k1 -o %s %s", settings.print_to_file,settings.print_to_file);
        system(syssort);
        free(syssort);
    }

    return 0;
}
//small helper for reading.
int limit(int nump_proc,unsigned int *count){
    if (nump_proc<1) {
        return 1;
    }
    else return(nump_proc!=(*count));
}
