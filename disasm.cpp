#include "disasm.h"
#include "constants.h"

char disasm_text[MAX_LENGHT_NAME] = "disasm.txt";

FILE* get_CPU_file()
{
	FILE* CPU_file = fopen (CPU_name_file, "rb");

	if (CPU_file == nullptr)
	{
		printf ("CPU: don't open file \"%s\"\n", CPU_name_file);
		return nullptr;
	}
	return CPU_file;
}

int* get_code(FILE* CPU_file, size_t* cnt_cmd)
{
	fread (cnt_cmd, 1, sizeof(int), CPU_file);
	int* code = (int*) calloc (*cnt_cmd + 1, sizeof(int));
	if (code == nullptr)
	{
		printf ("Has not memory for code\n");
		return nullptr;
	} 
	fread (code, *cnt_cmd, sizeof(int), CPU_file);
	
	return code;
}

void passes(const int* code, size_t cnt_cmd)
{
	labels data_of_labels[MAX_CNT_LABELS] = {};
	funcs  data_of_funcs[MAX_CNT_FUNCS]   = {};

	for(unsigned which_pass = 1; which_pass <= CNT_PASSES; which_pass++)
	{
		const int* start = code;
		if (which_pass == FIRST_PASS)
			first_pass(code, cnt_cmd, data_of_labels, data_of_funcs);
		else
		if (which_pass == SECOND_PASS)
			second_pass(code, cnt_cmd, data_of_labels, data_of_funcs);

		code = start;	
	}
}

void first_pass(const int* code, size_t cnt_cmd, labels* data_of_labels, funcs* data_of_funcs)
{
	unsigned 	cnt_labels = 0,
				cnt_funcs  = 0;
	for(size_t ip = 0; ip < cnt_cmd; ++ip)
	{
		if (code[ip] & TYPE_JMP)
		{
			data_of_labels[cnt_labels].num = cnt_labels+1;
			data_of_labels[cnt_labels].ip = code[++ip];
			cnt_labels++;
		}
		else if (code[ip] == CMD_CALL)
		{
			data_of_funcs[cnt_funcs].num = cnt_funcs+1;
			data_of_funcs[cnt_funcs].ip = code[++ip];
			cnt_funcs++;
		}
	}
}

#define DEF_CMD(name, num, arg, CPU_code, disasm_code) 	\
		case num:										\
			disasm_code									\
			break;

void second_pass(const int* code, size_t cnt_cmd, labels* data_of_labels, funcs* data_of_funcs)
{
	FILE* stream = fopen(disasm_text, "w");
	unsigned 	cnt_labels = 0,
				cnt_funcs  = 0;
	for (size_t ip = 0; ip < cnt_cmd; ip++)
	{
		find_label(ip, stream, data_of_labels);
		find_func (ip, stream, data_of_funcs);
		switch (code[ip] & CMD_MASK)
		{
			#include "cmd.h"
			default:
				printf ("Not found command %d\n", code[ip]);
				break;
		}
	}
	fclose(stream);
}

#undef DEF_CMD

void find_label(size_t ip, FILE* stream, labels* data_of_labels)
{
	for (size_t i = 0; i < MAX_CNT_LABELS && data_of_labels[i].ip != IP_POISSON; i++)
		if (ip == data_of_labels[i].ip)
		{
			fprintf (stream, ":l%d\n", data_of_labels[i].num);
			break;
		}
}

void find_func(size_t ip, FILE* stream, funcs* data_of_funcs)
{
	for (size_t i = 0; i < MAX_CNT_FUNCS  && data_of_funcs[i].ip  != IP_POISSON; i++)
		if (ip == data_of_funcs[i].ip)
		{
			fprintf (stream, ":f%d\n", data_of_funcs[i].num);
			break;
		}		
}

void convert_arg(const int* code, size_t* ip, FILE* stream)
{
	int plus = 0;
	if (code[*ip] & ARG_RAM) {
		fprintf(stream, "[");
		if (code[*ip] & ARG_IMMED)
		{
			fprintf(stream, "%d",code[++plus + (*ip)]);
			if (code[*ip] & ARG_REG) fprintf(stream, "+r%cx", code[++plus + (*ip)] + 'a' - 1);
		} else
		{
			if (code[*ip] & ARG_REG) fprintf(stream, "r%cx", code[++plus + (*ip)] + 'a' - 1);
		}
		fprintf(stream, "]\n");
	} else
	{
		if (code[*ip] & ARG_IMMED)
		{
			fprintf(stream, "%d",code[++plus + (*ip)]);
			if (code[*ip] & ARG_REG) fprintf(stream, "+r%cx", code[++plus + (*ip)] + 'a' - 1);
		} else
		{
			if (code[*ip] & ARG_REG) fprintf(stream, "r%cx", code[++plus + (*ip)] + 'a' - 1);
		}
		fprintf(stream, "\n");
	}
	*ip += plus;
}

void print_label(FILE* stream, const char* name_cmd, size_t label_ip, labels* data_of_labels)
{	
	fprintf(stream, "%s l%d\n", name_cmd, data_of_labels[label_ip].num);
}
