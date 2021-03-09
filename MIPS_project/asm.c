#define _CRT_SECURE_NO_DEPRECATE
#define MAX_LINE 500
#define MAX_LABEL 50
#define NUM_OF_OPCODE 22
#define NUM_OF_REG 16

#define MEM_SIZE 4096
#define MEM_LINE 8 


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h> 
#include <malloc.h>
#include <stdbool.h>

const char* opcodesTable[] = { "add", "sub", "and", "or", "xor", "mul", "sll", "sra", "srl", "beq", "bne", "blt", "bgt", "ble", "bge", "jal", "lw", "sw", "reti", "in", "out", "halt" };
const char* regTable[] = { "$zero", "$imm", "$v0", "$a0", "$a1", "$t0", "$t1", "$t2", "$t3", "$s0", "$s1", "$s2", "$gp", "$sp", "$fp", "$ra" };
int word_table[1024][2];
int word_table_pos = 0;

typedef struct Label {
	char name[MAX_LABEL];
	int addr;
} label;

typedef struct Commands {
	int Opcode;
	int rd;
	int rs;
	int rt;
	int extra;
	int label_addr;
} command;

//Functions declartion
void readline(FILE* inputFile, char* line);
void lineToCommand(char* line, command* com, int label_size, label* label_table, FILE* outputFile_1, FILE* outputFile_2);
bool splitLine(char* line, char* opcode, char* rd, char* rs, char* rt, char* extra);
int getOpcode(char* opcode);
int getReg(char* reg);
label* build_label_table(char *argv[], int* table_size);
bool is_label(char *line, char *line_afte_label);
int get_label_addr(char* extra, label* label_table, int label_size);
void add_to_dmemin(FILE *outputFile_2);

void main(int argc, char *argv[])
{
	label* label_table = NULL;
	int label_size = 0;
	FILE* inputFile = NULL;
	FILE* outputFile_1 = NULL;
	FILE* outputFile_2 = NULL;
	char line[MAX_LINE + 1];
	char line_afte_label[MAX_LINE + 1];
	command com;

	if (argc != 4) // Check num of  input arguments
	{
		printf("Some arguments are missing");
		return;
	}
	label_table = build_label_table(argv, &label_size); //First running for mapping label address
	outputFile_1 = fopen(argv[2], "w"); //open imemin.txt
	if (outputFile_1 == NULL)
	{
		printf("Error: file pointer is null.");
		return;
	}
	outputFile_2 = fopen(argv[3], "w"); //open dmemin.txt
	if (outputFile_2 == NULL)
	{
		printf("Error: file pointer is null.");
		return;
	}
	inputFile = fopen(argv[1], "r"); //open assemble input file
	if (inputFile == NULL)
	{
		printf("Error: file pointer is null.");
		return;
	}
	while (!feof(inputFile)) //Read input file until we get End-Of-File
	{
		readline(inputFile, line);
		// sort type of line and sending to LineToCommand function. if it's label we send the line after label
		if (is_label(line, line_afte_label) == true)
			lineToCommand(line_afte_label, &com, label_size, label_table, outputFile_1, outputFile_2);
		else
			lineToCommand(line, &com, label_size, label_table, outputFile_1, outputFile_2);
	}
	add_to_dmemin(outputFile_2); //create and edit demein putput file
	fclose(inputFile);
	fclose(outputFile_1);
	fclose(outputFile_2);
	free(label_table); //free label linked list
	return;
}


