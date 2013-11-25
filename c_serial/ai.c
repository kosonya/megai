#include <stdio.h>
#include <stdlib.h>

const char SYMBOLS[] = "[].+-<>";
const int MAXVAL = 6;

int next_arr_seq(int *arr, int len, int maxval);
void arr_seq_to_program(int *src, char *dst, int len);
int validate_and_optimize(char *program, int len);

int main()
{
	int *arr_seq;
	int i, seq_len;
	char *program;
	for(seq_len = 1; seq_len <= 6; seq_len++)
	{
		arr_seq = (int*)calloc(seq_len, sizeof(int));
		program = (char*)malloc((seq_len+1)*sizeof(char));
		do
		{
			arr_seq_to_program(arr_seq, program, seq_len);
			if (validate_and_optimize(program, seq_len))
			{
				program[seq_len] = '\0';
				printf("%s\n", program);
			}
		} while(next_arr_seq(arr_seq, seq_len, MAXVAL));
	}
	return 0;
}


int next_arr_seq(int *arr, int len, int maxval)
{
	int i;
	for(i = 0; i < len; i++)
	{
		arr[i]++;
		if (arr[i] <= maxval)
			return 1;
		arr[i] = 0;
	}
	return 0;
}

void arr_seq_to_program(int *src, char *dst, int len)
{
	int i;
	for(i = 0; i < len; i++)
		dst[i] = SYMBOLS[src[i]];
}

int validate_and_optimize(char *program, int len)
{
	int i, has_io = 0, nesting_level = 0;
	for(i = 0; i < len; i++)
	{
		if (program[i] == '[')
		{
			if ( (i < len - 1) && program[i+1] == ']')
				return 0;
			nesting_level++;
			continue;
		}
		if (program[i] == ']')
		{
			nesting_level--;
			if(nesting_level < 0)
			{
				return 0;
			}
			continue;
		}
		if (program[i] == '.')
		{
			has_io = 1;
			continue;
		}
		if (i < len - 1)
		{
			switch(program[i])
			{
				case '+':
					if (program[i+1] == '-')
						return 0;
					break;
				case '-':
					if (program[i+1] == '+')
						return 0;
					break;
				case '<':
					if (program[i+1] == '>')
						return 0;
					break;
				case '>':
					if (program[i+1] == '<')
						return 0;
					break;
				default:
					;
			}
		}
	}
	if (!has_io || nesting_level)
		return 0;
	return 1;
}
