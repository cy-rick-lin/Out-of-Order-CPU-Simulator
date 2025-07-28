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

// ---------------------- From SPEC ----------------------
// In general, this spec names a pipeline register based on the stage that it feeds into.

// 2. I think all values can be found in ROB, no matter when its value is waken up, no need to worry about
// the naming issue issue.


class OoO{
    private:
        uint64_t    pc;
        int         **TimeArray;
        int         NofLines;
        int         cycle = 0;

        int         op_type, dest, src1, src2;

        int         rob_size;
        int         iq_size;
        int         width;

        int         **ROB;
        int         **IS_Queue;
        int         ***EX_list;
        int         ISQ_oldest = 0;
        int         RMT[10][2] = {0};
        int         ARF_size = 10;
        int         ROB_head = 0;
        int         ROB_tail = 0;
        bool        ROB_full = false;
        bool        IS_full = false;
        bool        IS_empty = true;

        int         **FE_reg;
        int         **DE_reg;
        int         **RN_reg;
        int         **RN_reg_after_rename;
        int         **RR_reg;
        int         **DI_reg;
        int         **IS_reg;

        bool        RN_Change = true;
        bool        FE_empty = true;
        bool        DE_empty = true;
        bool        RN_empty = true;
        bool        RR_empty = true;
        bool        DI_empty = true;
        bool        *FE_Valid;
        bool        *DE_Valid;
        bool        *RN_Valid;
        bool        *RR_Valid;
        bool        *DI_Valid;
        bool        *IS_Valid;
        bool        *EX_Valid;
        bool        *WB_Valid;
        bool        *RT_Valid;
        
    public:
        int         CYCLE = 0;
        int         inst_count = 0;


    // Method
    //

    void Retire(){}

    void WriteBack(){
        // EX_list[i][j][7] = 1, send to wb stage
        // for(int i = 0; i < width; i++){
        //     for(int j = 0; j < 5; j++){
        //         if(EX_list[i][j][7] == 1){

        //         }
        // }
    }

    void Execute(){
        for(int i = 0; i < width; i++){
            for(int j = 0; j < 5; j++){
                if(EX_list[i][j][7] != 0)
                    EX_list[i][j][7]--;
                else{
                    // Issue Queue wake-up
                    // Other wake-up are at WB stage(ROB related)
                    for(int k = 0; k < iq_size; k++){
                        if(IS_Queue[k][6] == EX_list[i][j][2] && IS_Queue[k][4] == 1)
                            IS_Queue[k][5] = 1;
                        if(IS_Queue[k][9] == EX_list[i][j][2] && IS_Queue[k][7] == 1)
                            IS_Queue[k][8] = 1;
                    }                    
                }
            }
        }

        // Issue up to "width" instructions to FU
        // Find valid and the oldest instruction in IS_Queue
        // **********************************************************************
        //                       Execution List
        // ----------------------------------------------------------
        // | pc | op_type | dst | A/R | src1 | A/R | src2 | Counter |
        // ----------------------------------------------------------
        // Counter = 0, which means the instruction is finished or there's no valid instruction at this position
        for(int i = 0; i < width; i++){
            ISQ_oldest = find_oldest_valid_pc(IS_Queue, iq_size);
            IS_empty = check_IS_empty(ISQ_oldest);

            if(IS_empty)
                break;

            if(IS_Queue[ISQ_oldest][5] && IS_Queue[ISQ_oldest][8]){
                for(int j = 0; j < 5; j++){
                    if(EX_list[i][j][7] == 0){
                        EX_list[i][j][0] = IS_Queue[ISQ_oldest][1];
                        EX_list[i][j][1] = IS_Queue[ISQ_oldest][2];
                        EX_list[i][j][2] = IS_Queue[ISQ_oldest][3];
                        EX_list[i][j][3] = IS_Queue[ISQ_oldest][4];
                        EX_list[i][j][4] = IS_Queue[ISQ_oldest][6];
                        EX_list[i][j][5] = IS_Queue[ISQ_oldest][7];
                        EX_list[i][j][6] = IS_Queue[ISQ_oldest][9];
                        EX_list[i][j][7] = (IS_Queue[ISQ_oldest][2] == 2) ? IS_Queue[ISQ_oldest][2] : 5;

                        IS_Queue[ISQ_oldest][0] = false;
                        break;
                    }
                }
            }
        }
    }

