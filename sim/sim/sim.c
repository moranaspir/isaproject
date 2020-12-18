/*
sim.c
*/

/*--------------------------------------------------------------------------------------
											INCLUDES
--------------------------------------------------------------------------------------*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*--------------------------------------------------------------------------------------
											DEFINES
--------------------------------------------------------------------------------------*/
#define MAX_COMMAND_SIZE 6 //5 hex lettets
#define MAX_MEMORY_LINE 9
#define MAX_INST_MEMORY_SIZE 1024
#define MAX_MEMORY_SIZE 4096
#define MAX_DISK_SIZE 16384 
#define NUMBER_OF_REGISTERS_SIZE 16
#define NUMBER_OF_IO_REGISTERS_SIZE 22


typedef struct _cmd {
	unsigned int opcode;
	unsigned int rd;
	unsigned int rs;
	unsigned int rt;
	unsigned int immediate;
}Cmd;
/*--------------------------------------------------------------------------------------
											DECLARATIONS
--------------------------------------------------------------------------------------*/
int read_imemin(unsigned int mem[], char adress[]);
Cmd Line_to_command(unsigned int new_command);
unsigned int get_4bits(int number, int offset);
void new_line_for_trace_file(char line[], int regs[], int PC, unsigned int inst, int immediate);
int sign_ext(int immediate);
void creat_regout(char file[], int regs[]);
void doing_the_command(Cmd cmd, int regs[], int PC, unsigned mem[], int io_regs[], int reti_flag, int disk[]);
/*--------------------------------------------------------------------------------------
											IMPLEMENTATIONS
--------------------------------------------------------------------------------------*/

int main(int argc, char* argv)
{
	int PC = 0; //initialize PC
	int clk = 0; //clk cycle counter
	int regs[NUMBER_OF_REGISTERS_SIZE];//initializ registers
	int io_regs[NUMBER_OF_IO_REGISTERS_SIZE];
	unsigned int inst[MAX_INST_MEMORY_SIZE] = { 0 }; //initialize memory
	unsigned int mem[MAX_MEMORY_SIZE] = { 0 };
	unsigned int disk[MAX_DISK_SIZE] = { 0 };
	unsigned int irq2[MAX_MEMORY_SIZE] = { 0 };

	unsigned int new_command;
	unsigned int immediate;
	FILE *fp_trace = NULL;
	fp_trace = fpoen(argv[6], "w");

	if (read_imemin(inst, argv[1]) != 0 || read_dmemin(mem, argv[2]) != 0 || read_disk(disk, argv[3]) !=0|| read_irq2(irq2, argv[4]))
	{
		printf("couldn't open the imemine - exiting the program ");
		exit(1);
	}
	while (PC != -1)// as long as there ins't HALT command
	{
		if (PC <= MAX_INST_MEMORY_SIZE - 1) {
			new_command = inst[PC];
		}
		Cmd cmd = Line_to_command(new_command);
		if (cmd.immediate == 1)
		{
			immediate = inst[PC + 1];
			PC++;
		}
		regs[1] = sign_ext(immediate);
		timer(io_regs);
		char line_to_trace_file[143];
		new_line_for_trace_file(line_to_trace_file, regs, PC, new_command, immediate);//creating new line for trace file
		fprintf(fp_trace, "%s\n", line_to_trace_file);//adding the new line to trace file
		doing_the_command(cmd, regs, PC, mem, io_regs, reti_flag, disk);
		io_regs[8] = clk++;
	}
	creat_regout(regs,argv[5]);
}

int read_imemin(unsigned int inst[], char file[])
{
	FILE *fp_imemin = fopen(file, "r"); //open the file
	if (fp_imemin == NULL)//handling error in reading the file 
	{
		printf("Error while opning imemine file\n");
		return 1;
	}
	char line[MAX_COMMAND_SIZE];
	int i = 0;
	while (feof(fp_imemin) != 0 && fgets(line, MAX_COMMAND_SIZE, fp))
	{
		if (strcmp(line, "\n") == 0 || strcmp(line, "\0") == 0) // ignore the white spaces
			continue;
		inst[i] = strtol(line, NULL, 16); //convert string to to long integer of base 16
		i++;
	}
	fclose(fp); //close the file
	return 0;
}

Cmd Line_to_command(unsigned int new_command) //converting line into command structure
{
	Cmd command;
	command.rt = get_4bits(new_command, 0);
	command.rs = get_4bits(new_command, 1);
	command.rd = get_4bits(new_command, 2);
	command.opcode = get_4bits(new_command, 3) + (16 * get_4bits(new_command, 4));
	if (command.rt == 1 || command.rs == 1)
	{
		command.immediate = 1;
	}
	command.immediate = 0;
}

unsigned int get_4bits(int number, int offset)
{
	unsigned int mask = 0Xf << (offset * 4);
	return (number & mask) >> (offset * 4);
}

