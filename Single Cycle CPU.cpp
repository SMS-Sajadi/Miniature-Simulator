#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<stdbool.h>

#define R_type 1
#define I_type 2
#define J_type 3

#define dash_line printf("-------------------------------------------------------------------------\n")

const char* inst[] = {"add","sub","slt","or","nand","addi","slti","ori","lui","lw","sw","beq","jalr","j","halt"};

int mem[16000];
bool mem_use[16000];
int* machine_code;
int R[16];

struct instruction
{
    int type;
    int int_of_inst;
    int opcode;
    char mnemonic[5];
    int rd;
    int rs;
    int rt;
    int imm;
    int ins_count;
};

void print_inst(struct instruction* current_inst, int ALUres)
{
    printf("Current Instruction is:");
    switch(current_inst->type)
    {
        case R_type:
            printf("\t%s  %d,%d,%d\n", current_inst->mnemonic, current_inst->rd, current_inst->rs, current_inst->rt);
            if(current_inst->rd != 0)
                printf("R%d = %d ----> ", current_inst->rd, R[current_inst->rd]);
            else printf("R%d = %d ----> 0\n", current_inst->rd, R[current_inst->rd]);
            break;
        case I_type:
            if(current_inst->opcode == 8) printf("\t%s  %d,%d\n", current_inst->mnemonic, current_inst->rt, current_inst->imm);
            else if(current_inst->opcode == 12) printf("\t%s  %d,%d\n", current_inst->mnemonic, current_inst->rt, current_inst->rs);
            else if(current_inst->opcode == 11) printf("\t%s  %d,%d,%d\n", current_inst->mnemonic, current_inst->rs, current_inst->rt, current_inst->imm);
            else printf("\t%s  %d,%d,%d\n", current_inst->mnemonic, current_inst->rt, current_inst->rs, current_inst->imm);
            if(current_inst->opcode != 10 && current_inst->opcode != 11)
            {
                if(current_inst->rt != 0) printf("R%d = %d ----> ", current_inst->rt, R[current_inst->rt]);
                else printf("R0 = 0 ----> 0\n", current_inst->rt, R[current_inst->rt]);
            }
            else if(current_inst->opcode == 10) printf("Mem(%d) = %d ----> ", ALUres, mem[ALUres]);
            break;
        case J_type:
            if(current_inst->opcode != 14) printf("\t%s  %d\n", current_inst->mnemonic, current_inst->imm);
            else printf("\t%s\n", current_inst->mnemonic);
            break;
    }
}

void print_change(struct instruction* current_inst, int ALUres, int MEMres, int PC)
{
    switch(current_inst->type)
    {
        case R_type:
            if(current_inst->rd != 0) printf("%d\n", ALUres);
            printf("R%d = %d and R%d = %d\n\n", current_inst->rs, R[current_inst->rs], current_inst->rt, R[current_inst->rt]);
            break;
        case I_type:
            if(current_inst->rt != 0)
            {
                if(current_inst->opcode == 9) printf("Mem(%d) = %d\n", ALUres, MEMres);
                else if(current_inst->opcode == 10) printf("%d\n", current_inst->rt);
                else if(current_inst->opcode == 12) printf("%d\n", PC + 1);
                else if(current_inst->opcode != 11) printf("%d\n", ALUres);
            }
            printf("R%d = %d and R%d = %d and IMM = %d\n\n", current_inst->rs, R[current_inst->rs], current_inst->rt, R[current_inst->rt], current_inst->imm);
            break;
        case J_type:
            printf("Offset = %d\n\n", current_inst->imm);
            break;
    }
}

int SE(int num)
{
    num <<= 16;
    num >>= 16;
    return num;
}

int ZE(int num)
{
    unsigned int temp = (unsigned int) num;
    temp <<= 16;
    temp >>= 16;
    return temp;
}