    void Issue(){
        // Dispatch mechanism
        // Dispatch up to "width" instructions
        // **********************************************************************
        //                              Issue Queue
        // ----------------------------------------------------------------------
        // | v | pc | op_type | dst | A/R | ready1 | src1 | A/R | ready2 | src2 |
        // ----------------------------------------------------------------------
        // | 0 | 1  |    2    |  3  |  4  |    5   |   6  |  7  |    8   |   9  |
        // ----------------------------------------------------------------------
        IS_full = check_IS_full(IS_Queue, iq_size);

        if(!IS_full){
            for(int i = 0; i < width; i++){
                if(DI_Valid[i]){
                    for(int j = 0; j < iq_size; j++){       // Find space in IS_Queue
                        if(IS_Queue[j][0] == false){        // If v == 0, then the old instruction is not valid, hence replace
                            IS_Queue[j][0] = true;          // Set valid
                            IS_Queue[j][1] = DI_reg[i][0];
                            IS_Queue[j][2] = DI_reg[i][1];
                            IS_Queue[j][3] = DI_reg[i][2];
                            IS_Queue[j][4] = DI_reg[i][3];
                            IS_Queue[j][5] = DI_reg[i][5] ? DI_reg[i][5] : ROB[DI_reg[i][6]][1];
                            IS_Queue[j][6] = DI_reg[i][6];
                            IS_Queue[j][7] = DI_reg[i][7];
                            IS_Queue[j][8] = DI_reg[i][8] ? DI_reg[i][8] : ROB[DI_reg[i][9]][1];
                            IS_Queue[j][9] = DI_reg[i][9];
                        }
                    }
                    DI_empty = true;
                    TimeArray[DI_reg[i][0]][15] = cycle;
                }
            }

            for(int i = 0; i < width; i++){
                if(IS_Queue[i][0] == 1){
                    TimeArray[IS_Queue[i][1]][16]++;
                }
            }
        }
    }

    void Dispatch(){
        // src1 and src2 can either get value from ROB(after EX update) or ARF
        // If src1 or src2 get valiue from ROB, directly set rdy1 or rdy2 to 1
        // **********************************************************************
        //                              DI_reg
        // --------------------------------------------------------------
        // | pc | op_type | dst | A/R | rdy1 | src1 | A/R | rdy2 | src2 |
        // --------------------------------------------------------------
        if(DI_empty){
            for(int i = 0; i < width; i++){
                if(RR_Valid[i]){
                    for(int j = 0; j < 9; j++){
                        DI_reg[i][j] = RR_reg[i][j];
                    }
                    if(DI_reg[i][3] == 1 && ROB[DI_reg[i][5]][1] == 1 || DI_reg[i][3] == 0)
                        DI_reg[i][4] = 1;
                    if(DI_reg[i][6] == 1 && ROB[DI_reg[i][8]][1] == 1 || DI_reg[i][6] == 0)
                        DI_reg[i][7] = 1;

                    DI_Valid[i] = true;
                    DI_empty = false;
                    RR_empty = true;
                    TimeArray[DI_reg[i][0]][13] = cycle;
                    TimeArray[DI_reg[i][0]][14] = 1;
                }
                else{
                    for(int j = 0; j < 7; j++){
                        DI_reg[i][j] = 0;
                    }
                    DI_Valid[i] = false;
                }
            }
        }
        else{
            for(int i = 0; i < width; i++){
                if(DI_Valid[i]){
                    TimeArray[DI_reg[i][0]][14]++;
                }
            }
        }
    }

