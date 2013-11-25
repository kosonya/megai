#include <stdio.h>
#include <stdlib.h>

const char SYMBOLS[] = "[].+-<>";
const int MAXVAL = 6;

int next_arr_seq(int *arr, int len, int maxval);
void arr_seq_to_program(int *src, char *dst, int len);
int validate_and_optimize(char *program, int len);
int match_brackets(char *program, int *match_arr, int *stack, int len);

int main()
{
	int *arr_seq, *stack, *match_arr;
	int i, j, seq_len;
	char *program;


	arr_seq = (int*)calloc(seq_len, sizeof(int));
	program = (char*)malloc((seq_len+1)*sizeof(char));
	stack = (int*)calloc(seq_len, sizeof(int));
	match_arr = (int*)calloc(seq_len, sizeof(int));
/*
	for(i = 0; i <= 25; i++)
		//program[i] = ".[.].[.[.].].[.[.].[.].]."[i];
		program[i] = "........................."[i];
	match_brackets(program, match_arr, stack, seq_len);
	for(i = 0; i < 25; i++)
		printf("%3d", i);
	printf("\n");
	for(i = 0; i < 25; i++)
		printf("%3c", program[i]);
	printf("\n");
	for(i = 0; i < 25; i++)
		printf("%3d", match_arr[i]);
	printf("\n");
*/


	for(seq_len = 1; seq_len <= 6; seq_len++)
	{
		arr_seq = (int*)calloc(seq_len, sizeof(int));
		program = (char*)malloc((seq_len+1)*sizeof(char));
		stack = (int*)calloc(seq_len, sizeof(int));
		match_arr = (int*)calloc(seq_len, sizeof(int));
		do
		{
			arr_seq_to_program(arr_seq, program, seq_len);
			if (validate_and_optimize(program, seq_len))
			{
				program[seq_len] = '\0';
				if (match_brackets(program, match_arr, stack, seq_len))
				{
					printf("\n");
					for(j = 0; j < seq_len; j++)
						printf("%3d", j);
					printf("\n");
					for(j = 0; j < seq_len; j++)
						printf("%3c", program[j]);
					printf("\n");
					for(j = 0; j < seq_len; j++)
						printf("%3d", match_arr[j]);
					printf("\n\n");
				}
				else
					printf("%s\n", program);
			}
		} while(next_arr_seq(arr_seq, seq_len, MAXVAL));
		free(arr_seq);
		free(program);
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

int match_brackets(char *program, int *match_arr, int *stack, int len)
{
	int i, stack_head = -1, contains_brackets = 0;
	for(i = 0; i < len; i++)
	{
		switch (program[i])
		{
			case '[':
				stack_head++;
				stack[stack_head] = i;
				contains_brackets = 1;
				break;
			case ']':
				match_arr[i] = stack[stack_head];
				stack_head--;
				break;
			default:
				match_arr[i] = 0;
		}
	}
	if (!contains_brackets)
		return 0;
	for(i = 0; i < len; i++)
	{
		if (program[i] == ']')
			match_arr[match_arr[i]] = i;
	}
	return 1;
}
