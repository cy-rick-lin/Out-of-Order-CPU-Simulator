#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <cstdlib>
#include <typeinfo>
#include <bitset>
#include <cstdio>


class OoO{
    private:
        uint64_t    pc;
        int         **TimeArray;
        int         cycle = 0;

        int         op_type, dest, src1, src2;

        int         rob_size;
        int         iq_size;
        int         width;

        int         **FE_reg;
        int         **DE_reg;
        int         **RE_reg;

        bool DE_empty = true;
        bool RE_empty = true;

        bool FE_Valid[width];
        bool DE_Valid[width];
        bool RN_Valid[width];
        bool RR_Valid[width];
        bool DI_Valid[width];
        bool IS_Valid[width];
        bool EX_Valid[width];
        bool WB_Valid[width];
        bool RT_Valid[width];
        
    public:
        int         CYCLE;
        int         inst_count = 0;


    // Method
    //

    void Retire(){
    }

    void WriteBack(){}

    void Execute(){}

    void Issue(){}

    void Dispatch(){}

    void RegRead(){}

    void Rename(){}

    void Decode(){
        if(RE_empty){
            for(int i = 0; i < width; i++){
                if(DE_Valid[i]){
                    for(int j = 0; j < 5; j++){
                        RE_reg[i][j] = DE_reg[i][j];
                    }
                    RE_Valid[i] = true;
                }
                else{
                    for(int j = 0; j < 5; j++){
                        RE_reg[i][j] = 0;
                    }
                    RE_Valid[i] = false;
                }
            }
        }
        else{
            for(int i = 0; i < width; i++){
                if(DE_Valid[i]){
                    TimeArray[DE_reg[i][0]][8]++;
                }
            }
        }

        if(DE_empty){
            // When next stage is empty, all the instructions in the current stage are moved to the next stage
            // "Valid" is not equivalent to "Empty" signal
            for(int i = 0; i < width; i++){
                if(FE_Valid[i]){
                    for(int j = 0; j < 4; j++){
                        DE_reg[i][j] = FE_reg[i][j];
                    }
                    DE_Valid[i] = true;
                    DE_empty = false;
                    FE_empty = true;
                }
                else{
                    for(int j = 0; j < 4; j++){
                        DE_reg[i][j] = 0;
                    }
                    DE_Valid[i] = false;
                }
            }
        }
    }

    void Fetch(){
        if(FE_empty){
            for(int i = 0; i < width; i++){
                if(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF){
                    FE_reg[i] = {pc, op_type, dest, src1, src2};
                    FE_Valid[i] = true;
                    TimeArray[pc][0] = pc;
                    TimeArray[pc][1] = op_type;
                    TimeArray[pc][2] = src1;
                    TimeArray[pc][3] = src2;
                    TimeArray[pc][4] = dest;
                    TimeArray[pc][5] = cycle;
                    TimeArray[pc][6] = 1;
                }
                else{
                    FE_reg[i] = {0, 0, 0, 0, 0};
                    FE_Valid[i] = false;
                }
            }
            FE_empty = false;
        }
        else{
            for(int i = 0; i < width; i++){
                if(FE_Valid[i]){
                    TimeArray[FE_reg[i][0]][6]++;
                }
            }
        }
    }

