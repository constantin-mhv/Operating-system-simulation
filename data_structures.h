typedef struct list {
	void *data;
	struct list *next;
} TList;

typedef struct queue {
	int num_el;
	TList *first,
		*last;
} TQueue;

typedef struct stack {
	TList *first;
}TStack;

typedef void (*Ffree)(void *);
typedef int (*Fcmp)(void *, void *);

void* InitQ();
void* InitS();
TList* alloc_list(void *el);
int enqueue(void *el, TQueue *Q);
int push(void *el, TStack *S);
void* dequeue(TQueue *Q);
int moveQ(TQueue *Q, void **dest); //move first to dest
void* pop(TStack*S); // move first to dest
void free_dataS(void *x);
void freeQ(TQueue *Q, Ffree f);
void freeS(TStack *S, Ffree f);
void free_List(TList *L);
void remove_from_list(TList **L, void *el);
int add_to_list(TList **L, void *el);
int insertQ(TQueue *Q, void *E, Fcmp cmp, Ffree f);
int add_to_Q(TQueue *Q, void *E, Fcmp cmp, Ffree f);
void make_sameQ(TQueue *d, TQueue *s);
void remove_from_list(TList **L, void *el);
int flipS(TStack *dest, TStack *source);
