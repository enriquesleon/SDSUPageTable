


#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include "memory_trace_options.h"

//Handles all the main parsing in one function. quick and dirty.
int get_settings (int argc, char **argv, Memory_setting *settings){
    int c;
    int option_index;
    int strcon;
    unsigned int *levels;
    int i = 0;
    char *endptr;
    char *arguments;
    struct option long_options[] = {
        {"process to",required_argument,0,'n'},
        {"output filepath",required_argument,0,'p'},
        {"print translation",no_argument,0,'t'},
        {0,0,0,0}
    };
    while((c = getopt_long (argc, argv, "n:p:t",long_options, &option_index))!= -1)
    {
        switch (c){
            case 0:
                printf("0\n");
                break;
            case 'n':
                settings->number_to_process = atoi(optarg);
                break;
            case 'p':
                settings->print_to_file = optarg;
                if(!(settings->outputfile = fopen(settings->print_to_file, "w"))){
                    fprintf(stderr, "Could not open write file\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 't':
                settings->show_translation=SHOW_TRANSLATION;
                break;
            case '?':
                fprintf(stderr,"%s Usage:[-n #] [-p filepath] [-t]",argv[0]);
                break;
            default:
                break;
        }
    }
    
    if(argc-optind > 1){
        if(!(settings->tracefile = fopen(argv[optind], "rb"))){
            fprintf(stderr, "Could not open file: %s\n",argv[optind]);
            exit(EXIT_FAILURE);
        }
        optind++;
        levels = (unsigned int *) malloc((argc-optind)*sizeof(unsigned int));
        while(optind < argc){
            arguments = argv[optind++];
            strcon = (int)strtol(arguments,&endptr,10);
            if(arguments==endptr){
                fprintf(stderr,"No Digits found in Argument:%s\n",arguments);
                exit(EXIT_FAILURE);
            }
            if (strcon < 1) {
                fprintf(stderr,"Setting less than 1: %d\n",strcon);
                exit(EXIT_FAILURE);
            }
            else{
                levels[i++] = (unsigned int)strcon;
            }
        }
        settings->level_settings = levels;
    }
    else
        exit(EXIT_FAILURE);
    return i;
}

//checks if the parameters for the levels are valid or not.
int valid_level_settings(unsigned int *level_settings,int length){
    int total = 0;
    int i = 0;
    while(i<length){
        total += level_settings[i];
        if(level_settings[i]<1){
            printf("Error in level settings:%d\n",level_settings[i]);
            return -1;
        }
        i++;
    }
    return MAX_BITS-total;
}
