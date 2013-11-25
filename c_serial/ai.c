#include <stdio.h>
#include <stdlib.h>


const char SYMBOLS[] = "[].+-<>";
const int MAXVAL = 6;
const int MAXITERS = 10000;

int next_arr_seq(int *arr, int len, int maxval);
void arr_seq_to_program(int *src, char *dst, int len);
int validate_and_optimize(char *program, int len);
int match_brackets(char *program, int *match_arr, int *stack, int len);

/*Return codes:
	0 - halted. Either stoppped correctly, or ran out of memory;
	1 - has some output;
	2 - runs, but has no output.
*/
int machine_next_step(char *program, int *tape, int *match_arr, int *cmd_pointer, int *tape_pointer, int *output, int program_len, int tape_len);


int main()
{
	int *arr_seq, *stack, *match_arr, *tape, tape_len, tape_pointer, cmd_pointer, machine_output, *desired_output;
	int i, seq_len, iters, desired_out_len, j;
	char *program;
	int desired_output_st[] = {1, 2, 3, 4, 5};

	tape_len = MAXITERS*2;

	desired_out_len = sizeof(desired_output_st)/sizeof(int);
	desired_output = (int*)malloc(desired_out_len*sizeof(int));
	for(i = 0; i < desired_out_len; i++)
	{
		desired_output[i] = desired_output_st[i];
		//printf("%d - %d\n", i, desired_output[i]);
	}

	for(seq_len = 1; seq_len <= 7; seq_len++)
	{
		printf("Testing programs of length = %d\n", seq_len);
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
				match_brackets(program, match_arr, stack, seq_len);
				//printf("%s\n", program);
				tape = (int*)calloc(tape_len, sizeof(int));
				tape_pointer = tape_len/2;
				cmd_pointer = 0;
				j = 0;
				for(iters = 0; iters <= MAXITERS; iters++)
				{
					switch (machine_next_step(program, tape, match_arr, &cmd_pointer, &tape_pointer, &machine_output, seq_len, tape_len))
					{
						case 0:
							goto machine_loop_exit; //fuck
						case 1:
							//printf("M:%d D[%d]:%d; ", machine_output, j, desired_output[j]);
							if (machine_output != desired_output[j])
							{
								//printf("\nFail\n");
								goto machine_loop_exit;
							}
							j++;
							if (j >= desired_out_len)
							{
								printf("Success!!!\n");
								printf("Desired output: ");
								for(j = 0; j < desired_out_len; j++)
									printf("%d ", desired_output[j]);
								printf("\nMachine: %s\n", program);
								goto main_loop_exit;
							}
							
						case 2:
							;
					}

				}
				machine_loop_exit:
				free(tape);
				//printf("\n\n");
				
			}
		} while(next_arr_seq(arr_seq, seq_len, MAXVAL));
		free(arr_seq);
		free(program);
		free(stack);
		free(match_arr);
	}
	main_loop_exit:
	free(desired_output);
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


/*Return codes:
	0 - halted. Either stoppped correctly, or ran out of memory;
	1 - has some output;
	2 - runs, but has no output.
*/
int machine_next_step(char *program, int *tape, int *match_arr, int *cmd_pointer, int *tape_pointer, int *output, int program_len, int tape_len)
{
	switch (program[*cmd_pointer])
	{
		case '+':
			tape[*tape_pointer]++;
			if (*cmd_pointer >= program_len - 1)
			{
				return 0;
			}
			(*cmd_pointer)++;
			return 2;
		case '-':
			tape[*tape_pointer]--;
			if (*cmd_pointer >= program_len - 1)
			{
				return 0;
			}
			(*cmd_pointer)++;
			return 2;
		case '>':
			if (*tape_pointer < tape_len - 1)
			{
				(*tape_pointer)++;
			}
			else
			{
				return 0;
			}
			if (*cmd_pointer >= program_len - 1)
			{
				return 0;
			}
			(*cmd_pointer)++;
			return 2;
		case '<':
			if (*tape_pointer > 0)
			{
				(*tape_pointer)--;
			}
			else
			{
				return 0;
			}
			if (*cmd_pointer >= program_len - 1)
			{
				return 0;
			}
			(*cmd_pointer)++;
			return 2;
		case '.':
			*output = tape[*tape_pointer];
			(*cmd_pointer)++;
			return 1;
		case '[':
			if (tape[*tape_pointer] == 0)
			{
				*cmd_pointer = match_arr[*cmd_pointer];
				return 2;
			}
			if (*cmd_pointer >= program_len - 1)
			{
				return 0;
			}
			(*cmd_pointer)++;
			return 2;
		case ']':
			if (tape[*tape_pointer] != 0)
			{
				*cmd_pointer = match_arr[*cmd_pointer];
				return 2;
			}
			if (*cmd_pointer >= program_len - 1)
			{
				return 0;
			}
			(*cmd_pointer)++;
			return 2;
	}
}
