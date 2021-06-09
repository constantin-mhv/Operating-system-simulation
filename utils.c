#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

int execute(TMemory *M, char str[MAX], FILE *out) {
/* the string commands are read and the necessary operations are performed */
	char c1[10], c2[10]; // to read the first and second words
	int r = TRUE;

	sscanf(str, "%s %s", c1, c2);
	if (strcmp(c1, "add") == 0){
		r = add(M, str, out);
	} else if (strcmp(c1, "get") == 0) {
		r = get(M, str, out);
	} else if (strcmp(c1, "run") == 0) {
		r = run(M, str);
	} else if (strcmp(c1, "print") == 0 && strcmp(c2, "stack") == 0) {
		r = print_stack_proc(M, str, out);
	} else if (strcmp(c1, "print") == 0 && strcmp(c2, "waiting") == 0) {
		r = printQ(M->waiting, out, "Waiting", "remaining_time");
	} else if (strcmp(c1, "print") == 0 && strcmp(c2, "finished") == 0) {
		r = printQ(M->finished, out, "Finished", "executed_time");
	} else if (strcmp(c1, "push") == 0) {
		r = push_data(M, str, out);
	} else if (strcmp(c1, "pop") == 0) {
		r = pop_data(M, str, out);
	} else if (strcmp(c1, "finish") == 0) {
		r = finish(M, str, out);
	}
	return r;
}

int add(TMemory *M, char str[MAX], FILE *out) {
	int mem_size,  exec_time, priority, r = TRUE;

	sscanf(str, "%*s %d %d %d", &mem_size, &exec_time, &priority);
	TProcess *P = alloc_proc(M->PIDs, mem_size, exec_time, priority);
	if (!P)
		return FALSE;

	r = add_to_memory(&M->proc, P);
	if (r == FALSE)
		return FALSE;
	/* if add_to_memory returns -1 => there is no space for a process*/
	else if (r == -1) {
		fprintf(out, "Cannot reserve memory for PID %d.\n", P->PID );
		M->PIDs[P->PID] = '-';
		free_proc(P);
		return TRUE;
	}
	/* if the Idle process is running, the new process goes into running state */
	if (M->running == NULL) {
		M->running = P;
	} else {
		/* otherwise it is inserted in the queue */
		r = insertQ(M->waiting, P, cmp_process, free_proc);
		if (r == FALSE)
			return FALSE;
	}
	
	fprintf(out,
		"Process created successfully: PID: %d, Memory starts at 0x%x.\n",
			P->PID, P->begin_mem);
	return TRUE;
}

int get(TMemory *M, char str[MAX], FILE *out) {
	int PID, found = FALSE, r = TRUE;
	sscanf(str, "%*s %d", &PID);
	/* check if the searched process is not running */
	if (M->running != NULL) {
		if (M->running->PID == PID) {
			fprintf(out, "Process %d is running (remaining_time: %d).\n",
				PID, M->running->rem_time);
			found = TRUE;
		}
	}
	/** find_for_Fget returns -1 if the process was not found, so
	 * found will remain set to FALSE */

	/* search in the queue, the message is displayed accordingly */
	r = find_for_Fget(PID, M->waiting, "waiting", out);
	if (r == FALSE)
		return FALSE;
	if (r == TRUE)
		found = TRUE;
	/* search in the queue finished, the message is displayed accordingly */
	r = find_for_Fget(PID, M->finished, "finished", out);
	if (r == FALSE)
		return FALSE;
	if (r == TRUE)
		found = TRUE;

	if (found == FALSE) {
		fprintf(out, "Process %d not found.\n", PID);
	}
	return TRUE;
}
/*returneaza FALSE daca au fost erori, daca nu a fost gasit
nici un proces cu PID in coada
se returneaza -1 si TRUE in caz contrar*/

/** Returns FALSE if there were errors,
 *  returns -1 if no process with PID was found in queue
 *  returns TRUE otherwise */