    bool AdvanceCycle(){
        // 0 fu{0} src{29,14} dst{-1} FE{0,1} DE{1,1} RN{2,1} RR{3,1} DI{4,1} IS{5,1} EX{6,1} WB{7,1} RT{8,1}
        // 1 fu{2} src{29,-1} dst{14} FE{1,1} DE{2,1} RN{3,1} RR{4,1} DI{5,1} IS{6,1} EX{7,5} WB{12,1} RT{13,1}
        // 2 fu{2} src{29,-1} dst{15} FE{2,1} DE{3,1} RN{4,1} RR{5,1} DI{6,1} IS{7,1} EX{8,5} WB{13,1} RT{14,1}
        // 3 fu{2} src{30,-1} dst{19} FE{3,1} DE{4,1} RN{5,1} RR{6,1} DI{7,1} IS{8,1} EX{9,5} WB{14,1} RT{15,1}
        // 4 fu{2} src{14,-1} dst{14} FE{4,1} DE{5,1} RN{6,1} RR{7,1} DI{8,1} IS{9,3} EX{12,5} WB{17,1} RT{18,1}
        // 5 fu{2} src{15,-1} dst{16} FE{5,1} DE{6,1} RN{7,1} RR{8,1} DI{9,1} IS{10,3} EX{13,5} WB{18,1} RT{19,1}
        // 6 fu{0} src{29,14} dst{-1} FE{6,1} DE{7,1} RN{8,1} RR{9,1} DI{10,1} IS{11,6} EX{17,1} WB{18,1} RT{19,2}
        // 7 fu{0} src{19,-1} dst{3} FE{7,1} DE{8,1} RN{9,1} RR{10,1} DI{11,1} IS{12,2} EX{14,1} WB{15,1} RT{16,6}
        // 8 fu{1} src{0,0} dst{23} FE{8,1} DE{9,1} RN{10,1} RR{11,1} DI{12,1} IS{13,2} EX{15,2} WB{17,1} RT{18,5}
        cycle++;
        bool valid[10] = {false};
        valid[0] = BitwiseOr(FE_Valid, width);
        valid[1] = BitwiseOr(DE_Valid, width);
        valid[2] = BitwiseOr(RN_Valid, width);
        valid[3] = BitwiseOr(RR_Valid, width);
        valid[4] = BitwiseOr(DI_Valid, width);
        valid[5] = BitwiseOr(IS_Valid, width);
        valid[6] = BitwiseOr(EX_Valid, width);
        valid[7] = BitwiseOr(WB_Valid, width);
        valid[8] = BitwiseOr(RT_Valid, width);

        for(int i = 0; i < 9; i++){
            valid[9] |= valid[i];
        }

        return valid[9];
    }

    void OoO_init(
        FILE *FP,
        uint64_t rob_size,
        uint64_t iq_size,
        uint64_t width
    ){
        //fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2);

        OoO::rob_size = int(rob_size);
        OoO::iq_size = int(iq_size);
        OoO::width = int(width);

        NofLines = countLines(FP);
        TimeArray = new int*[NofLines];
        for(int i = 0; i < NofLines; i++){
            TimeArray[i] = new int[23];
        }

        //oldest = iq_size;
        ROB = new int*[rob_size];
        ISQueue = new int*[iq_size];
        RMT = new int*[ARF_size];

        int** FE_reg = new int*[width];
        int** DE_reg = new int*[width];
        for(int i = 0; i < width; i++){
            FE_reg[i] = new int[5];
            DE_reg[i] = new int[5];
        }

        for(int i = 0; i < width; i++){
            for(int j = 0; j < 5; j++){
                FE_reg[i][j] = 0;
                DE_reg[i][j] = 0;
            }
        }
    }

    void PRINT(
        FILE *debug_fptr
    ){
        // if(cycle > 9600){
            for(int i = 0; i < 100; i++){
                fprintf(debug_fptr, "%d fu{%d} src{%d,%d} dst{%d}", TimeArray[i][0], TimeArray[i][1], TimeArray[i][2], TimeArray[i][3], TimeArray[i][4]);
                fprintf(debug_fptr, " FE{%d,%d} DE{%d,%d} RN{%d,%d} RR{%d,%d}", TimeArray[i][5], TimeArray[i][6], TimeArray[i][7], TimeArray[i][8], TimeArray[i][9], TimeArray[i][10], TimeArray[i][11], TimeArray[i][12]);
                fprintf(debug_fptr, " DI{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d}\n", TimeArray[i][13], TimeArray[i][14], TimeArray[i][15], TimeArray[i][16], TimeArray[i][17], TimeArray[i][18], TimeArray[i][19], TimeArray[i][20], TimeArray[i][21], TimeArray[i][22]);
                // printf("%d fu{%d} src{%d,%d} dst{%d}", TimeArray[i][0], TimeArray[i][1], TimeArray[i][2], TimeArray[i][3], TimeArray[i][4]);
                // printf(" FE{%d,%d} DE{%d,%d} RN{%d,%d} RR{%d,%d}", TimeArray[i][5], TimeArray[i][6], TimeArray[i][7], TimeArray[i][8], TimeArray[i][9], TimeArray[i][10], TimeArray[i][11], TimeArray[i][12]);
                // printf(" DI{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d}\n", TimeArray[i][13], TimeArray[i][14], TimeArray[i][15], TimeArray[i][16], TimeArray[i][17], TimeArray[i][18], TimeArray[i][19], TimeArray[i][20], TimeArray[i][21], TimeArray[i][22]);

            }
            //fprintf(debug_fptr, "\n");
            // printf("\n");
        // }
    }

