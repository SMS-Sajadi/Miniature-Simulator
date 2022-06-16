#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define R_type 1
#define I_type 2
#define J_type 3

#define dash_line printf("------------------------------------------------\n")

const char* inst[] = {"add","sub","slt","or","nand","addi","slti","ori","lui","lw","sw","beq","jalr","j","halt"};

int mem[16000];
int* machine_code;

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

void Loader(FILE* input_file, int* PC)
{
    int entry, size = 0;
    struct instruction temp;
    //change to bool
    int first = 1, flag = 0;
    while(fscanf(input_file, "%d\n", &entry) != EOF) size++;
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
            flag = 1;
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
                flag = 1;
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
                flag = 1;
            }
        }
        else
        {
            if((entry & 0x00ff0000) != 0)
            {
                mem[size - 1] = entry;
                flag = 1;
            }
        }
        if(!flag && first)
        {
            *PC = size - 1;
            first = 0;
        }
        flag = 0;
    }
    rewind(input_file);
    return;
}

void IF(struct instruction* current_inst, int* PC)
{
    current_inst->int_of_inst = machine_code[*PC];
    return;
}

int ID_part1(struct instruction* current_inst)
{
    current_inst->opcode = current_inst->int_of_inst >> 24;
    if(0 > current_inst->opcode || current_inst->opcode > 14 || current_inst->int_of_inst >> 28 != 0)
    {
        return -1;
    }
    else if(current_inst->opcode < 5)
    {
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

void EXE(struct instruction* current_inst)
{

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
    int PC;
    struct instruction current_inst;

    current_inst.ins_count = 0;

    printf("loading\n");
    Loader(machp, &PC);
    printf("loading done\n");
    printf("Start Address is: %d", PC);

    while(1)
    {
        IF(&current_inst, &PC);
        temp = ID_part1(&current_inst);
        if(temp == -1)
        {
            PC++;
            continue;
        }
        EXE(&current_inst);
    }

    fclose(machp);
    return 0;
}
