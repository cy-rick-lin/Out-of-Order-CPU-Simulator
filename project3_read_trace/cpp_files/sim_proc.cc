#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <fstream>
#include <string>
#include <iostream>
#include <cstdio>
#include "sim_proc.h"
#include "OoO.cpp"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim 256 32 4 gcc_trace.txt
    argc = 5
    argv[0] = "sim"
    argv[1] = "256"
    argv[2] = "32"
    ... and so on
*/
int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
    int op_type, dest, src1, src2;  // Variables are read from trace file
    uint64_t pc; // Variable holds the pc read from input file
    
    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];

    // params.rob_size = 16;
    // params.iq_size = 8;
    // params.width = 1;
    // trace_file = "./gcc_trace.txt";
    // printf("rob_size:%lu "
    //         "iq_size:%lu "
    //         "width:%lu "
    //         "tracefile:%s\n", params.rob_size, params.iq_size, params.width, trace_file);
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // The following loop just tests reading the trace and echoing it back to the screen.
    //
    // Replace this loop with the "do { } while (Advance_Cycle());" loop indicated in the Project 3 spec.
    // Note: fscanf() calls -- to obtain a fetch bundle worth of instructions from the trace -- should be
    // inside the Fetch() function.
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    OoO OoO;
    OoO.OoO_init(FP, params.rob_size, params.iq_size, params.width);

    FILE *debug_fptr = fopen("dubug.txt", "w");
    if(debug_fptr == NULL){
        printf("Error in opening debug.txt!\n");
        return 1;
    }

    do{
        OoO.RT();
        OoO.WB();
        OoO.EX();
        OoO.IS();
        OoO.DI();
        OoO.RR();
        OoO.RN();
        OoO.DE();
        OoO.Fetch(FP);
        fprintf(debug_fptr, "\n%d\n", OoO.CYCLE-1);
        OoO.PRINT(debug_fptr);
        OoO.DEBUG(debug_fptr);
    } while(OoO.AdvanceCycle());

    OoO.PRINT(debug_fptr);
    fclose(debug_fptr);

    printf("# === Simulator Command =========\n");
    printf("# ./sim %d %d %d val_trace_gcc1\n", params.rob_size, params.iq_size, params.width);
    printf("# === Processor Configuration ===\n");
    printf("# ROB_SIZE = %d\n", params.rob_size);
    printf("# IQ_SIZE  = %d\n", params.iq_size);
    printf("# WIDTH    = %d\n", params.width);
    printf("# === Simulation Results ========\n");
    printf("# Dynamic Instruction Count    = %d\n", OoO.inst_count);
    printf("# Cycles                       = %d\n", OoO.CYCLE);
    printf("# Instructions Per Cycle (IPC) = %.2f\n", float(OoO.inst_count)/float(OoO.CYCLE));

    // while(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
    //     printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly

    return 0;
}



// ab120024 0 1 2 3
// ab120028 1 4 1 3
// ab12002c 2 -1 4 7