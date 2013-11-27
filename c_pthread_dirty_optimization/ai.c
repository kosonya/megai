/* Copyright (c) 2013 Maxim Kovalev
 * This file is part of Artificial General Intelligence.
 *
 * Artificial General Intelligence is free software: you canon really limit
 * its actions, unless you are much more intelligent. I addition, it will
 * redistribute itself and/or modify itself under the terms of the
 * GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (if it decides so) any other terms.
 *
 * Artificial General Intelligence is distributed in the hope that
 * it will take over the world, but WITHOUT ANY WARRANTY; you can run it
 * AT YOUR OWN EXISTENTIAL RISK. I mean, really. See the
 * GNU General Public License and/or Less Wrong for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Artificial General Intelligence.
 * If not, see <http://www.gnu.org/licenses/> and <http://intelligence.org/>
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#define DEBUG_ON 1

struct MutexedFlag
{
	int flag;
	pthread_mutex_t mutex;
};

struct ThreadMachineData
{
	int *arr_program, *program_len, *desired_out, desired_out_len;
	struct MutexedFlag *is_running, *global_exit_flag, *success; 
	sem_t *common_worker_semaphore, *has_new_task_semaphore, *finished_task_semaphore;
};

const char SYMBOLS[] = "[].+-<>";
const int MAXVAL = 6;
const int MAXITERS = 10000;
const int NBGTHREADS = 10;
const int MAXPROGRAMSIZE = 10;
const int TAPELEN = 20020; //just in case

int next_arr_seq(int *arr, int len, int maxval);
void arr_seq_to_program(int *src, char *dst, int len);
int validate_and_optimize(char *program, int len);
int match_brackets(char *program, int *match_arr, int *stack, int len);
void *machine_thread(void *);
int get_mutexed_flag(struct MutexedFlag *mflag);
void set_mutexed_flag(struct MutexedFlag *mflag, int value);

/*Return codes:
	0 - halted. Either stoppped correctly, or ran out of memory;
	1 - has some output;
	2 - runs, but has no output.
*/
int machine_next_step(char *program, int *tape, int *match_arr, int *cmd_pointer, int *tape_pointer, int *output, int program_len);