void Loader(FILE* input_file, int* PC, int* MEM_usage)
{
    int entry, size = 0;
    struct instruction temp;
    bool first = true, flag = false;
    while(fscanf(input_file, "%d\n", &entry) != EOF) size++;
    *MEM_usage = size;
    machine_code = (int*)malloc(4 * size);
    fseek(input_file, 0, SEEK_SET);
    size = 0;
    while(fscanf(input_file, "%d", &entry) != EOF)
    {
        machine_code[size++] = entry;
        temp.opcode = entry >> 24;
        if(0 > temp.opcode || temp.opcode > 14 || entry >> 28 != 0)
        {
            mem[size - 1] = entry;
            flag = true;
        }
        else if(temp.opcode < 5)
        {
            int help = 0x0000f000;
            temp.rd = (entry & help) >> 12;
            help = 0x000f0000;
            temp.rt = (entry & help) >> 16;
            help = 0x00f00000;
            temp.rs = (entry & help) >> 20;
            help = 0x0000000f;
            if(temp.rd > 15 || temp.rt > 15 || temp.rs > 15 || (entry & help) != 0)
            {
                mem[size - 1] = entry;
                flag = true;
            }
        }
        else if(temp.opcode < 13)
        {
            int help = 0x000f0000;
            temp.rt = (entry & help) >> 16;
            help = 0x00f00000;
            temp.rs = (entry & help) >> 20;
            if(temp.rt > 15 || temp.rs > 15)
            {
                mem[size - 1] = entry;
                flag = true;
            }
        }
        else
        {
            if((entry & 0x00ff0000) != 0)
            {
                mem[size - 1] = entry;
                flag = true;
            }
        }
        if(!flag && first)
        {
            *PC = size - 1;
            first = false;
        }
        flag = false;
    }
    rewind(input_file);
    return;
}

void IF(struct instruction* current_inst, int PC)
{
    current_inst->int_of_inst = machine_code[PC];
    return;
}

int ID_part1(struct instruction* current_inst, int* Reg_read)
{
    current_inst->opcode = current_inst->int_of_inst >> 24;
    if(0 > current_inst->opcode || current_inst->opcode > 14 || current_inst->int_of_inst >> 28 != 0)
    {
        return -1;
    }
    else if(current_inst->opcode < 5)
    {
        *Reg_read = 2 + *Reg_read;
        int help = 0x0000f000;
        current_inst->rd = (current_inst->int_of_inst & help) >> 12;
        help = 0x000f0000;
        current_inst->rt = (current_inst->int_of_inst & help) >> 16;
        help = 0x00f00000;
        current_inst->rs = (current_inst->int_of_inst & help) >> 20;
        current_inst->type = R_type;
        strcpy(current_inst->mnemonic, inst[current_inst->opcode]);
        help = 0x0000000f;
        if(current_inst->rd > 15 || current_inst->rt > 15 || current_inst->rs > 15 || (current_inst->int_of_inst & help) != 0)
        {
            return -1;
        }
    }
    else if(current_inst->opcode < 13)
    {
        if(current_inst->opcode != 8) (*Reg_read)++;
        if(current_inst->opcode == 11) (*Reg_read)++;
        int help = 0x000f0000;
        current_inst->rt = (current_inst->int_of_inst & help) >> 16;
        help = 0x00f00000;
        current_inst->rs = (current_inst->int_of_inst & help) >> 20;
        help = 0x0000ffff;
        current_inst->imm = (current_inst->int_of_inst & help);
        current_inst->type = I_type;
        strcpy(current_inst->mnemonic, inst[current_inst->opcode]);
        if(current_inst->rt > 15 || current_inst->rs > 15)
        {
            return -1;
        }
    }
    else
    {
        int help = 0x0000ffff;
        current_inst->imm = (current_inst->int_of_inst & help);
        current_inst->type = J_type;
        strcpy(current_inst->mnemonic, inst[current_inst->opcode]);
        if((current_inst->int_of_inst & 0x00ff0000) != 0)
        {
            return -1;
        }
    }
    return 1;
}

int EXE(struct instruction* current_inst)
{
    switch(current_inst->opcode)
    {
        case 0:
            return R[current_inst->rs] + R[current_inst->rt];
            break;
        case 1:
            return R[current_inst->rs] - R[current_inst->rt];
            break;
        case 2:
            return R[current_inst->rs] < R[current_inst->rt] ? 1 : 0;
            break;
        case 3:
            return R[current_inst->rs] | R[current_inst->rt];
            break;
        case 4:
            return ~(R[current_inst->rs] & R[current_inst->rt]);
            break;
        case 5:
            return R[current_inst->rs] + SE(current_inst->imm);
            break;
        case 6:
            return R[current_inst->rs] < SE(current_inst->imm) ? 1 : 0;
            break;
        case 7:
            return R[current_inst->rs] | ZE(current_inst->imm);
            break;
        case 8:
            return current_inst->imm << 16;
            break;
        case 9:
        case 10:
            return R[current_inst->rs] + SE(current_inst->imm);
            break;
        case 11:
            return R[current_inst->rs] - R[current_inst->rt];
            break;
        default:
            break;
    }
}