    void DEBUG(
        FILE *debug_fptr
    ){
        if(cycle > 0){
            fprintf(debug_fptr, "\n##############################\n");
            fprintf(debug_fptr, "        CYCLE %d\n", cycle-1);
            fprintf(debug_fptr, "##############################\n");

            fprintf(debug_fptr, "V   RMT tag\n");
            fprintf(debug_fptr, "------------\n");

            for(int i = 0; i < ARF_size; i++){
                fprintf(debug_fptr, "%d    %d\n", RMT[i][0], RMT[i][1]);
            }

            fprintf(debug_fptr, "\nIssue Queue      IS_full: %d       IS_full_FE: %d\n", IS_full, IS_full_FE);
            fprintf(debug_fptr, "----------------------------------------------------------------------\n");
            fprintf(debug_fptr, "v   op   dst     rdy1    src1   src1    rdy2    src2    src2    pc\n");
            for(int i = 0; i < iq_size; i++){
                fprintf(debug_fptr, "%d   %d    %d       %d       %d      %d       %d       %d       %d       %d\n", ISQueue[i][0], ISQueue[i][1], ISQueue[i][2], ISQueue[i][3], ISQueue[i][4], ISQueue[i][5], ISQueue[i][6], ISQueue[i][7], ISQueue[i][8], ISQueue[i][9]);
            }

            fprintf(debug_fptr, "\nROB\n");
            fprintf(debug_fptr, "-------------------------------\n");
            fprintf(debug_fptr, "dst      rdy     pc\n");
            for(int i = 0; i < rob_size; i++){
                fprintf(debug_fptr, "%d      %d      %d\n", ROB[i][0], ROB[i][1], ROB[i][2]);
            }

            fprintf(debug_fptr, "\nProcessor State\n");
            fprintf(debug_fptr, "--------------------------------------------\n");
            fprintf(debug_fptr, "FE:  v  pc_now  op  dst  src1   src2\n");
            fprintf(debug_fptr, "     %d     %d     %d    %d     %d      %d\n", Valid[0], FE_reg[0][4], FE_reg[0][0], FE_reg[0][1], FE_reg[0][2], FE_reg[0][3]);

            fprintf(debug_fptr, "DE:  v  pc_now  op  dst  src1   src2\n");
            fprintf(debug_fptr, "     %d     %d     %d    %d     %d      %d\n", Valid[1], DE_reg[0][4], DE_reg[0][0], DE_reg[0][1], DE_reg[0][2], DE_reg[0][3]);

            fprintf(debug_fptr, "RN:  v  pc_now  op  dst  ROB1   src1   ROB2    src2    head    tail    head    tail    ROB_full=%d     ROB_full2=%d\n", ROB_full, ROB_full2);
            fprintf(debug_fptr, "     %d     %d     %d    %d     %d      %d      %d      %d       %d       %d       %d       %d\n", Valid[2], RN_reg[1][6], RN_reg[1][0], RN_reg[1][1], RN_reg[1][2], RN_reg[1][3], RN_reg[1][4], RN_reg[1][5], head_temp0, tail_temp1, head_temp0, tail_temp2);

            fprintf(debug_fptr, "RR:  v  pc_now  op  dst  ROB1   src1   ROB2    src2    protect1:%d%d     protect2:%d%d\n", protect1[0], protect1[1], protect2[0], protect2[1]);
            fprintf(debug_fptr, "     %d     %d     %d    %d     %d      %d      %d      %d\n", Valid[3], RR_reg[1][6], RR_reg[1][0], RR_reg[1][1], RR_reg[1][2], RR_reg[1][3], RR_reg[1][4], RR_reg[1][5]);

            fprintf(debug_fptr, "DI:  v  pc_now  op  dst  ROB1   src1   ROB2    src2\n");
            fprintf(debug_fptr, "     %d     %d     %d    %d     %d      %d      %d      %d\n", Valid[4], DI_reg[0][6], DI_reg[0][0], DI_reg[0][1], DI_reg[0][2], DI_reg[0][3], DI_reg[0][4], DI_reg[0][5]);

            fprintf(debug_fptr, "IS:  v  pc_now  op  dst  ROB1   src1   ROB2    src2\n");
            fprintf(debug_fptr, "     %d     %d     %d    %d     %d      %d      %d      %d\n", Valid[5], IS_reg[0][6], IS_reg[0][0], IS_reg[0][1], IS_reg[0][2], IS_reg[0][3], IS_reg[0][4], IS_reg[0][5]);

            fprintf(debug_fptr, "\nEX:          1              2              3             4               5\n");
            fprintf(debug_fptr, "          v  pc  dst     v  pc  dst     v  pc  dst     v  pc  dst     v  pc  dst\n");
            fprintf(debug_fptr, "simple    %d  %d    %d\n", simple_Valid[0], simple_pc[0], simple_reg[0]);
            fprintf(debug_fptr, "complex   %d  %d    %d      %d  %d    %d\n", complex_Valid[0][0], complex_pc[0][0], complex_reg[0][0], complex_Valid[0][1], complex_pc[0][1], complex_reg[0][1]);
            fprintf(debug_fptr, "cache     %d  %d    %d      %d  %d    %d      %d  %d    %d      %d  %d    %d      %d  %d    %d\n", cache_Valid[0][0], cache_pc[0][0], cache_reg[0][0], cache_Valid[0][1], cache_pc[0][1], cache_reg[0][1], cache_Valid[0][2], cache_pc[0][2], cache_reg[0][2], cache_Valid[0][3], cache_pc[0][3], cache_reg[0][3], cache_Valid[0][4], cache_pc[0][4], cache_reg[0][4]);

            fprintf(debug_fptr, "\nWB:         Valid       dst     pc\n");
            fprintf(debug_fptr, "Simple      %d            %d      %d\n", WB_Valid[0][0], WB_reg[0][0], WB_pc[0][0]);
            fprintf(debug_fptr, "complex     %d            %d      %d\n", WB_Valid[0][1], WB_reg[0][1], WB_pc[0][1]);
            fprintf(debug_fptr, "cache       %d            %d      %d\n", WB_Valid[0][2], WB_reg[0][2], WB_pc[0][2]);

            fprintf(debug_fptr, "\nRT-ROB\n");
            fprintf(debug_fptr, "-------------------------------\n");
            fprintf(debug_fptr, "dst      rdy     pc    head: %d    head_pc: %d     head_rdy: %d    head: %d    tail: %d\n", head_temp1, head_pc, head_rdy, head_temp2[0], ROB_tail);
            for(int i = 0; i < rob_size; i++){
                fprintf(debug_fptr, "%d      %d      %d\n", ROB[i][0], ROB[i][1], ROB[i][2]);
            }
        }

        //printf("RT:")
    }

    int countLines(
        FILE* file
    ){
        if (file == NULL) {
            printf("Error: File pointer is NULL\n");
            return -1;
        }

        int count = 0;
        char ch;
        while ((ch = fgetc(file)) != EOF) {
            if (ch == '\n') {
                count++;
            }
        }
        return count;
    }

    bool BitwiseOr(bool* array, int size) {
        bool result = array[0];
        for(int i = 1; i < size; i++) {
            result |= array[i];
        }
        return result;
    }
};