void new_line_for_trace_file(char line[], int regs[], int PC, unsigned int inst, int immediate)
{
	int i = 0;
	char pc_hex[4];
	char inst_hex[6];
	sprintf(pc_hex, "%03X", PC); //convering PC to hex
	sprintf(inst_hex, "%05X", inst); //converting inst to hex
	//writing the new line
	sprintf(line, pc_hex);
	sprintf(line + strlen(line), " ");
	sprintf(line + strlen(line), inst_hex);
	sprintf(line + strlen(line), " ");
	//adding the registres 
	for (i = 0; i <= 15; i++)
	{
		char new_reg_char = [9];
		int new_reg_num = 0;
		if (i == 1)//for immediate
		{
			sprintf(new_reg_char, "%08X", sign_ext(immediate));//conveting the sign extended immediate to hex
			sprintf(line + strlen(line), new_reg_char);//writing in the line
			sprintf(line + strlen(line), " ");
			continue;
		}
		new_reg_num = regs[i];
		sprintf(new_reg_char, "%08X", new_reg_num);//conveting to hex
		sprintf(line + strlen(line), new_reg_char);//writing in the line
		sprintf(line + strlen(line), " "); 
	}
}

int sign_ext(int immediate)
{
	int value = (0x00000FFF & immediate);
	int mask = 0x00000800;
	if (mask & immediate){
		value += 0xFFFFF000;
	}
	return value;
}

void creat_regout(char file[],int regs[])
{
	int i = 0;
	FILE* fp_regout;
	fp_regout = fopen(file, "w");
	if (fp_regout == NULL)//handling error in writing to the file 
	{
		printf("Error while writing regout file\n");
		exit(1);
	}
	for (i = 2; i <= 15; i++) { //print registers 2-15
		fprintf(fp_regout, "%08X\n" ,regs[i]);
	}
	fclose(fp_regout);//close the file
}

//instructions 

void add(int regs[], Cmd cmd)
{
	regs[cmd.rd] = regs[cmd.rs] + regs[cmd.rt];
}

void sub(int regs[], Cmd cmd)
{
	regs[cmd.rd] = regs[cmd.rs] - regs[cmd.rt];
}

void and(int regs[], Cmd cmd)
{
	regs[cmd.rd] = regs[cmd.rs] & regs[cmd.rt];
}

void or (int regs[], Cmd cmd)
{
	regs[cmd.rd] = regs[cmd.rs] | regs[cmd.rt];
}

void xor (int regs[], Cmd cmd)
{
	regs[cmd.rd] = regs[cmd.rs] ^ regs[cmd.rt];
}

void mul (int regs[], Cmd cmd)
{
	regs[cmd.rd] = regs[cmd.rs] * regs[cmd.rt];
}

void sll(int regs[], Cmd cmd)
{
	regs[cmd.rd] = regs[cmd.rs] << regs[cmd.rt];
}

void sra(int regs[], Cmd cmd)
{
	int mask = regs[cmd.rs] >> 31 << 31 >> (regs[cmd.rt]) << 1;
	regs[cmd.rd] = mask ^ (regs[cmd.rs] >> regs[cmd.rt]);
}

void srl(int regs[], Cmd cmd)
{
	regs[cmd.rd] = regs[cmd.rs] >> regs[cmd.rt];
}

int beq(int regs[], Cmd cmd, int PC)
{
	if (regs[cmd.rs] == regs[cmd.rt])
		return PC = regs[cmd.rd];
	else
	{
		PC++;
		return PC;
	}
}

int bne(int regs[], Cmd cmd, int PC)
{
	if (regs[cmd.rs] != regs[cmd.rt])
		return PC = regs[cmd.rd];
	else
	{
		PC++;
		return PC;
	}
}

int blt(int regs[], Cmd cmd, int PC)
{
	if (regs[cmd.rs] < regs[cmd.rt])
		return PC = regs[cmd.rd];
	else
	{
		PC++;
		return PC;
	}
}

int bgt(int regs[], Cmd cmd, int PC)
{
	if (regs[cmd.rs] > regs[cmd.rt])
		return PC = regs[cmd.rd];
	else 
	{
		PC++;
		return PC;
	}
}

int ble(int regs[], Cmd cmd, int PC)
{
	if (regs[cmd.rs] <= regs[cmd.rt])
		return PC = regs[cmd.rd];
	else
	{
		PC++;
		return PC;
	}
}

int bge(int* regs, Cmd cmd, int PC)
{
	if (regs[cmd.rs] >= regs[cmd.rt])
		return PC = regs[cmd.rd];
	else
	{
		PC++;
		return PC;
	}
}

int jal(int regs[], Cmd cmd, int PC)
{
	regs[15] = PC + 1;
	return PC = regs[cmd.rd];
}