    void RegRead(){
        // src1 and src2 can either get value from ROB(after EX update) or ARF
        // If src1 or src2 get valiue from ROB, directly set rdy1 or rdy2 to 1
        // **********************************************************************
        //                              RR_reg
        // --------------------------------------------------------------
        // | pc | op_type | dst | A/R | rdy1 | src1 | A/R | rdy2 | src2 |
        // --------------------------------------------------------------
        if(RR_empty){
            for(int i = 0; i < width; i++){
                if(RN_Valid[i]){
                    for(int j = 0; j < 9; j++){
                        RR_reg[i][j] = RN_reg_after_rename[i][j];
                    }
                    if(RR_reg[i][3] == 1 && ROB[RR_reg[i][5]][1] == 1 || RR_reg[i][3] == 0)
                        RR_reg[i][4] = 1;
                    if(RR_reg[i][6] == 1 && ROB[RR_reg[i][8]][1] == 1 || RR_reg[i][6] == 0)
                        RR_reg[i][7] = 1;

                    RR_Valid[i] = true;
                    RR_empty = false;
                    RN_empty = true;
                    TimeArray[RN_reg_after_rename[i][0]][11] = cycle;
                    TimeArray[RN_reg_after_rename[i][1]][12] = 1;
                }
                else{
                    for(int j = 0; j < 7; j++){
                        RR_reg[i][j] = 0;
                    }
                    RR_Valid[i] = false;
                }
            }
        }
        else{
            for(int i = 0; i < width; i++){
                if(RR_Valid[i]){
                    TimeArray[RN_reg_after_rename[i][0]][12]++;
                }
            }
        }
    }

    void Rename(){
        if(RN_empty & !ROB_full){
            for(int i = 0; i < width; i++){
                if(DE_Valid[i]){
                    for(int j = 0; j < 5; j++){
                        RN_reg[i][j] = DE_reg[i][j];
                    }
                    RN_Valid[i] = true;
                    RN_empty = false;
                    DE_empty = true;
                    RN_Change = true;
                    TimeArray[DE_reg[i][0]][9] = cycle;
                    TimeArray[DE_reg[i][0]][10] = 1;
                }
                else{
                    for(int j = 0; j < 5; j++){
                        RN_reg[i][j] = 0;
                    }
                    RN_Valid[i] = false;
                    RN_Change = false;
                }
            }
        }
        else{
            for(int i = 0; i < width; i++){
                if(RN_Valid[i]){
                    // Sus(Probably not sus, as RN_Valid is true, the inst was not going to next stage, so the RN_reg[x][0]
                    // value is still the last pc, which is the same with RN_after_rename_reg
                    TimeArray[RN_reg[i][0]][10]++; 
                    RN_Change = false;
                }
            }
        }

        // If Rename stage has new value from Decode stage and the value
        // is valid, then update the ROB
        // ##############################################################
        //         ROB                  RMT
        // -------------------    ---------------
        // | dst | rdy | pc |     | V | ROB tag |
        // -------------------    ---------------

        //               RN_reg
        // ------------------------------------
        // | pc | op_type | dst | src1 | src2 |
        // ------------------------------------

        // --------------------------------
        // | Rename | ROB | RMT |
        // --------------------------------
        // ROB_tail: point to the empty position in ROB

        //    RN_reg_after_rename
        // --------------------------------------------------------------
        // | pc | op_type | dst | A/R | rdy1 | src1 | A/R | rdy2 | src2 |
        // --------------------------------------------------------------
        // If source is ROB, then set A/R to 1, otherwise set A/R to 0
        if(RN_Change){
            for(int i = 0; i < width; i++){
                if(RN_Valid[i]){
                    ROB[ROB_tail][0] = RN_reg[i][2];
                    ROB[ROB_tail][1] = 0;
                    ROB[ROB_tail][2] = RN_reg[i][0];

                    RMT[RN_reg[i][2]][0] = 1;
                    RMT[RN_reg[i][2]][1] = ROB_tail;

                    RN_reg_after_rename[i][0] = RN_reg[i][0];
                    RN_reg_after_rename[i][1] = RN_reg[i][1];
                    RN_reg_after_rename[i][2] = ROB_tail;
                    RN_reg_after_rename[i][3] = (RMT[RN_reg[i][3]][0] == 1);
                    RN_reg_after_rename[i][4] = 0;
                    RN_reg_after_rename[i][5] = (RMT[RN_reg[i][3]][0] == 1) ? RMT[RN_reg[i][3]][1] : RN_reg[i][3];
                    RN_reg_after_rename[i][6] = (RMT[RN_reg[i][4]][0] == 1);
                    RN_reg_after_rename[i][7] = 0;
                    RN_reg_after_rename[i][8] = (RMT[RN_reg[i][4]][0] == 1) ? RMT[RN_reg[i][4]][1] : RN_reg[i][4];

                    ROB_tail += (ROB_tail == rob_size-1) ? (-rob_size + 1) : 1;
                }
            }
            RN_Change = false;
        }
    }