int MEM(struct instruction* current_inst, int ALUres, int* MEM_read, int* MEM_write, int* MEM_usage)
{
    switch(current_inst->opcode)
    {
        case 9:
            (*MEM_read)++;
            if(mem_use[ALUres] == false)
            {
                (*MEM_usage)++;
                mem_use[ALUres] = true;
            }
            return mem[ALUres];
            break;
        case 10:
            (*MEM_write)++;
            if(mem_use[ALUres] == false)
            {
                (*MEM_usage)++;
                mem_use[ALUres] = true;
            }
            mem[ALUres] = R[current_inst->rt];
            return ALUres;
            break;
        default:
            return ALUres;
    }
}

void WB(struct instruction* current_inst, int MEMres, int PC, int* Reg_write)
{
    switch(current_inst->type)
    {
        case R_type:
            (*Reg_write)++;
            if(current_inst->rd != 0) R[current_inst->rd] = MEMres;
            break;
        case I_type:
            switch(current_inst->opcode)
            {
                case 10:
                case 11:
                    break;
                default:
                    (*Reg_write)++;
                    if(current_inst->rt != 0) R[current_inst->rt] = MEMres;
                    break;
            }
            break;
        case J_type:
            if(current_inst->opcode == 12 && current_inst->rt != 0) R[current_inst->rt] = PC + 1;
            break;
    }
}

void ID_part2(struct instruction* current_inst, int* PC, int MEMres)
{
    switch(current_inst->opcode)
    {
        case 11:
            *PC = MEMres == 0 ? *PC + 1 + SE(current_inst->imm) : *PC + 1;
            break;
        case 12:
            *PC = R[current_inst->rs];
            break;
        case 13:
            *PC = ZE(current_inst->imm);
            break;
        default:
            *PC = *PC + 1;
    }
}

int main(int argc, char** argv) {
    FILE* machp, * fopen();
    if (argc < 2) {
        printf("***** Please run this program as follows:\n");
        printf("***** %s.exe machprog.m\n", argv[0]);
        printf("***** where machprog.m is your machine code.\n");
        exit(1);
    }
    if ((machp = fopen(argv[1], "r")) == NULL) {
        printf("%s cannot be openned\n", argv[1]);
        exit(1);
    }
    //Start of Single Cycle CPU Code
    int temp;
    int ALUres;
    int MEMres;
    int PC;
    int Reg_read = 0, Reg_write = 0;
    int MEM_read = 0, MEM_write = 0, MEM_usage;
    char entry = 's';
    struct instruction current_inst;
    time_t start, end;
    double diff_time, total_time = 0;

    current_inst.ins_count = 0;

    dash_line;
    printf("\tLoading the Program...\n");
    Loader(machp, &PC, &MEM_usage);
    printf("\tProgram Loaded\a\n");
    printf("\tStart Address is: %d\n", PC);

    while(true)
    {
        //calculating start time
        start = clock();

        IF(&current_inst, PC);
        temp = ID_part1(&current_inst, &Reg_read);
        if(temp == -1)
        {
            PC++;
            continue;
        }
        dash_line;
        current_inst.ins_count++;
        ALUres = EXE(&current_inst);
        //Printing the instruction for better understanding
        print_inst(&current_inst, ALUres);

        MEMres = MEM(&current_inst, ALUres, &MEM_read, &MEM_write, &MEM_usage);
        //printing changes
        print_change(&current_inst, ALUres, MEMres, PC);

        WB(&current_inst, MEMres, PC, &Reg_write);
        ID_part2(&current_inst, &PC, MEMres);
        //calculating end time and difference
        end = clock();
        diff_time = (double)(end - start)/CLOCKS_PER_SEC;
        total_time += diff_time;
        printf("Time that is taken by this Instruction is: %f seconds\n", diff_time);
        dash_line;

        if(current_inst.opcode == 14) break;

        //Getting user to choose to see step by step or continue to the last
        if(entry != 'e')
        {
            printf("\n-Press 'e' to continue program execution to the end or enter any other key to continue step by step!-\n");
            scanf("%c", &entry);
        }
    }
    printf("\n\tTotally %d reads and %d writes happened from Register File.\n", Reg_read, Reg_write);
    printf("\tAlso, %d reads and %d writes happened from Main Memory.\n", MEM_read, MEM_write);
    printf("\tThis program used %d words = %d Bytes of Memory.\n", MEM_usage, MEM_usage * 4);
    printf("\n--This simulator executed %d instructions in %f seconds--\a\n\n", current_inst.ins_count, total_time);

    free(machine_code);
    fclose(machp);
    return 0;
}
