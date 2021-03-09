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
#define MONITORX_SIZE 352
#define MONITORY_SIZE 288

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
int read_imemin(unsigned int inst[], char file[]);
int read_dmemin(signed int mem[], char file[]);
int read_disk(unsigned int disk[], char file[]);
int read_irq2in(unsigned int irq2in[], char file[]);
Cmd Line_to_command(unsigned int new_command);
unsigned int get_4bits(int number, int offset);
void new_line_for_trace_file(char line[], int regs[], int PC, unsigned int inst, int immediate);
void new_line_for_hwregtrace_file(char line_for_hwregtrace[], int io_regs[], int regs[], int clk, Cmd cmd);
void new_line_for_leds_file(char line_for_leds[], int regs[], int clk, Cmd cmd);
int sign_ext(int immediate);
void create_regout_file(char file[], int regs[]);
int doing_the_command(Cmd cmd, int regs[], int PC, unsigned int mem[], int io_regs[], int reti_busy, unsigned int disk[]);
void check_disk(unsigned int disk[], int io_regs[], unsigned int mem[]);
void check_if_disk_free(int io_regs[]);
void check_irq2(unsigned int irq2[], int io_regs[], int clk);
void monitor_command(int regs[], int io_regs[], unsigned int monitor[MONITORX_SIZE][MONITORY_SIZE], Cmd cmd);
void create_monitor_file(char file[], unsigned int monitor[MONITORX_SIZE][MONITORY_SIZE],unsigned char monitor_c[]);
void create_monitor_yuv_file(char file[], unsigned char monitor_c[]);
void create_cycles_file(char file[], int clk, int inst_counter);
void create_dmemout_file(char file[], unsigned int mem[]);
void create_diskout_file(char file[], unsigned int disk[]);
Cmd interrupt_occured(int io_regs[], int inst[],int* PC_ptr, int reti_busy,Cmd cmd);
void timer(int io_regs[]);
int neg_to_2s_compliment(signed int num);
Cmd nop(Cmd cmd);
void add(int regs[], Cmd cmd);
void sub(int regs[], Cmd cmd);
void and(int regs[], Cmd cmd);
void or (int regs[], Cmd cmd);
void xor (int regs[], Cmd cmd);
void mul(int regs[], Cmd cmd);
void sll(int regs[], Cmd cmd);
void sra(int regs[], Cmd cmd);
void srl(int regs[], Cmd cmd);
int beq(int regs[], Cmd cmd, int PC);
int bne(int regs[], Cmd cmd, int PC);
int blt(int regs[], Cmd cmd, int PC);
int bgt(int regs[], Cmd cmd, int PC);
int ble(int regs[], Cmd cmd, int PC);
int bge(int* regs, Cmd cmd, int PC);
int jal(int regs[], Cmd cmd, int PC);
void lw(int regs[], Cmd cmd, unsigned int mem[]);
void sw(int regs[], Cmd cmd, unsigned int mem[]);
int reti(int io_regs[], int reti_busy);
void in(int io_regs[], int regs[], Cmd cmd);
void out(int io_regs[], int regs[], Cmd cmd, unsigned int disk[], int mem[]);