int find_for_Fget(int PID, TQueue *Q, char *word, FILE *out) {
	int found = -1;
	if (Q->first == NULL) {
		return -1;
	}

	TQueue *q = InitQ();
	if (!q)
		return FALSE;
	TProcess *p;
	int r = TRUE;

	while (Q->first) {
		p = dequeue(Q);
		if (PID == p->PID) {
			if (strcmp(word, "finished") == 0 ) {
				fprintf(out, "Process %d is %s.\n",
				PID, word);
			} else {
				fprintf(out,
				"Process %d is %s (remaining_time: %d).\n",
					PID, word, p->rem_time);
			}
			found = TRUE;
		}
		r = enqueue(p, q);
		if (r == FALSE){
			freeQ(q, free_dataS);
			free_proc(p);
			return FALSE;
		}
	}
	make_sameQ(Q, q);
	free(q);

	return found;
}
/* to find unoccupied PID */
int find_PID(char *PIDs) {
	int i;
	for (i = 1; i < 32768; i++) {
		if (PIDs[i] == '-'){ // if pid is not busy
			PIDs[i] = '1'; // it is set that pid is busy
			return i;
		}
	}
	return TRUE;
}
/* printing information about the required queue */
int printQ(TQueue *Q, FILE *out, char *type, char *word) {
	int r = TRUE;
	TQueue *q = InitQ();
	if (!q)
		return FALSE;
	TProcess *p;
	if (!q)
		return FALSE;
	fprintf(out, "%s queue:\n[", type);
	while (Q->first) {
		p = dequeue(Q);
		if (Q->first)
			fprintf(out,
				"(%d: priority = %d, %s = %d),\n",
				p->PID, p->priority, word, p->rem_time);
		else
			fprintf(out,
				"(%d: priority = %d, %s = %d)",
				p->PID, p->priority, word, p->rem_time);
		r = enqueue(p, q);
		if (r == FALSE) {
			free_proc(p);
			freeQ(q, free_proc);
			return FALSE;
		}
	}
	fprintf(out, "]\n" );
	make_sameQ(Q, q);
	free(q);

	return TRUE;
}

int run(TMemory *M, char str[MAX]) {
	int time, min, r = TRUE;
	TProcess *p;
	sscanf(str, "%*s %d", &time);

	while (time > 0) {
		if (M->running != NULL) {
			/* the process that will run is taken from the running state */
			p = M->running;
		} else {
			/* otherwise it is taken from the queue */
			p = dequeue(M->waiting);
			/** in this case the remaining execution time is
			 * equal to time quantum */
			M->rem_t_unit = M->t_unit;
			/* it goes into running state */
			M->running = p;
		}
		if (p == NULL){
			/** in case there were no more processes in the queue,
			 * the remaining execution time is set and the function ends */
			M->rem_t_unit = M->t_unit;
			return TRUE;
		}
		/* choose min3 and subtract from other values */
		min = min3(time, p->rem_time, M->rem_t_unit);
		M->rem_t_unit -= min;
		p->rem_time -= min;
		time -= min;
		/** if the execution time ends for a process, a new process
		 * is extracted from the queue and check what to do
		 * with the running process */	
		if (M->rem_t_unit == 0) { 
			M->rem_t_unit = M->t_unit;
			M->running = dequeue(M->waiting);
			if (p->rem_time == 0) {
				r = add_to_finish(M, p);
				if (r == FALSE) {
					free_proc(p);
					return FALSE;
				}
			} else{
				r = insertQ(M->waiting, p, cmp_process, free_proc);
				if (r == FALSE){
					free_proc(p);
					return FALSE;
				}
			}
			p = M->running;
			continue;
		}
		/* if the run time is over, check what to do with the process */
		if (p->rem_time == 0) {
			M->rem_t_unit = M->t_unit;
			r = add_to_finish(M, p);
			if (r == FALSE) {
				free_proc(p);
				return FALSE;
			}
			M->running = NULL;
		}
	}
	/* if M->running is NULL, it means it's over time  quantum/execution time */
	if (M->running == NULL) {
		M->rem_t_unit = M->t_unit;
		M->running = dequeue(M->waiting);
	}
	return TRUE;	
}

/** function is similar to run, but total execution time is calculated,
 * the function ends its execution when the last process
 * finishes its execution */
int finish(TMemory *M, char str[MAX], FILE *out) {
	int t_unit = M->t_unit, min, finish_time = 0, r = FALSE;
	TProcess *p;

	while (M->proc != NULL) {
		if (M->running != NULL) {
			p = M->running;
		} else {
			p = dequeue(M->waiting);
			M->running = p;
		}
		if (p == NULL)
			break;
		min = min2(p->rem_time, t_unit);
		t_unit -= min;
		p->rem_time -= min;
		finish_time += min;
		if (t_unit == 0) {
			t_unit = M->t_unit;
			M->running = dequeue(M->waiting);
			if (p->rem_time == 0) {
				r = add_to_finish(M, p);
				if (r == FALSE) {
					free_proc(p);
					return FALSE;
				}
			} else{
				r = insertQ(M->waiting, p, cmp_process, free_proc);
				if (r == FALSE) {
					free_proc(p);
					return FALSE;
				}
			}
			p = M->running;
			continue;
		}
		if (p->rem_time == 0) {
			r = add_to_finish(M, p);
			if (r == FALSE) {
				free_proc(p);
				return FALSE;
			}
			M->running = NULL;
		}
	}
	fprintf(out, "Total time: %d\n", finish_time);
	return TRUE;	
}