// Mapping each label to his address
label* build_label_table(char *argv[], int* table_size)
{
	char opcode[6];
	char rd[6];
	char rs[6];
	char rt[6];
	char extra[MAX_LABEL];
	char label_name[MAX_LABEL];
	FILE* inputFile;
	int counter = 0;
	char line[MAX_LINE + 1];
	int i = 0;
	int j = 0;
	label* label_table = NULL;
	bool is_empty_line;
	char line_after_label[MAX_LINE + 1];

	inputFile = fopen(argv[1], "r"); //open input file for reading
	if (inputFile == NULL)
	{
		printf("Error: file pointer is null.");
		return 0;
	}
	while (!feof(inputFile))
	{
		readline(inputFile, line);
		if (is_label(line, line_after_label) == false) // means the line isn't a label type
		{
			is_empty_line = splitLine(line, opcode, rd, rs, rt, extra); //calling split line and checks if line is empty
			if (is_empty_line == false || opcode[0] == '.') //skipping .word command and empty line for counting
				continue;
			if (strcmp(rd, "$imm") == 0 || strcmp(rs, "$imm") == 0 || strcmp(rt, "$imm") == 0) //checks if the line contains $imm field and counting the current address 
				counter += 2;
			else
				counter += 1;
		}
		else
		{
			i = 0;
			j = 0;
			//extract label name from line
			while (line[i] == ' ' || line[i] == '\t')
				i++;
			while (line[i] != ':')
				label_name[j++] = line[i++];
			label_name[j] = '\0';

			//adding label name to linked list of label struct
			if (label_table == NULL)
			{
				label_table = (label*)malloc(sizeof(label));
				strcpy(label_table->name, label_name);
				label_table->addr = counter;
				(*table_size)++;
			}
			else
			{
				(*table_size)++;
				label_table = (label*)realloc(label_table, sizeof(label)*(*table_size));
				strcpy(label_table[(*table_size) - 1].name, label_name);
				label_table[(*table_size) - 1].addr = counter;
			}

			// checks of the the line after label name (could be a legal command line) and keep counting current address
			is_empty_line = splitLine(line_after_label, opcode, rd, rs, rt, extra);
			if (is_empty_line == false || opcode[0] == '.')
				continue;
			if (strcmp(rd, "$imm") == 0 || strcmp(rs, "$imm") == 0 || strcmp(rt, "$imm") == 0)
				counter += 2;
			else
				counter += 1;
		}
	}
	fclose(inputFile);
	return label_table;
}

//function which checks if line contains label and extract the line after label if the answer is Yes
bool is_label(char *line, char *line_after_label)
{
	int i = 0;
	int j = 0;

	//skipping whitespace and tabs
	while (line[i] == ' ' || line[i] == '\t')
	{
		i++;
		if (line[i] == '\0')
			return false; //means it's empty line
	}
	while (line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '\0' && line[i] != '$' && line[i] != ':')
		i++;
	if (line[i] != ':')
		return false;
	else //means it's label + extract line agter label
	{
		i++;
		while (line[i] != '\0')
			line_after_label[j++] = line[i++];
		line_after_label[j] = '\0';
	}
	return true;
}


//function that reading line and adding '\0' in the end
void readline(FILE* inputFile, char* line)
{
	int counter = 0;

	char tav = getc(inputFile);
	while ((tav != '\n') && (tav != EOF))
	{
		line[counter++] = tav;
		tav = getc(inputFile);
	}
	line[counter] = '\0';
}

//split line to fields and adding to imem output file
void lineToCommand(char* line, command* com, int label_size, label* label_table, FILE* outputFile_1, FILE* outputFile_2)
{
	char opcode[6];
	char rd[6];
	char rs[6];
	char rt[6];
	char extra[MAX_LABEL];
	int first_line = 0;
	int second_line = 0;
	bool is_empty_line;
	int address;
	int value;

	is_empty_line = splitLine(line, opcode, rd, rs, rt, extra);
	if (is_empty_line == false)
		return;
	if (opcode[0] == '.') //means it's .word opcode
	{
		if (rd[1] == 'x' || rd[1] == 'X') //Hexa number 
			sscanf(rd, "%x", &address);
		else //decimal number
			address = atoi(rd);
		if (rs[1] == 'x' || rs[1] == 'X') //Hexa number 
			sscanf(rs, "%x", &value);
		else //decimal number
			value = atoi(rs);
		//saving address and value in global table and updating the num of .word commands
		word_table[word_table_pos][0] = address;
		word_table[word_table_pos][1] = value;
		word_table_pos++;
		return;
	}
	else //means it's not a .word command
	{
		com->Opcode = getOpcode(opcode); //convert string opcode to int
		com->rd = getReg(rd); //convert rd register string name to his int value
		com->rs = getReg(rs); //convert rs register string name to his int value
		com->rt = getReg(rt); //convert rt register string name to his int value
	}
	first_line = (com->Opcode << 12) | (com->rd << 8) | (com->rs << 4) | (com->rt); //shifting each field to his hexa location 
	fprintf(outputFile_1, "%05X\n", first_line); //adding line to imem putput file
	if (com->rs != 1 && com->rd != 1 && com->rt != 1) //chaeks if we need to use second line
		return;
	if ((extra[0] >= '0' && extra[0] <= '9') || extra[0] == '-') //means thet extra is number, pos or neg num
	{
		if (extra[1] == 'x' || extra[1] == 'X') //hexa number
			sscanf(extra, "%x", &second_line);
		else //secimal number
			second_line = atoi(extra);
	}
	else //means thet extra is label
		second_line = get_label_addr(extra, label_table, label_size); //return label address
	fprintf(outputFile_1, "%05X\n", second_line & 0xFFFFF); //adding line to imem putput file
	return;
}