    void Decode(){
        if(DE_empty){
            // When next stage is empty, all the instructions in the current stage are moved to the next stage
            // "Valid" is not equivalent to "Empty" signal
            // DE_reg
            // ------------------------------------
            // | pc | op_type | dst | src1 | src2 |
            // ------------------------------------
            for(int i = 0; i < width; i++){
                if(FE_Valid[i]){
                    for(int j = 0; j < 4; j++){
                        DE_reg[i][j] = FE_reg[i][j];
                    }
                    DE_Valid[i] = true;
                    DE_empty = false;
                    FE_empty = true;
                    TimeArray[FE_reg[i][0]][7] = cycle;
                    TimeArray[FE_reg[i][0]][8] = 1;
                }
                else{
                    for(int j = 0; j < 4; j++){
                        DE_reg[i][j] = 0;
                    }
                    DE_Valid[i] = false;
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
    }

    void Fetch(
        FILE *FP
    ){
        // FE_reg
        // --------------------------------
        // | pc | op_type | dst | src1 | src2 |
        // --------------------------------
        if(FE_empty){
            for(int i = 0; i < width; i++){
                if(fscanf(FP, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF){
                    FE_reg[i][0] = pc;
                    FE_reg[i][1] = op_type;
                    FE_reg[i][2] = dest;
                    FE_reg[i][3] = src1;
                    FE_reg[i][4] = src2;
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
                    FE_reg[i][0] = 0;
                    FE_reg[i][1] = 0;
                    FE_reg[i][2] = 0;
                    FE_reg[i][3] = 0;
                    FE_reg[i][4] = 0;
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

    int find_oldest_valid_pc(
        int **array,
        int size
    ){
        int oldest = NofLines + 1;

        for(int i = 0; i < size; i++){
            if(array[i][1] < oldest && array[i][0] == true){
                oldest = array[i][1];
            }
        }
        return oldest;
    }

    bool check_IS_empty(
        int oldest
    ){
        if(oldest == NofLines + 1){
            return true;
        }
        else{
            return false;
        }
    }

    bool check_IS_full(
        int **array,
        int size
    ){
        int count = 0;
        for(int i = 0; i < size; i++){
            if(array[i][0] == false){
                count++;
            }
        }
        if(count <= width){
            return true;
        }
        else{
            return false;
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
        CYCLE = cycle;
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

        // Oldest = iq_size;
        ROB = new int*[rob_size];
        IS_Queue = new int*[iq_size];
        EX_list = new int**[width];

        FE_reg = new int*[width];
        DE_reg = new int*[width]; 
        RN_reg = new int*[width];
        RN_reg_after_rename = new int*[width];
        RR_reg = new int*[width];
        DI_reg = new int*[width];
        IS_reg = new int*[width];

        for(int i = 0; i < width; i++){
            FE_reg[i] = new int[5];
            DE_reg[i] = new int[5];
            RN_reg[i] = new int[5];
            RN_reg_after_rename[i] = new int[9];
            RR_reg[i] = new int[7];
            DI_reg[i] = new int[7];
            IS_Queue[i] = new int[10];
            IS_reg[i] = new int[7];
        }

        for(int i = 0; i < width; i++){
            EX_list[i] = new int*[5];
            for(int j = 0; j < 5; j++){
                EX_list[i][j] = new int[5];
            }
        }

        for(int i = 0; i < width; i++){
            for(int j = 0; j < 5; j++){
                FE_reg[i][j] = 0;
                DE_reg[i][j] = 0;
                RN_reg[i][j] = 0;
            }
        }

        for(int i = 0; i < width; i++){
            for(int j = 0; j < 7; j++){
                RR_reg[i][j] = 0;
                DI_reg[i][j] = 0;
                IS_reg[i][j] = 0;
            }
        }

        for(int i = 0; i < width; i++){
            for(int j = 0; j < 9; j++){
                RN_reg_after_rename[i][j] = 0;
            }
        }

        for(int i = 0; i < iq_size; i++){
            for(int j = 0; j < 10; j++){
                IS_Queue[i][j] = 0;
            }
        }

        for(int i = 0; i < width; i++){
            for(int j = 0; j < 5; j++){
                for(int k = 0; k < 5; k++){
                    EX_list[i][j][k] = 0;
                }
            }
        }

        FE_Valid = new bool[width];
        DE_Valid = new bool[width];
        RN_Valid = new bool[width];
        RR_Valid = new bool[width];
        DI_Valid = new bool[width];
        IS_Valid = new bool[width];
        EX_Valid = new bool[width];
        WB_Valid = new bool[width];
        RT_Valid = new bool[width];

        for(int i = 0; i < width; i++){
            FE_Valid[i] = false;
            DE_Valid[i] = false;
            RN_Valid[i] = false;
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

            fprintf(debug_fptr, "\nROB\n");
            fprintf(debug_fptr, "-------------------------------\n");
            fprintf(debug_fptr, "dst      rdy     pc\n");
            for(int i = 0; i < rob_size; i++){
                fprintf(debug_fptr, "%d      %d      %d\n", ROB[i][0], ROB[i][1], ROB[i][2]);
            }

            // fprintf(debug_fptr, "\nIssue Queue      IS_full: %d       IS_full_FE: %d\n", IS_full, IS_full_FE);
            // fprintf(debug_fptr, "\nIssue Queue");
            // fprintf(debug_fptr, "----------------------------------------------------------------------\n");
            // fprintf(debug_fptr, "v   op   dst     rdy1    src1   src1    rdy2    src2    src2    pc\n");
            // for(int i = 0; i < iq_size; i++){
            //     fprintf(debug_fptr, "%d   %d    %d       %d       %d      %d       %d       %d       %d       %d\n", IS_Queue[i][0], IS_Queue[i][1], IS_Queue[i][2], IS_Queue[i][3], IS_Queue[i][4], IS_Queue[i][5], IS_Queue[i][6], IS_Queue[i][7], IS_Queue[i][8], IS_Queue[i][9]);
            // }

            fprintf(debug_fptr, "\nProcessor State\n");
            fprintf(debug_fptr, "--------------------------------------------\n");
            fprintf(debug_fptr, "FE:  v  pc_now  op  dst  src1   src2\n");
            fprintf(debug_fptr, "     %d     %d     %d    %d     %d      %d\n", BitwiseOr(FE_Valid, width), FE_reg[0][4], FE_reg[0][0], FE_reg[0][1], FE_reg[0][2], FE_reg[0][3]);

            fprintf(debug_fptr, "DE:  v  pc_now  op  dst  src1   src2\n");
            fprintf(debug_fptr, "     %d     %d     %d    %d     %d      %d\n", BitwiseOr(DE_Valid, width), DE_reg[0][4], DE_reg[0][0], DE_reg[0][1], DE_reg[0][2], DE_reg[0][3]);

            fprintf(debug_fptr, "RN:  v  pc_now  op  dst  ROB1   src1   ROB2    src2    head    tail     ROB_full=%d\n", ROB_full);
            fprintf(debug_fptr, "     %d     %d     %d    %d     %d      %d      %d      %d       %d       %d       %d       %d\n", BitwiseOr(RN_Valid, width), RN_reg_after_rename[1][6], RN_reg_after_rename[1][0], RN_reg_after_rename[1][1], RN_reg_after_rename[1][2], RN_reg_after_rename[1][3], RN_reg_after_rename[1][4], RN_reg_after_rename[1][5], ROB_head, ROB_tail);

            // fprintf(debug_fptr, "RR:  v  pc_now  op  dst  ROB1   src1   ROB2    src2    protect1:%d%d     protect2:%d%d\n", protect1[0], protect1[1], protect2[0], protect2[1]);
            // fprintf(debug_fptr, "     %d     %d     %d    %d     %d      %d      %d      %d\n", Valid[3], RR_reg[1][6], RR_reg[1][0], RR_reg[1][1], RR_reg[1][2], RR_reg[1][3], RR_reg[1][4], RR_reg[1][5]);

            // fprintf(debug_fptr, "DI:  v  pc_now  op  dst  ROB1   src1   ROB2    src2\n");
            // fprintf(debug_fptr, "     %d     %d     %d    %d     %d      %d      %d      %d\n", Valid[4], DI_reg[0][6], DI_reg[0][0], DI_reg[0][1], DI_reg[0][2], DI_reg[0][3], DI_reg[0][4], DI_reg[0][5]);

            // fprintf(debug_fptr, "IS:  v  pc_now  op  dst  ROB1   src1   ROB2    src2\n");
            // fprintf(debug_fptr, "     %d     %d     %d    %d     %d      %d      %d      %d\n", Valid[5], IS_reg[0][6], IS_reg[0][0], IS_reg[0][1], IS_reg[0][2], IS_reg[0][3], IS_reg[0][4], IS_reg[0][5]);

            // fprintf(debug_fptr, "\nEX:          1              2              3             4               5\n");
            // fprintf(debug_fptr, "          v  pc  dst     v  pc  dst     v  pc  dst     v  pc  dst     v  pc  dst\n");
            // fprintf(debug_fptr, "simple    %d  %d    %d\n", simple_Valid[0], simple_pc[0], simple_reg[0]);
            // fprintf(debug_fptr, "complex   %d  %d    %d      %d  %d    %d\n", complex_Valid[0][0], complex_pc[0][0], complex_reg[0][0], complex_Valid[0][1], complex_pc[0][1], complex_reg[0][1]);
            // fprintf(debug_fptr, "cache     %d  %d    %d      %d  %d    %d      %d  %d    %d      %d  %d    %d      %d  %d    %d\n", cache_Valid[0][0], cache_pc[0][0], cache_reg[0][0], cache_Valid[0][1], cache_pc[0][1], cache_reg[0][1], cache_Valid[0][2], cache_pc[0][2], cache_reg[0][2], cache_Valid[0][3], cache_pc[0][3], cache_reg[0][3], cache_Valid[0][4], cache_pc[0][4], cache_reg[0][4]);

            // fprintf(debug_fptr, "\nWB:         Valid       dst     pc\n");
            // fprintf(debug_fptr, "Simple      %d            %d      %d\n", WB_Valid[0][0], WB_reg[0][0], WB_pc[0][0]);
            // fprintf(debug_fptr, "complex     %d            %d      %d\n", WB_Valid[0][1], WB_reg[0][1], WB_pc[0][1]);
            // fprintf(debug_fptr, "cache       %d            %d      %d\n", WB_Valid[0][2], WB_reg[0][2], WB_pc[0][2]);

            // fprintf(debug_fptr, "\nRT-ROB\n");
            // fprintf(debug_fptr, "-------------------------------\n");
            // fprintf(debug_fptr, "dst      rdy     pc    head: %d    head_pc: %d     head_rdy: %d    head: %d    tail: %d\n", head_temp1, head_pc, head_rdy, head_temp2[0], ROB_tail);
            // for(int i = 0; i < rob_size; i++){
            //     fprintf(debug_fptr, "%d      %d      %d\n", ROB[i][0], ROB[i][1], ROB[i][2]);
            // }
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