int push_data(TMemory *M, char str[MAX], FILE *out) {
	int *data, PID, nr, r = TRUE;
	TProcess *p = NULL;
	sscanf(str, "%*s %d %d", &PID, &nr);
	// running is checked
	if (M->running != NULL) {
		if (M->running->PID == PID) {
			p = M->running;
		}
	} /* otherwise search in the queue */
	if (p == NULL) {
		r = find_PID_proc(M->waiting, PID, &p);
		if (r == FALSE)
			return FALSE;
		if (p == NULL) {
			fprintf(out, "PID %d not found.\n", PID);
			return TRUE;
		}
	} 
	if (p->oc_mem + 4 > p->size) {
		fprintf(out, "Stack overflow PID %d.\n", PID);
		return TRUE;
	} else {
		/* add to the stack */
		data = calloc(1, sizeof(int));
		if (!data) {
			return FALSE;
		}
		memcpy(data, &nr, 4);
		r = push(data, p->data);
		if (r == FALSE){
			free(data);
			return FALSE;
		}
		p->oc_mem += 4;
	}
	return TRUE;
}
int pop_data(TMemory *M, char str[MAX], FILE *out) {
	int *data, PID, r = TRUE;
	TProcess *p = NULL;
	sscanf(str, "%*s %d", &PID);
	/* running is checked */
	if (M->running != NULL) {
		if (M->running->PID == PID) {
			p = M->running;
		}
	} /* otherwise search in the queue */
	if (p == NULL) {
		r = find_PID_proc(M->waiting, PID, &p);
		if (r == FALSE)
			return FALSE;
		if (p == NULL) {
			fprintf(out, "PID %d not found.\n", PID);
			return TRUE;
		}
	} 
	if (p->data->first == NULL) {
		fprintf(out, "Empty stack PID %d.\n", PID);
	} else {
		data = pop(p->data);
		free(data);
		p->oc_mem -= 4;
	}
	
	return TRUE;
} 

int add_to_finish(TMemory *M, TProcess *P) {
	freeS(P->data, free_dataS);
	P->data = NULL;
	P->rem_time = P->exec_time;
	/* busy PID is released */
	M->PIDs[P->PID] = '-';
	remove_from_list(&M->proc,(void*)P);
	return (enqueue(P, M->finished));
}

/** find process with PID.
 * If no errors occured, in res ei saved found process,
 * otherwise res is NULL */
int find_PID_proc(TQueue *Q, int PID, TProcess **res) {
	TQueue *q = InitQ();
	if (!q)
		return FALSE;
	TProcess *p;
	int r = TRUE;
	*res = NULL;
	
	while (Q->first) {
		p = dequeue(Q);
		if (PID == p->PID) {
			*res = p;
		}

		r = enqueue(p, q);
		if (r == FALSE){
			freeQ(q, free_dataS);
			free_proc(p);
			return FALSE;
		}
	}
	make_sameQ(Q, q);
	free(q);
	return TRUE;
}

int min3(int a, int b, int c) {
	int min;
	min = a;
	if (b < min) {
		min = b;
	}
	if (c < min)
		min = c;
	return min;
}

int print_stack_proc(TMemory *M, char str[MAX], FILE *out) {
	int PID, r = TRUE;
	sscanf(str, "%*s %*s %d", &PID);
	TProcess *P = NULL;
	/* running is checked */
	if (M->running != NULL) {
		if (M->running->PID == PID) {
			P = M->running;
		}
	}
	if (P == NULL) {
		/* otherwise search in waiting */
		r = find_PID_proc(M->waiting, PID, &P);
		if (r == FALSE)
			return FALSE;
		if (P == NULL) {
			fprintf(out, "PID %d not found.\n", PID);
			return TRUE;
		}
	}
	if (P->data->first == NULL) {
		fprintf(out, "Empty stack PID %d.\n", P->PID);
		return TRUE;
	}
	TStack *s = InitS();
	if (!s)
		return FALSE;
	int *data;
	r = flipS(s, P->data);
	if (r == FALSE) {
		freeS(s, free_dataS);
		return FALSE;
	}
	fprintf(out, "Stack of PID %d:", P->PID );
	/** the numbers are displayed and added back to the process stack
	 * (stack already reversed) */
	while (s->first) {
		data = pop(s);
		fprintf(out, " %d", *(int*)data);
		r = push(data, P->data);
		if (r == FALSE) {
			freeS(s, free_dataS);
			return FALSE;
		}
	}
	free(s);
	fprintf(out, ".\n" );
	return TRUE;
}

int min2(int a, int b) {
	return a < b ? a : b;
}

