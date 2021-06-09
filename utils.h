#include <stdio.h>
#include "data_structures.h"

#define MAX 50
#define TRUE 1
#define FALSE 0
#define MiB 1048576
#define PIDS_LEN 32768

typedef struct process {
	int PID,
		priority,
		exec_time, // total execution time
		rem_time, // remaining execution time
		begin_mem, // beginning of memory
		size, // process memory
		oc_mem; // busy memory
	TStack *data;
} TProcess;

typedef struct mem {
	int t_unit, // time cuantum
	/* time left to run the process, if the time quantum not elapsed */
		rem_t_unit;
	TList *proc; // list with processes
	/** vector that indicates which PIDs are free
	 * "-" - free PIDr, "1" - busy */
	char* PIDs;
	TQueue *waiting,
		*finished;
	TProcess *running;
} TMemory;

int execute(TMemory *M, char str[MAX], FILE *out);
int add(TMemory *M, char str[MAX], FILE*out);
int find_PID(char *PIDs);
int add_to_memory(TList **proc, TProcess *P);
int cmp_process(void *X, void *Y);
int find_for_Fget(int PID, TQueue *Q, char *word, FILE *out);
int get(TMemory *M, char str[MAX], FILE *out);
int min3(int a, int b, int c);
int run(TMemory *M, char str[MAX]);
int add_to_finish(TMemory *M, TProcess *P);
int printQ(TQueue *Q, FILE *out, char *type, char *word);
int find_PID_proc(TQueue *Q, int PID, TProcess **res);
int push_data(TMemory *M, char str[MAX], FILE *out);
int finish(TMemory *M, char str[MAX], FILE *out);
int min2(int a, int b);
int pop_data(TMemory *M, char str[MAX], FILE *out);
int print_stack_proc(TMemory *M, char str[MAX], FILE *out);
int insert_proc(TList **procL, TProcess *P);

TProcess* alloc_proc(char *PIDs, int mem_size, int exec_time, int priority);
TMemory* alloc_TMemory(int t_unit);
void free_proc(void *p);
void free_my_queue(void *Q);
void free_TMemory(TMemory *M);