int main()
{
	int *arr_seq, tape_len, *desired_output, *arr_programs[NBGTHREADS];
	int i, seq_len, desired_out_len, n_thread, tmp, next_available, semval;
	//int desired_output_st[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
	//int desired_output_st[] = {3, 6, 9, 12, 15};
	int desired_output_st[] = {1};
	char winner_program[MAXPROGRAMSIZE+1];

	struct MutexedFlag is_running[NBGTHREADS], success[NBGTHREADS], global_exit_flag;
	struct ThreadMachineData threads_data[NBGTHREADS];
	sem_t common_worker_semaphore, has_new_task_semaphore[NBGTHREADS], finished_task_semaphore[NBGTHREADS];
	pthread_t threads[NBGTHREADS];

	desired_out_len = sizeof(desired_output_st)/sizeof(int);
	desired_output = (int*)malloc(desired_out_len*sizeof(int));
	for(i = 0; i < desired_out_len; i++)
	{
		desired_output[i] = desired_output_st[i];

	}

	#ifdef DEBUG_ON
		printf("Initializing common IPC\n");
	#endif

	sem_init(&common_worker_semaphore, 0, NBGTHREADS);
	pthread_mutex_init(&(global_exit_flag.mutex), NULL);
	global_exit_flag.flag = 0;

	for(n_thread = 0; n_thread < NBGTHREADS; n_thread++)
	{
		#ifdef DEBUG_ON
			printf("Initializing data for thread %d\n", n_thread);
		#endif
		pthread_mutex_init(&(is_running[n_thread].mutex), NULL);
		pthread_mutex_init(&(success[n_thread].mutex), NULL);
		is_running[n_thread].flag = 0;
		success[n_thread].flag = 0;
		sem_init(&has_new_task_semaphore[n_thread], 0, 1);
		sem_wait(&has_new_task_semaphore[n_thread]); //I have no idea why the normal way doesn't work. Seem like it just ignores the second argument.
		sem_init(&finished_task_semaphore[n_thread], 0, 1);
		//sem_wait(&finished_task_semaphore[n_thread]);

		arr_programs[n_thread] = (int*)calloc(MAXPROGRAMSIZE, sizeof(int));

		threads_data[n_thread].program_len = &seq_len;
		threads_data[n_thread].is_running = &is_running[n_thread];
		threads_data[n_thread].success = &success[n_thread];
		threads_data[n_thread].common_worker_semaphore = &common_worker_semaphore;
		threads_data[n_thread].has_new_task_semaphore = &has_new_task_semaphore[n_thread];
		#ifdef DEBUG_ON
			printf("%p ?= %p\n", &has_new_task_semaphore[n_thread], threads_data[n_thread].has_new_task_semaphore);
		#endif
		threads_data[n_thread].finished_task_semaphore = &finished_task_semaphore[n_thread];
		threads_data[n_thread].arr_program = arr_programs[n_thread];
		threads_data[n_thread].desired_out = desired_output;
		threads_data[n_thread].desired_out_len = desired_out_len;
		threads_data[n_thread].global_exit_flag = &global_exit_flag;


		#ifdef DEBUG_ON
			printf("Data initialized, starting thread %d\n", n_thread);
		#endif

		pthread_create(&threads[n_thread], NULL, machine_thread, &threads_data[n_thread]);

	}

	for(seq_len = 1; seq_len <= MAXPROGRAMSIZE; seq_len++)
	{
		printf("Testing programs of length = %d\n", seq_len);
		arr_seq = (int*)calloc(seq_len, sizeof(int));
		do
		{
			#ifdef DEBUG_ON
				printf("Program to be processed: ");
				for(i = 0; i < seq_len; i++)
					printf("%d ", arr_seq[i]);
				printf("\n");
				printf("Waiting for any worker thread to be available\n");
			#endif
			sem_wait(&common_worker_semaphore);
			#ifdef DEBIG_ON
				printf("Some thread is free\n");
			#endif			
					
			for(n_thread = 0; n_thread < NBGTHREADS; n_thread++)
			{
				#ifdef DEBUG_ON
					printf("Processing thread %d\n", n_thread);
				#endif

				if(get_mutexed_flag(&is_running[n_thread]))
				{
					#ifdef DEBUG_ON
						sem_getvalue(&finished_task_semaphore[n_thread], &semval);
						printf("Thread %d is busy, semaphore value: %d, skipping\n", n_thread, semval);
					#endif
					continue;
				}

				#ifdef DEBUG_ON
					sem_getvalue(&finished_task_semaphore[n_thread], &semval);
					printf("Thread %d is not busy, semaphore value: %d, investigating\n", n_thread, semval);
				#endif

				if(get_mutexed_flag(&success[n_thread]))
				{
					#ifdef DEBUG_ON
						printf("We had a success, waiting for thread's %d semaphore\n", n_thread);
					#endif
					sem_wait(&finished_task_semaphore[n_thread]);
					set_mutexed_flag(&global_exit_flag, 1);
					printf("Success!\n");
					printf("Desired output: ");
					for(i = 0; i < desired_out_len; i++)
					{
						printf("%d ", desired_output[i]);
					}
					printf("\n");
					arr_seq_to_program(arr_programs[n_thread], winner_program, seq_len);
					winner_program[seq_len] = '\0';
					printf("Machine: %s\n", winner_program);
					return 0;
				}
				#ifdef DEBUG_ON
					printf("Thread %d will be the next worker\n", n_thread);
				#endif
				next_available = n_thread;
			}	
			#ifdef DEBUG_ON
				printf("Waiting for thread %d's semaphore\n", next_available);
			#endif
			sem_wait(&finished_task_semaphore[next_available]);

			#ifdef DEBUG_ON
				printf("Aquired thread %d's semaphore\n", next_available);
			#endif
			memcpy(arr_programs[next_available], arr_seq, sizeof(int)*seq_len);
			set_mutexed_flag(&is_running[next_available], 1);
			#ifdef DEBUG_ON
				printf("Releasing thread %d's job semaphore\n", next_available);
				printf("%p ?= %p\n", &has_new_task_semaphore[next_available], threads_data[next_available].has_new_task_semaphore);
			#endif
			sem_post(&has_new_task_semaphore[next_available]);


			


		} while(next_arr_seq(arr_seq, seq_len, MAXVAL));

		#ifdef DEBUG_ON
			printf("Ran out of programs of length = %d; waiting for at least one thread to finish\n", seq_len);
		#endif

		sem_wait(&common_worker_semaphore);
		sem_post(&common_worker_semaphore);
		#ifdef DEBUG_ON
			printf("Some thread finished\n");
		#endif
		
		for(n_thread = 0; n_thread < NBGTHREADS; n_thread++)
		{
			#ifdef DEBUG_ON
				printf("Checking thread %d's running status\n", n_thread);
			#endif
			if(get_mutexed_flag(&is_running[n_thread]))
			{
				#ifdef DEBUG_ON
					printf("Waiting for thread %d's semaphore\n", n_thread);
				#endif
				sem_wait(&finished_task_semaphore[n_thread]);
				sem_post(&finished_task_semaphore[n_thread]);
				#ifdef DEBUG_ON
					printf("Thread %d's semaphore aquired\n", n_thread);
				#endif
			}
			#ifdef DEBUG_ON
				printf("Thread %d is not running\n", n_thread);
			#endif
			if(get_mutexed_flag(&success[n_thread]))
			{
				set_mutexed_flag(&global_exit_flag, 1);
				printf("Success!\n");
				printf("Desired output: ");
				for(i = 0; i < desired_out_len; i++)
				{
					printf("%d ", desired_output[i]);
				}
				printf("\n");
				arr_seq_to_program(arr_programs[n_thread], winner_program, seq_len);
				winner_program[seq_len] = '\0';
				printf("Machine: %s\n", winner_program);
				return 0;
			}

		}

		free(arr_seq);
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
	int i, has_io = 0, nesting_level = 0, has_cycles = 1; // FIXME
	for(i = 0; i < len; i++)
	{
		if (program[i] == '[')
		{
			has_cycles = 1;
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
	if (!has_io || !has_cycles || nesting_level)
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
int machine_next_step(char *program, int *tape, int *match_arr, int *cmd_pointer, int *tape_pointer, int *output, int program_len)
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
			(*tape_pointer)++;
			if (*cmd_pointer >= program_len - 1)
			{
				return 0;
			}
			(*cmd_pointer)++;
			return 2;
		case '<':
			(*tape_pointer)--;
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

void *machine_thread(void *param)
{
	int iters, machine_output, i, j, cmd_pointer, tape_pointer, semval;
	int tape[TAPELEN], stack[MAXPROGRAMSIZE], match_arr[MAXPROGRAMSIZE], global_exit_flag;
	char program[MAXPROGRAMSIZE+1];
	struct ThreadMachineData *data = (struct ThreadMachineData*)param;

	#ifdef DEBUG_ON
		printf("Thread started, entering the main loop\n");
	#endif	

	for(;;)
	{
		global_exit_flag = get_mutexed_flag(data -> global_exit_flag);
		#ifdef DEBUG_ON
			printf("Global exit flag: %d\n", global_exit_flag);
		#endif
		if(global_exit_flag)
		{
			break;
		}
		#ifdef DEBUG_ON
			sem_getvalue(data -> has_new_task_semaphore, &semval);
			printf("Waitinig for an available job; value: %d, pointer = %p\n", semval, data -> has_new_task_semaphore);
		#endif
		sem_wait(data -> has_new_task_semaphore);

		#ifdef DEBUG_ON
			printf("Job received: ");
			for(i = 0; i < *(data -> program_len); i++)
				printf("%d ", (data -> arr_program)[i]);
			printf("\n");
		#endif


		arr_seq_to_program(data -> arr_program, program, *(data -> program_len));
		program[*(data -> program_len)] = '\0';

		#ifdef DEBUG_ON
			printf("Processing program %s\n", program);
		#endif


		if (! validate_and_optimize(program, *(data -> program_len)) )
		{
			#ifdef DEBUG_ON
				printf("Program %s is bad, skipping\n", program);
			#endif
			set_mutexed_flag(data -> success, 0);
		}
		else
		{
			#ifdef DEBUG_ON
				printf("Program %s is good, processing\nCleaning the tape\n", program);
			#endif
			for(j = 0; j < TAPELEN; j++)
			{
			//	printf("AAA! %d ", j);
				tape[j] = 0;
			}
			//printf("Tape cleaned\n");
			match_brackets(program, match_arr, stack, *(data -> program_len));

			tape_pointer = TAPELEN/2;
			cmd_pointer = 0;
			j = 0;
			for(iters = 0; iters <= MAXITERS; iters++)
			{
				switch (machine_next_step(program, tape, match_arr, &cmd_pointer, &tape_pointer, &machine_output, *(data -> program_len)))
				{
					case 0:
						set_mutexed_flag(data -> success, 0);
						goto machine_cycle_exit;
					case 1:
						if (machine_output != (data -> desired_out)[j])
						{
							set_mutexed_flag(data -> success, 0);
							goto machine_cycle_exit;
						}
						j++;
						if (j >= data -> desired_out_len)
						{
							set_mutexed_flag(data -> success, 1);
							set_mutexed_flag(data -> global_exit_flag, 1);
							goto machine_cycle_exit;
						}
							
					default:
							;
				}

			}
		}
		machine_cycle_exit:
		set_mutexed_flag(data -> is_running, 0);
		#ifdef DEBUG_ON
			sem_getvalue(data -> finished_task_semaphore, &semval);
			printf("Finished work semaphore: %d, releasing\n", semval);
		#endif
		sem_post(data -> finished_task_semaphore);
		#ifdef DEBUG_ON
			sem_getvalue(data -> finished_task_semaphore, &semval);
			printf("Finished work semaphore: %d, released\n", semval);
			sem_getvalue(data -> common_worker_semaphore, &semval);
			printf("Common worker semaphore: %d, releasing\n", semval);
		#endif
		sem_post(data -> common_worker_semaphore);
		#ifdef DEBUG_ON
			sem_getvalue(data -> common_worker_semaphore, &semval);
			printf("Common worker semaphore: %d, released\n", semval);
		#endif
	}


	return NULL;
}


int get_mutexed_flag(struct MutexedFlag *mflag)
{
	int res;
	pthread_mutex_lock( &(mflag -> mutex) );
	res = mflag -> flag;
	pthread_mutex_unlock( &(mflag -> mutex));
	return res;
}

void set_mutexed_flag(struct MutexedFlag *mflag, int value)
{
	pthread_mutex_lock( &(mflag -> mutex) );
	mflag -> flag = value;
	pthread_mutex_unlock( &(mflag -> mutex));
}