TProcess* alloc_proc(char *PIDs, int mem_size, int exec_time, int priority) {
	TProcess *P = (TProcess*)calloc(1, sizeof(TProcess));

	if (!P) {
		return NULL;
	}
	P->size = mem_size;
	P->exec_time = exec_time;
	P->oc_mem = 0;
	P->rem_time = exec_time;
	P->priority = priority;
	P->PID = find_PID(PIDs);
	P->data = InitS();
	if (P->data == NULL) {
		free(P);
		return NULL;
	}
	return P;

}

int add_to_memory(TList **procL, TProcess *P) {
	TList *new_procL = NULL, *temp;
	TProcess *p;
	int r = TRUE;
	/* returns -1 if failed to add process */
	r = insert_proc(procL, P);
	if (r == FALSE)
		return FALSE;
	else if (r == TRUE) 
		return TRUE;
	else if (r == -1) { // defragmentation is done
		while (*procL) {
			temp = *procL;
			p = (*procL)->data;
			r = insert_proc(&new_procL, p);
			if (r == FALSE) {
				free_List(new_procL);
				return FALSE;
			}
			*procL = (*procL)->next;
			free(temp);
		}
		r = insert_proc(&new_procL, P);
		if (r == FALSE) {
			free_List(new_procL);
			return FALSE;
		} else if (r == -1) {
			/* there is no space for process */
			*procL =new_procL;
			return -1;
		}
	}
	return TRUE;
	

}
int insert_proc(TList **procL, TProcess *P) {
	TList *L = *procL, *new;
	TProcess *p, *next_p;

	if (*procL == NULL) {
		*procL = alloc_list(P); /* add to the beginning */
		if (!*procL)
			return FALSE;
		P->begin_mem = 0;
		return TRUE;
	} else {
		p = L->data;
		if (p->begin_mem != 0) {
			/* check if it can be added before the first process */
			if (P->size <= p->begin_mem) {
				P->begin_mem = 0;
				new = alloc_list(P);
				if (!new)
					return FALSE;
				new->next = *procL;
				*procL = new;
				return TRUE;
			}
		}
		for (; L; L = L->next) {
			if (L->next == NULL) {
				break;
			}
			p = L->data;
			next_p = L->next->data;
			if (next_p->begin_mem - (p->begin_mem + p->size)	
					>= P->size) {
				/* process is inserted among others */
				new = alloc_list(P);
				if (!new)
					return FALSE;
				new->next = L->next;
				L->next = new;
				P->begin_mem = p->begin_mem + p->size;
				return TRUE;
			}
		}
		p = L->data;
		if ((p->begin_mem + p->size + P->size) <= MiB * 3) {
			/* add process to end of list */
			new = alloc_list(P);
			if (!new)
				return FALSE;
			L->next = new;
			P->begin_mem = p->begin_mem + p->size;
			return TRUE;
		}
	} // no process was added to memory
	return -1;
}
/* for adding processes to the queue */
int cmp_process(void *X, void *Y) {
	TProcess *x = (TProcess*)X;
	TProcess *y = (TProcess*)Y;

	if (x->priority > y->priority)
		return -1;
	else if (x->priority < y->priority)
		return 1;
	else {
		if (x->rem_time < y->rem_time)
			return -1;
		else if (x->rem_time > y->rem_time)
			return 1;
		else {
			if (x->PID < y->PID)
				return -1;
			else
				return 1;
		}
	}
}

void free_proc(void *el) {
	TProcess *P = (TProcess *)el;
	freeS(P->data, free_dataS);
	free(P);
}

void free_TMemory(TMemory *M) {
	free_List(M->proc);
	free(M->PIDs);
	freeQ(M->waiting, free_proc);
	freeQ(M->finished, free_proc);
	if (M->running != NULL)
		free_proc(M->running);
	free(M);
}
void free_dataS(void *x) {
	free(x);
}

TMemory* alloc_TMemory(int t_unit) {
	TMemory *M = (TMemory*)calloc(1,sizeof(TMemory));
	if (!M) {
		return NULL;
	}
	M->PIDs = calloc(1, PIDS_LEN);
	if (!M->PIDs){
		free (M);
		return NULL;
	}
	memset(M->PIDs, '-', PIDS_LEN);
	M->t_unit = t_unit;
	M->rem_t_unit = t_unit;
	M->proc = NULL;
	M->running = NULL;
	M->waiting = InitQ(sizeof(int));
	if (!M->waiting) {
		free(M->PIDs);
		free(M);
		return NULL;
	}
	M->finished = InitQ();
	if (!M->finished) {
		free(M->waiting);
		free(M->PIDs);
		free(M);
		return NULL;
	}
	return M;

}