void lw(int regs[], Cmd cmd, unsigned int mem[])
{
	if (regs[cmd.rs] + regs[cmd.rt] < MAX_MEMORY_SIZE)
		regs[cmd.rd] = mem[regs[cmd.rs] + regs[cmd.rt]];
}

void sw(int regs[], Cmd cmd, unsigned int mem[])
{
	if (regs[cmd.rs] + regs[cmd.rt] < MAX_MEMORY_SIZE)
		mem[regs[cmd.rs] + regs[cmd.rt]] = regs[cmd.rd];
}

int reti(int io_regs[], int PC, int* reti_flag)// flag to know the status of reti 
{
	if (*reti_flag != 0)
		*reti_flag = 0;
	else
		*reti_flag = 1;
	return io_regs[7];
}

//in command
void in(int io_regs[], int regs[], Cmd cmd)
{
	if (regs[cmd.rs] + regs[cmd.rt] < 22)
		regs[cmd.rd] = io_regs[regs[cmd.rs] + regs[cmd.rt]];
}

// out command
void out(int io_regs[], int regs[], Cmd cmd, int disk[], int mem[])
{
	if (regs[cmd.rs] + regs[cmd.rt] < 22)
		if ((regs[cmd.rs] + regs[cmd.rt]) == 14)
		{
			io_regs[14] = regs[cmd.rd];
			disk_handel(disk, io_regs, mem);
		}
		else
			io_regs[regs[cmd.rs] + regs[cmd.rt]] = regs[cmd.rd];
}

void doing_the_command(Cmd cmd, int regs[],int PC, unsigned mem[], int io_regs[], int reti_flag, int disk[])
{
	switch (cmd.opcode)
	{
	case 0: //add 
	{
		if (cmd.rd == 0 && cmd.rs == 0 && cmd.rt == 0 && cmd.immediate == 0)
			break;
		else {
			add(regs, cmd);
			regs[0] = 0; // make sure $zero is zero
			PC++;
			break;
		}
	}
	case 1: //sub
	{
		sub(regs, cmd);
		regs[0] = 0;
		PC++;
		break;
	}
	case 2: //and
	{
		and (regs, cmd);
		regs[0] = 0;
		PC++;
		break;
	}
	case 3://or
	{
		or (regs, cmd);
		regs[0] = 0;
		PC++;
		break;
	}
	case 4://xor
	{
		xor (regs, cmd);
		regs[0] = 0;
		PC++;
		break;
	}
	case 5://mull
	{
		mul(regs, cmd);
		regs[0] = 0;
		PC++;
		break;
	}
	case 6: //sll 
	{
		sll(regs, cmd);
		regs[0] = 0;
		PC++;
		break;
	}
	case 7: //sra 
	{
		sra(regs, cmd);
		regs[0] = 0;
		PC++;
		break;
	}
	case 8: //srl
	{
		srl(regs, cmd);
		regs[0] = 0;
		PC++;
		break;
	}
	case 9: //beq 
	{
		PC = beq(regs, cmd, PC);
		regs[0] = 0;
		break;
	}
	case 10: //bne
	{
		PC = bne(regs, cmd, PC);
		regs[0] = 0;
		break;
	}
	case 11: //blt
	{
		PC = blt(regs, cmd, PC);
		regs[0] = 0;
		break;
	}
	case 12: //bgt
	{
		PC = bgt(regs, cmd, PC);
		regs[0] = 0;
		break;
	}
	case 13: //ble
	{
		PC = ble(regs, cmd, PC);
		regs[0] = 0;
		break;
	}
	case 14: //bge 
	{
		PC = bge(regs, cmd, PC);
		regs[0] = 0;
		break;
	}
	case 15: //jal 
	{
		PC = jal(regs, cmd, PC);
		regs[0] = 0;
		break;
	}
	case 16: //
	{
		lw(regs, cmd, mem);
		regs[0] = 0;
		PC++;
		break;
	}
	case 17: //sw
	{
		sw(regs, cmd, mem);
		regs[0] = 0;
		PC++;
		break;
	}
	case 18: //reti
	{
		PC = reti(io_regs, PC, reti_flag);
		break;
	}
	case 19://in
	{
		in(io_regs, regs, cmd);
		regs[0] = 0;
		PC++;
		break;
	}
	case 20://
	{
		out(io_regs, regs, cmd, disk, mem);
		regs[0] = 0;
		PC++;
		break;
	}
	case 21: //halt - exit the simulator
	{
		PC = -1;
		break;
	}
	}
	return PC;
}

void timer(int io_regs[])
{
	if (io_regs[11] == 1)//timer enabled
	{
		if (io_regs[12] == io_regs[13])//max timer value
		{
			io_regs[12] = 0;
		}
		else
			io_regs[12] ++;
	}
}