#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_structures.h"

#define TRUE 1
#define FALSE 0

void* InitQ() {
	TQueue *Q = (TQueue*)calloc(1, sizeof(TQueue));
	if (!Q)
		return NULL;
	Q->first = NULL;
	Q->last = NULL;
	Q->num_el = 0;
	return Q;
}
void* InitS() {
	TStack *S = (TStack*)calloc(1, sizeof(TStack));
	if (!S)
		return NULL;
	S->first = NULL;
	return S;
}
TList* alloc_list(void *el) {
	TList *L = (TList*)calloc(1, sizeof(TList));
	if (!L) {
		return NULL;
	}
	L->data = el;
	L->next = NULL;
	return L;
}

int add_to_list(TList **L, void *el) {
	TList *new = alloc_list(el);
	if (!new)
		return FALSE;
	if (*L == NULL){ // list was empty
		*L = new;
		return TRUE;
	}
	TList *l = *L;
	/* reach the end of the list */
	while (l->next != NULL) {
		l = l->next;
	}
	l->next = new;
	return TRUE;
}

int enqueue(void *el, TQueue *Q) {
	TList *new = alloc_list(el);
	if (!new)
		return FALSE;
	if (Q->first == NULL) {
		Q->first = new;
		Q->last = new;
	} else {
		Q->last->next = new;
		Q->last = new;
	}
	Q->num_el++;
	return TRUE;
}

int push(void *el, TStack *S) {
	TList *new = alloc_list(el);
	if (!new)
		return FALSE;
	new->next = S->first;
	S->first = new;
	return TRUE;
}

void* dequeue(TQueue *Q) {
	void *el;
	if (Q->first == NULL)
		return NULL;
	TList *temp = Q->first;
	el = Q->first->data;
	Q->first = Q->first->next;
	free(temp);
	Q->num_el--;
	return el;
}

void* pop(TStack*S) {
	void *el;
	if (S->first == NULL) {
		el = NULL;
	 	return FALSE;
	}
	TList *temp = S->first;
 	el = S->first->data;
	S->first = S->first->next;
	free(temp);
	return el;
}

/* function f is used to free element in case of error */
int insertQ(TQueue *Q, void *E, Fcmp cmp, Ffree f) {
	TQueue *q = InitQ();
	void *e;
	int r = TRUE;
	/** add new element in the queue with the cmp function,
	 * then add all elements to new queue from old queue using cmp function */
	r = add_to_Q(Q, E, cmp, f);
	if (r == FALSE){
		free(q);
		return FALSE;
		}
	int num = Q->num_el;
	while (num > 0) {
		e = dequeue(Q);
		r = add_to_Q(q, e, cmp, f);
		if (r == FALSE){
			freeQ(q, f);
			return FALSE;
		}
		num--;
	}
	make_sameQ(Q, q);
	free(q);
	return TRUE;
}

/* reverse stack */
int flipS(TStack *dest, TStack *source) {
	int r = TRUE;
	void *data;
	while (source->first) {
		data = pop(source);
		r = push(data, dest);
		if (r == FALSE){
			free(data);
			return FALSE;
		}
	}
	return TRUE;
}
/** the function f is used to free element in case of error.
 * Add item to queue using cmp  */
int add_to_Q(TQueue *Q, void *E, Fcmp cmp, Ffree f) {
	void *e;
	int r = TRUE;
	if (Q->first == NULL) {
		r = enqueue(E, Q);
		if (r == FALSE)
			return FALSE;
		return TRUE;
	}
	TQueue *q = InitQ();
	int inserted = 0;
	while (Q->first) {
		e = dequeue(Q);
		if (inserted == 0 && cmp(E, e) < 0) {
			r = enqueue(E, q);
			if (r == FALSE){
				freeQ(q, f);
				return FALSE;
			}
			inserted = 1;
		}
		r = enqueue(e, q);
		if (r == FALSE) {
			freeQ(q, f);
			return FALSE;
		}
	}
	/* add element to the end */
	if (inserted == 0) {
		r = enqueue(E, q);
		if (r == FALSE) {
			freeQ(q, f);
			return FALSE;
		}
	}
	make_sameQ(Q, q);
	free(q);
	return TRUE;
} 
void remove_from_list(TList **L, void *el) {
	TList *l = *L, *prev;
	for (; l; l = l->next) {
		if (l->data == el){
			break;
		}
		prev = l;
	}
	if ((*L)->data == l->data){ // el is first element in the list
		*L = (*L)->next;
		free(l);
	} else if (l->next == NULL) { // el is last element in list
		free(l);
		prev->next = NULL;
	} else {
		prev->next = l->next; // el is not first and last
		free(l);
	}
}

void free_List( TList *L) {
	TList *temp;
	while (L) {
		temp = L;
		L = L->next;
		free(temp);
	}
}

/* makes the destination queue equal to the source queue */
void make_sameQ(TQueue *d, TQueue *s) {
	d->first = s->first;
	d->last = s->last;
	d->num_el = s->num_el;
}

void freeQ(TQueue *Q, Ffree f) {
	TList *temp;
	while (Q->first) {
		temp = Q->first;
		f(Q->first->data);
		Q->first = Q->first->next;
		free(temp);
	}
	free(Q);
}

void freeS(TStack *S, Ffree f) {
	TList *temp;
	if (S == NULL)
		return;
	while (S->first) {
		temp = S->first;
		f(S->first->data);
		S->first = S->first->next;
		free(temp);
	}
	free(S);
}
