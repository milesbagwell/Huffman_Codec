/*******************************************************************************
** Miles Bagwell
**
** ECE 4680 Spring 2016
** Lab 4 LZW Codec
*******************************************************************************/

typedef struct node_tag_t 
{
    int char_flag;				//Either 0 or 1
    int freq_value;	        	//Value of frequency
    unsigned char letter;   	//letter value
    struct node_tag_t *L;		//Left leaf
    struct node_tag_t *R;		//Right leaf
    struct node_tag_t *P;   	//Parent leaf
    struct node_tag_t *prev;   	
    struct node_tag_t *next;   	
} node_t;

typedef struct list_tag {
    node_t *head;
    node_t *tail;
	node_t *smallest;
	unsigned char **codes;
	int lengths[256];
    int entries;
	int size;
} list_t;



int *get_frequencies(FILE *);
int get_freq_sum(int *freq_list);
list_t * list_construct(int *);
void list_insert_sorted(list_t *, int, int);
void tree_construct(list_t *);
void get_codes(list_t *);
void find_code(list_t *, node_t *);

void encode(FILE *fin, FILE *fout, list_t *L);
unsigned char *build_c_buff(unsigned char c, int c_len, unsigned char *c_code);

void decode(FILE *fin, FILE *fout, list_t *L, int fsize);
void add_to_str(unsigned char *byte, int *byte_len, unsigned char *str, int *str_len);
int find_match(list_t *L, unsigned char *str, int str_len);
int str_comp(unsigned char *a, int a_len, unsigned char *b, int b_len);


