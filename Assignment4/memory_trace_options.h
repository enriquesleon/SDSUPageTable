

#include <stdio.h>
#include <getopt.h>
#define SHOW_TRANSLATION 1
#define DONT_SHOW_TRANSLATION 0
#define MAX_BITS 32
#ifndef SOURCE_MEMORY_TRACE_OPTIONS_H_
#define SOURCE_MEMORY_TRACE_OPTIONS_H_

typedef struct Memory_setting
{
    int number_to_process;
    int show_translation;
    unsigned int *level_settings;
    char *print_to_file;
    char *input_file;
    FILE *tracefile;
    FILE *outputfile;
    
}Memory_setting;


int get_settings(int argc,char** argv, Memory_setting *settings);
int valid_level_settings(unsigned int *level_settings,int length);


#endif /* SOURCE_MEMORY_TRACE_OPTIONS_H_ */