//return address int num from label linked list
int get_label_addr(char* extra, label* label_table, int label_size)
{
	for (int i = 0; i < label_size; i++)
	{
		if (strcmp(label_table[i].name, extra) == 0)
			return label_table[i].addr;
	}
	return -1;
}


//function that split command line to fields and also return True/False if it's empty line
bool splitLine(char* line, char* opcode, char* rd, char* rs, char* rt, char* extra)
{
	int i = 0;
	int j = 0;

	while (line[i] == ' ' || line[i] == '\t') //skipping whitespace and tabs
		i++;
	if (line[i] == '\0')
		return false; //means it's empty line
	if (line[i] != '.') //not a '.word' command
	{
		//parsing opcode field
		while (line[i] != ' ' && line[i] != '\t' && line[i] != '$')
			opcode[j++] = line[i++];
		opcode[j] = '\0';
		j = 0;

		while (line[i] == ' ' || line[i] == '\t')
			i++;

		//parsing rd field
		while (line[i] != ',' && line[i] != ' ' && line[i] != '\t')
			rd[j++] = line[i++];
		rd[j] = '\0';
		j = 0;

		while (line[i] == ' ' || line[i] == '\t' || line[i] == ',')
			i++;

		//parsing rs field
		while (line[i] != ',' && line[i] != ' ' && line[i] != '\t')
			rs[j++] = line[i++];
		rs[j] = '\0';
		j = 0;

		while (line[i] == ' ' || line[i] == '\t' || line[i] == ',')
			i++;

		//parsing rt field
		while (line[i] != ',' && line[i] != ' ' && line[i] != '\t')
			rt[j++] = line[i++];
		rt[j] = '\0';
		j = 0;

		while (line[i] == ' ' || line[i] == '\t' || line[i] == ',')
			i++;

		//parsing last field (extra)
		while (line[i] != '#' && line[i] != ' ' && line[i] != '\t' && line[i] != '\n')
			extra[j++] = line[i++];
		extra[j] = '\0';

		return true; //means it's not empty line
	}
	else //'.word' command
	{
		strcpy(opcode, ".word"); //copy '.word' to opcode 

		while (line[i] < '0' || line[i] > '9') //skipping non numbers inputs
			i++;

		//parsing address value of .word command
		while (line[i] != ' ' && line[i] != '\t' && line[i] != '-')
			rd[j++] = line[i++];
		rd[j] = '\0';
		j = 0;

		while (line[i] == ' ' || line[i] == '\t')
			i++;

		//parsing data value of .word command
		while (line[i] != ' ' && line[i] != '\t' && line[i] != '\0' && line[i] != '#')
			rs[j++] = line[i++];
		rs[j] = '\0';

		return true; //means it's not empty line
	}
}


//function that return int num of string opcode
int getOpcode(char* opcode)
{
	for (int i = 0; i <= NUM_OF_OPCODE; i++)
	{
		if (strcmp(opcodesTable[i], opcode) == 0)
			return i;
	}
	return -1;
}

//function that return int num of string register
int getReg(char* reg)
{
	for (int i = 0; i <= NUM_OF_REG; i++)
	{
		if (strcmp(regTable[i], reg) == 0)
			return i;
	}
	return -1;
}

//function that create and edit dmem output file
void add_to_dmemin(FILE *outputFile_2)
{
	int counter = 0;
	int i, j, temp_address, temp_value;

	//Bubble sort of .word table - sorting by his address
	for (i = 0; i < (word_table_pos - 1); ++i)
	{
		for (j = 0; j < word_table_pos - 1 - i; ++j)
		{
			if (word_table[j][0] > word_table[j + 1][0])
			{
				temp_address = word_table[j + 1][0];
				temp_value = word_table[j + 1][1];
				word_table[j + 1][0] = word_table[j][0];
				word_table[j + 1][1] = word_table[j][1];
				word_table[j][0] = temp_address;
				word_table[j][1] = temp_value;
			}
		}
	}

	//add output line to dmem putput file
	for (i = 0; i < word_table_pos; i++)
	{
		while (counter < word_table[i][0])
		{
			fprintf(outputFile_2, "%08X\n", 0); //adding zero's line for unused address
			counter++;
		}
		fprintf(outputFile_2, "%08X\n", word_table[i][1]); //adding value of currnet address from word global table 
		counter++;
	}
	return;
}