/*--------------------------------------------------------------------------------------
											IMPLEMENTATIONS
--------------------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
	int PC = 0; //initialize PC
	int* PC_ptr = &PC; 
	int clk = 0; //clk cycle counter
	int inst_counter = 0;
	int reti_busy = 0;
	int regs[NUMBER_OF_REGISTERS_SIZE] = { 0 };//initializ registers
	int io_regs[NUMBER_OF_IO_REGISTERS_SIZE] = { 0 };
	unsigned int inst[MAX_INST_MEMORY_SIZE] = { 0 }; //initialize memory
	signed int mem[MAX_MEMORY_SIZE] = { 0 };
	unsigned int disk[MAX_DISK_SIZE] = { 0 };
	unsigned int irq2[MAX_MEMORY_SIZE] = { 0 };
	unsigned int monitor[MONITORX_SIZE][MONITORY_SIZE] = { 0 };
	unsigned char monitor_c[MONITORX_SIZE*MONITORY_SIZE] = { 0 };
	unsigned int new_command;
	unsigned int immediate = 0;
	FILE *fp_trace = fopen(argv[7], "w");
	FILE *fp_hwregtrace = fopen(argv[8], "w");
	FILE *fp_leds = fopen(argv[10], "w");
	if (fp_trace == NULL || fp_hwregtrace == NULL || fp_leds == NULL)
	{
		printf("Error while opening file");
		exit(1);
	}
	if (read_imemin(inst, argv[1]) != 0 || read_dmemin(mem, argv[2]) != 0 || read_disk(disk, argv[3]) !=0|| read_irq2in(irq2, argv[4]) !=0)
	{
		printf("Error while opening files");
		exit(1);
	}
	while (PC != -1)// as long as there ins't HALT command
	{
		if (PC < MAX_INST_MEMORY_SIZE)
			new_command = inst[PC];
		Cmd cmd = Line_to_command(new_command);
		if (cmd.immediate == 1)
		{
			if (PC + 1 < MAX_INST_MEMORY_SIZE)
			{
				immediate = inst[PC + 1];
				regs[1] = sign_ext(immediate);
			}
		}
		if ((io_regs[0] && io_regs[3]) || (io_regs[1] && io_regs[4]) || (io_regs[2] && io_regs[5])) //irq==1
		{
			cmd = interrupt_occured(io_regs,inst, PC_ptr, reti_busy,cmd);
			if (PC < MAX_INST_MEMORY_SIZE)
				new_command = inst[PC];
			if (cmd.immediate == 1)
			{
				if (PC + 1 < MAX_INST_MEMORY_SIZE)
				{
					immediate = inst[PC + 1];
					regs[1] = sign_ext(immediate);
				}
			}
		}
		timer(io_regs); //checks if timer is enabled
		check_irq2(irq2, io_regs,clk); // checks if irq2 id triggered
		check_if_disk_free(io_regs); //checks if disk is done
		char line_to_trace_file[200];
		new_line_for_trace_file(line_to_trace_file, regs, PC, new_command, immediate);//creating new line for trace file
		fprintf(fp_trace, "%s\n", line_to_trace_file);//adding the new line to trace file
		if (cmd.immediate == 1) // if it's a command with immediate, increment the clock and pc
		{
			PC++;
			io_regs[8] = clk++;
			if (io_regs[5] == 0)
				check_irq2(irq2, io_regs, clk);
		}
		if (cmd.opcode == 19 || cmd.opcode == 20) // in or out instruction
		{
			char line_for_hwregtrace[200];
			new_line_for_hwregtrace_file(line_for_hwregtrace, io_regs, regs, clk, cmd);// creating new line for hwregtrace file
			fprintf(fp_hwregtrace, "%s\n", line_for_hwregtrace); // adding the new line to hwregtrace file
		}
		if (cmd.opcode == 20 && (regs[cmd.rs] + regs[cmd.rt] == 9)) //a led is on/off 
		{
			char line_for_leds[200];
			new_line_for_leds_file(line_for_leds, regs, clk, cmd);// creating new line for leds file
			fprintf(fp_leds, "%s\n", line_for_leds); // adding the new line to leds file
		}
		PC = doing_the_command(cmd, regs, PC, mem, io_regs, reti_busy, disk);
		inst_counter++;
		if ((cmd.opcode == 19 || cmd.opcode == 20) && (regs[cmd.rs] + regs[cmd.rt] == 18)) //monitor command
		{
			monitor_command(regs, io_regs, monitor, cmd);
		}
		io_regs[8] = clk++;
	}
	create_regout_file(argv[6],regs);
	create_monitor_file(argv[11],monitor,monitor_c);
	create_monitor_yuv_file(argv[12], monitor_c);
	create_cycles_file(argv[9], clk, inst_counter);
	create_dmemout_file(argv[5], mem);
	create_diskout_file(argv[13], disk);
	fclose(fp_trace);
	fclose(fp_hwregtrace);
	fclose(fp_leds);
	return 0;
}

int read_imemin(unsigned int inst[], char file[])//reads the imemin.txt file and puts it into array
{
	FILE *fp_imemin = fopen(file, "r"); //open the file
	if (fp_imemin == NULL)//handling error in reading the file 
	{
		printf("Error while opning imemin file\n");
		return 1;
	}
	char line[MAX_COMMAND_SIZE];
	int i = 0;
	while (feof(fp_imemin) == 0 && fgets(line, MAX_COMMAND_SIZE, fp_imemin) != NULL)
	{
		if (strcmp(line, "\n") == 0 || strcmp(line, "\0") == 0) // ignore the white spaces
			continue;
		inst[i] = strtol(line, NULL, 16); //convert string to to long integer of base 16
		i++;
	}
	fclose(fp_imemin); //close the file
	return 0;
}

int read_dmemin(signed int mem[], char file[])
{
	FILE *fp_dmemin = fopen(file, "r"); //open the file
	if (fp_dmemin == NULL)//handling error in reading the file 
	{
		printf("Error while opning dmemin file\n");
		return 1;
	}
	char line[MAX_MEMORY_LINE];
	int i = 0;
	while (!feof(fp_dmemin) && fgets(line, MAX_MEMORY_LINE, fp_dmemin))
	{
		if (strcmp(line, "\n") == 0 || strcmp(line, "\0") == 0) // ignore the white spaces
			continue;
		sscanf(line, "%x", &mem[i]);
		i++;
	}
	fclose(fp_dmemin); //close the file
	return 0;
}

int read_disk(unsigned int disk[], char file[])
{
	FILE *fp_disk = fopen(file, "r"); //open the file
	if (fp_disk == NULL)//handling error in reading the file 
	{
		printf("Error while opning diskin file\n");
		return 1;
	}
	char line[MAX_MEMORY_LINE];
	int i = 0;
	while (feof(fp_disk) == 0 && fgets(line, MAX_MEMORY_LINE, fp_disk) !=NULL)
	{
		if (strcmp(line, "\n") == 0 || strcmp(line, "\0") == 0) // ignore the white spaces
			continue;
		sscanf(line, "%x", &disk[i]);
		i++;
	}
	fclose(fp_disk); //close the file
	return 0;
}

int read_irq2in(unsigned int irq2in[], char file[])
{
	FILE *fp_irq2in = fopen(file, "r"); //open the file
	if (fp_irq2in == NULL)//handling error in reading the file 
	{
		printf("Error while opning irq2in file\n");
		return 1;
	}
	char line[MAX_MEMORY_LINE];
	int i = 0;
	while (feof(fp_irq2in) == 0 && fgets(line, MAX_MEMORY_LINE, fp_irq2in) != NULL)
	{
		if (strcmp(line, "\n") == 0 || strcmp(line, "\0") == 0) // ignore the white spaces
			continue;
		irq2in[strtol(line, NULL, 10)] = 1;//builds an array where there is 1 in the number of cycle that has interrupt
		i++;
	}
	fclose(fp_irq2in); //close the file
	return 0;
}

void check_irq2(unsigned int irq2[], int io_regs[], int clk)
{
	if ((clk<MAX_MEMORY_SIZE) && (irq2[clk] == 1)) //irq2 is triggered
		io_regs[5] = 1;
	else
		io_regs[5] = 0;
}

Cmd Line_to_command(unsigned int new_command) //converting line into command structure
{
	Cmd command;
	command.rt = get_4bits(new_command, 0);
	command.rs = get_4bits(new_command, 1);
	command.rd = get_4bits(new_command, 2);
	command.opcode = get_4bits(new_command, 3) + (16 * get_4bits(new_command, 4));
	if (command.rt == 1 || command.rs == 1 || command.rd == 1) 
		command.immediate = 1; // instruction with immediate
	else
		command.immediate = 0;
	//handeling illegal opcode or registers
	if (command.opcode > 21) // if it's an illegal opcode
		command = nop(command);
	if (command.opcode < 9 || command.opcode == 16 || command.opcode == 19)//if it's an arithmetic/lw/in opcode
	{
		if (command.rd == 1)
			command = nop(command);
	}

	return command;
}

unsigned int get_4bits(int number, int offset) // returns 1 hex number
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
		char new_reg_char [9];
		int new_reg_num = 0;
		if (i == 1)//for immediate 
		{
			sprintf(new_reg_char, "%08X", sign_ext(immediate));//conveting the sign extended immediate to hex
			sprintf(line + strlen(line), new_reg_char);//writing in the line
			sprintf(line + strlen(line), " ");
			continue;
		}
		if (regs[i] < 0)
			new_reg_num = neg_to_2s_compliment(regs[i]);
		else
			new_reg_num = regs[i];
		sprintf(new_reg_char, "%08X", new_reg_num);//conveting to hex
		sprintf(line + strlen(line), new_reg_char);//writing in the line
		if (i <= 14)
			sprintf(line + strlen(line), " "); 
	}
}

void new_line_for_hwregtrace_file(char line_for_hwregtrace[], int io_regs[], int regs[], int clk, Cmd cmd)
{
	char clk_char[10];
	char new_reg_char[10];
	int new_reg_num = 0;
	sprintf(clk_char, "%d", clk);
	sprintf(line_for_hwregtrace, clk_char); //add clock cycle to line
	sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), " "); // add white space
	if (cmd.opcode == 19) // in instraction  
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "READ "); //add read to line
	if (cmd.opcode == 20) // out instraction
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "WRITE ");//add write to line
	switch (regs[cmd.rs] + regs[cmd.rt]) // cases of diffrent IORegs number and add the register name to line 
	{
	case 0:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irq0enable "); 
		break;
	}
	case 1:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irq1enable "); 
		break;
	}
	case 2:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irq2enable "); 
		break;
	}
	case 3:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irq0status ");
		break;
	}
	case 4:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irq1status "); 
		break;
	}
	case 5:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irq2status ");
		break;
	}
	case 6:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irqhandler "); 
		break;
	}
	case 7:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "irqreturn ");
		break;
	}
	case 8:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "clks ");
		break;
	}
	case 9:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "leds ");
		break;
	}
	case 10:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "reserved ");
		break;
	}
	case 11:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "timerenable ");
		break;
	}
	case 12:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "timercurrent ");
		break;
	}
	case 13:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "timermax ");
		break;
	}
	case 14:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "diskcmd ");
		break;
	}
	case 15:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "disksector ");
		break;
	}
	case 16:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "diskbuffer ");
		break;
	}
	case 17:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "diskstatus ");
		break;
	}
	case 18:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "monitorcmd ");
		break;
	}
	case 19:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "monitorx ");
		break;
	}
	case 20:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "monitory ");
		break;
	}
	case 21:
	{
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), "monitordata ");
		break;
	}
	}
	if (cmd.opcode == 19) // in instraction
	{
		new_reg_num = io_regs[regs[cmd.rs] + regs[cmd.rt]];
		sprintf(new_reg_char, "%08X",new_reg_num); // convrets to 8 hex 
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), new_reg_char); //adds read data to line
	}
	if (cmd.opcode ==20) // ouot instraction
	{
		new_reg_num = regs[cmd.rd];
		sprintf(new_reg_char, "%08X", new_reg_num);
		sprintf(line_for_hwregtrace + strlen(line_for_hwregtrace), new_reg_char); //adds the written data to line
	}
}

void new_line_for_leds_file(char line_for_leds[], int regs[], int clk, Cmd cmd)
{
	char cycle[10];
	char leds_so_far[10];
	sprintf(cycle, "%d", clk); 
	sprintf(leds_so_far, "%08X", regs[cmd.rd]);
	sprintf(line_for_leds, cycle); // adding current cycle number to the new line
	sprintf(line_for_leds + strlen(line_for_leds), " "); // adding space to new line
	sprintf(line_for_leds + strlen(line_for_leds), leds_so_far);// adding current leds to line
}


int sign_ext(int immediate)
{
	int value = (0x000FFFFF & immediate);
	int mask = 0x00080000;
	if (mask & immediate) {
		value += 0xFFF00000;
	}
	return value;
}

void create_regout_file(char file[],int regs[])
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

int reti(int io_regs[], int reti_busy)// flag to know the status of reti 
{
	if (reti_busy = 1)//reti is busy
		reti_busy = 0;//reti is done
	if (reti_busy =0)
		reti_busy = 1; //reti is busy now
	return io_regs[7]; //return PC of the return address
}

//in command
void in(int io_regs[], int regs[], Cmd cmd)
{
	if (regs[cmd.rs] + regs[cmd.rt] < 22)
		regs[cmd.rd] = io_regs[regs[cmd.rs] + regs[cmd.rt]];
}

// out command
void out(int io_regs[], int regs[], Cmd cmd, unsigned int disk[], int mem[])
{
	if (regs[cmd.rs] + regs[cmd.rt] < 22)
		if ((regs[cmd.rs] + regs[cmd.rt]) == 14)
		{
			io_regs[14] = regs[cmd.rd];
			check_disk(disk, io_regs, mem);
		}
		else
			io_regs[regs[cmd.rs] + regs[cmd.rt]] = regs[cmd.rd];
}

int doing_the_command(Cmd cmd, int regs[],int PC, unsigned int mem[], int io_regs[], int reti_busy, unsigned int disk[])
{
	switch (cmd.opcode)
	{
	case 0: //add 
	{
		add(regs, cmd);
		regs[0] = 0;
			PC++;
		break;
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
	case 16: //lw
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
		PC = reti(io_regs, reti_busy);
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
			io_regs[3] = 1;
			io_regs[12] = 0;
		}
		else
		{
			io_regs[3] = 0;
			io_regs[12] ++;
		}
	}
}

void check_disk(unsigned int disk[], int io_regs[], unsigned int mem[])
{
	if (io_regs[17] == 0)//disk is free to receive new command
	{
		io_regs[11] = 1; //timer enable
		io_regs[13] = 1024; //set max timer value to 1024
		io_regs[17] = 1; //disk is now busy handaling a read/write command
		switch (io_regs[14])
		{
		case 0: //no command
		{
			io_regs[11] = 0; //timer disabled
			io_regs[13] = 0; //max timer value is 0
			io_regs[17] = 0; //disk is free to recive new command
		}
		case 1: //read sector
		{
			int i = 0;
			for (i = 0; i < 128; i++) //128 words to read
			{
				//check if there is no overflew in the disk or memory. if there isn't, write.
				if ((((128 * io_regs[15]) + i) < MAX_DISK_SIZE) && (io_regs[16] + i < MAX_MEMORY_SIZE))
					mem[io_regs[16] + i] = disk[(128 * io_regs[15]) + i]; 
			}
			break;
		}
		case 2: //write sector
		{
			int i = 0;
			for (i = 0; i < 128; i++) //128 words to read
			{
				//check if there is no overflew in the disk or memory. if there isn't, read.
				if ((((128 * io_regs[15]) + i) < MAX_DISK_SIZE) && (io_regs[16] + i < MAX_MEMORY_SIZE))
					disk[(128 * io_regs[15]) + i] = mem[io_regs[16] + i] ;
			}
			break;
		}
		}
	}
	if (io_regs[3] == 1) //timer is trigged. corrent timer value = 1024
	{
		io_regs[14] = 0;
		io_regs[17] = 0;
		io_regs[4] = 1; // disk interaption. disk is done doing is job
		// reset timer
		io_regs[3] = 0;
		io_regs[11] = 0;
	}
	else
		io_regs[4] = 0;
}

void check_if_disk_free(int io_regs[])
{
	if (io_regs[3] == 1 && io_regs[0]!= 1) //timer is trigged. corrent timer value = 1024, disk is done.
	{
		io_regs[14] = 0;
		io_regs[17] = 0;
		io_regs[4] = 1; // disk interaption. disk is done doing is job
		// reset timer
		io_regs[3] = 0;
		io_regs[11] = 0;
	}
	else
		io_regs[4] = 0;
}

void monitor_command(int regs[], int io_regs[], unsigned int monitor[MONITORX_SIZE][MONITORY_SIZE], Cmd cmd)
{
	if (cmd.opcode == 20) //out instraction
	{
		if (io_regs[18] == 1)
		{
			if ((io_regs[19] <= MONITORX_SIZE) && (io_regs[20] <= MONITORY_SIZE) && (0 <= io_regs[21] <= 255))
			{
				monitor[io_regs[20]][io_regs[19]] = io_regs[21];
					io_regs[18] = 0;
			}
		}
	}
	if (cmd.opcode == 19) //in insraction
		io_regs[18] = 0;
}

void create_monitor_file(char file[], unsigned int monitor[MONITORX_SIZE][MONITORY_SIZE],unsigned char monitor_c[])
{
	FILE* fp_monitor = NULL;
	fp_monitor = fopen(file, "w");
	int k = 0;
	if (fp_monitor == NULL)//handling error in writing to the file 
	{
		printf("Error while writing monitor file\n");
		exit(1);
	}
	for (int i = 0; i < MONITORY_SIZE; i++)
	{
		for (int j = 0; j < MONITORX_SIZE; j++) {
			fprintf(fp_monitor, "%02X\n", monitor[i][j]);
			monitor_c[k++] = monitor[i][j]; //create an array of char from the monitor int matrix for the monitor_yuv file
		}
	}
	fclose(fp_monitor);
}

void create_monitor_yuv_file(char file[], unsigned char monitor_c[])
{
	FILE* fp_monitor_yuv = NULL;
	fp_monitor_yuv = fopen(file, "wb");
	if (fp_monitor_yuv == NULL)//handling error in writing to the file 
	{
		printf("Error while writing monitor_yuv file\n");
		exit(1);
	}
	fwrite(monitor_c, 1,MONITORX_SIZE*MONITORY_SIZE, fp_monitor_yuv);
	fclose(fp_monitor_yuv);
}

void create_cycles_file(char file[], int clk, int inst_counter)
{
	FILE* fp_cycles = NULL;
	fp_cycles = fopen(file, "w");
	char cycle[10];
	char inst[10];
	if (fp_cycles == NULL)//handling error in writing to the file 
	{
		printf("Error while writing cycles file\n");
		exit(1);
	}
	sprintf(cycle, "%d\n", clk);
	sprintf(inst, "%d", inst_counter);
	fputs(cycle, fp_cycles);
	fputs(inst, fp_cycles);
	fclose(fp_cycles);
}

void create_dmemout_file(char file[], unsigned int mem[])
{
	int i = 0;
	FILE* fp_dmemout;
	fp_dmemout = fopen(file, "w");
	if (fp_dmemout == NULL)//handling error in writing to the file 
	{
		printf("Error while writing dmemout file\n");
		exit(1);
	}
	for (i = 0; i < MAX_MEMORY_SIZE; i++)
	{
		if (i < MAX_MEMORY_SIZE-1)
			fprintf(fp_dmemout, "%08X\n", *mem);
		else
			fprintf(fp_dmemout, "%08X", *mem);
		mem++; //get the next memory line
	}
	fclose(fp_dmemout);//close the file
}

void create_diskout_file(char file[], unsigned int disk[])
{
	int i = 0;
	FILE* fp_diskout;
	fp_diskout = fopen(file, "w");
	if (fp_diskout == NULL)//handling error in writing to the file 
	{
		printf("Error while writing diskout file\n");
		exit(1);
	}
	for (i = 0; i < MAX_MEMORY_SIZE; i++)
	{
		fprintf(fp_diskout, "%08X\n", *disk);
		disk++; //get the next memory line
	}
	fclose(fp_diskout);//close the file
}

Cmd interrupt_occured(int io_regs[],int inst[], int* PC_ptr, int reti_busy, Cmd cmd)
{
	if (reti_busy == 0) //we are done with the last interapt
	{
		int new_command = 0;
		io_regs[7] = *PC_ptr; //current PC is saved in irqreturn
		*PC_ptr = io_regs[6]; //the new PC is the one in irqhandler
		new_command = inst[io_regs[6]];
		return cmd = Line_to_command(new_command); //return the new command
	}
	return cmd; //if we're not done with the last interpt, don't jump
}

int neg_to_2s_compliment(signed int num)
{
	int pos_num = 0 ;
	pos_num = abs(num);
	signed int mask = 0xffffffff;
	pos_num = pos_num ^ mask; //inverting all bits
	pos_num++; //adding 1 
	return pos_num;
}

Cmd nop(Cmd cmd)
{
	cmd.opcode = 0;
	cmd.rd = 0;
	cmd.rs = 0;
	cmd.rt = 0;
	cmd.immediate = 0;
	return cmd;
}