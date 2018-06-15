/*******************************************************************************
** Miles Bagwell
**
** ECE 4680 Spring 2016
** Lab 4 LZW Codec
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "functions.h"

////////////////////////////////////////////////////////////////////////////////
int *get_frequencies(FILE *fin){
	int i;
	int *occurances;
	unsigned char current;

	occurances = calloc(256,sizeof(int));

	while(fread(&current,1,1,fin) == 1){
		i = (int) current;
		occurances[i]++;

	}
	
	return occurances;
}
////////////////////////////////////////////////////////////////////////////////
list_t * list_construct(int *occurances){
    list_t *L;
	int i;

    L = (list_t *) malloc(sizeof(list_t));
	L->codes = (unsigned char **) malloc(256 * sizeof(unsigned char *));

    L->head = NULL;
    L->tail = NULL;
    L->entries = 0;

	// Fill List
	for(i=0; i<256; i++){
	    if(occurances[i] != 0){
			list_insert_sorted(L, i, occurances[i]);
		}
	}
	
	L->size = L->entries;
	
    return L;
}
////////////////////////////////////////////////////////////////////////////////
void list_insert_sorted(list_t *L, int symbol, int count){
    node_t *N, *temp;
    node_t *Left, *Right;

	// Allocate New Node
    N = (node_t *) malloc(sizeof(node_t));

	// Create Parent Node
	if (symbol == -1){
		Left = L->head;
		Right = Left->next;

		Left->P  = N;
		Right->P = N;

		N->char_flag = 0;
		N->freq_value = Left->freq_value + Right->freq_value;
		N->L = Left;
		N->R = Right;
		N->P = NULL;

		L->head = Right->next;

		L->entries -= 2;

	}

	// Character Node
	else {
		N->letter = (unsigned char) symbol;
		N->char_flag = 1;
    	N->freq_value = count;
    	N->L = NULL;
    	N->R = NULL;
    	N->P = NULL;

	}	

	// Empty List
    if (L->entries == 0){
        N->prev = N->next = NULL;
        L->head = N;
        L->tail = N;
    }

	// Insert into Head
	else if (N->freq_value < L->head->freq_value){
		N->next = L->head;
		N->prev = NULL;
		L->head->prev = N;
		L->head = N;
	}
	
	// Insert into Tail
    else if (N->freq_value >= L->tail->freq_value){
		N->next = NULL;
		N->prev = L->tail;
		L->tail->next = N;
		L->tail = N;
	}

	else{

		temp = L->head;

		while(N->freq_value >= temp->freq_value){
			temp = temp->next;
		} 
		
		N->next = temp;
		N->prev = temp->prev;
		temp->prev->next = N;
		temp->prev = N;

    }

	L->entries++;
}
////////////////////////////////////////////////////////////////////////////////
void tree_construct(list_t *L){

	L->smallest = L->head;

	while(L->entries > 1){
		list_insert_sorted(L, -1, 0);
	}

}
////////////////////////////////////////////////////////////////////////////////
void get_codes(list_t *L){
	node_t *temp = L->smallest;

	// Find Codes
	while(temp != NULL){
	    if(temp->char_flag == 1){
			find_code(L,temp);
		}

		temp = temp->next;

	}		
}
////////////////////////////////////////////////////////////////////////////////
void find_code(list_t *L, node_t *N){
	int i, character, size = 0;
	unsigned char code[80];
	node_t *temp, *temp2;

	temp = N;
	
	i = 0;

	while(temp != L->head){
		temp2 = temp->P;

		if (temp2->L == temp){
			code[i] = (unsigned char)(0x30);
		}
		else if (temp2->R == temp){
			code[i] = (unsigned char)(0x31);
		}
		else {
			printf("Whhhaaaaaaaaattt\n");
			exit(1);
		}

		
		temp = temp->P;
		size++;
		i++;
	}

	character = (int) N->letter;
	L->lengths[character] = size; 
	L->codes[character] = (unsigned char *) malloc(size * sizeof(unsigned char));
	
	
	int j = size-1;
	for (i = 0; i < size; i++, j--){
		L->codes[character][i] = code[j];

	}

		
	

}
////////////////////////////////////////////////////////////////////////////////

void encode(FILE *fin, FILE *fout, list_t *L)
{
    // Create sweet variables
    unsigned char c;
    unsigned char byte = 0x00;
    unsigned char *c_code = (unsigned char *)calloc(256, sizeof(unsigned char));
    int buffer_size = 0;
    int byte_length = 0;
   

    int control = 0;    

    // Read entire file
    while(fread(&c, sizeof(unsigned char), 1, fin) == 1)
    {
	 memset(c_code, '\0', sizeof(c_code));
	 buffer_size = L->lengths[(int)c];
	 strcpy(c_code,L->codes[(int)c]);	
	 int i = 0;
	
	 //if(control < 5)
	 //fprintf(stdout, "c_code = (%s) :: buffer_size = (%d).\n", c_code, buffer_size);

	 // Loop through entire c_code 
	 while(i < buffer_size)
	 {
	     // If byte is full, write it out
	     if(byte_length == 8)
	     {
		 //if(control < 5)
		 //fprintf(stdout, "byte = (%X).\n", (int)byte);
	         fwrite(&byte, sizeof(unsigned char), 1, fout);
		 byte = 0x00;
		 byte_length = 0;
	     }

	
	     byte <<= 1;
	

	     // Does c_code[i] == '1'?
	     if(c_code[i] == (unsigned char)(0x31))
		byte |= 0x01;

	     byte_length++;	     
	     i++;
	 }
	 //control++; 
    }
    
    // If any leftover bits, write them out
    if(byte_length > 0)
    {
	byte <<= (8-byte_length);
	fwrite(&byte, sizeof(unsigned char), 1, fout);
    }	
}

////////////////////////////////////////////////////////////////////////////////
void decode(FILE *fin, FILE *fout, list_t *L, int fsize)
{
    int matches_made = 0;
    unsigned char *byte = (unsigned char *)calloc(1, sizeof(unsigned char));     
    int byte_length = 0;

    while(matches_made < fsize)
    {
	// String used to check for list code matches
	unsigned char *str = (unsigned char *)calloc(256, sizeof(unsigned char));
	int match = -1;
	int str_len = 0;	

	while(1)
	{
	    // Read new byte
	    if(byte_length == 0)
	    {
		memset(byte, '\0', sizeof(byte));
	        fread(byte, sizeof(unsigned char), 1, fin);
		//fprintf(stdout, "byte = (%x).\n", (int)*byte);
	        byte_length = 8;
	    }
	    
	    // Add new byte to 'str'
	    add_to_str(byte, &byte_length, str, &str_len);
	    //fprintf(stdout, "str = (%s).\n", str);
	    
	    // Check for a match
	    match = find_match(L, str, str_len);
	    if(match != -1)
	    {
		matches_made++;
		//fprintf(stdout, "MATCH: str = (%s) :: L->codes[%d] = (%s).\n", str, match, L->codes[match]);
	    	break;
	    }
	}

	// Reset str
	memset(str, '\0', sizeof(str));
	//if(str_len > 8)
	    //fprintf(stdout, "codes[match] = (%s).\n", L->codes[match]);
	
	// Write out result
	unsigned char blah = (unsigned char)match;
	fwrite(&blah, sizeof(unsigned char), 1, fout);
    }
    //fprintf(stdout, "Matches_made = (%d) :: limit = (%d).\n", matches_made, fsize);
}

////////////////////////////////////////////////////////////////////////////////
void add_to_str(unsigned char *byte, int *byte_len, unsigned char *str, int *str_len)
{
    unsigned char *bit = (unsigned char *)calloc(1, sizeof(unsigned char));
  
    // Extract bit and shift
    bit[0] |= (byte[0] & 0x80);
    byte[0] <<= 1;
    (*byte_len)--;
	
    // Compare
    if(bit[0] == (unsigned char)(0x80))
        str[*str_len] = (unsigned char)(0x31);
    else
        str[*str_len] = (unsigned char)(0x30);

    (*str_len)++;
    
    free(bit);
    bit = NULL;
}

////////////////////////////////////////////////////////////////////////////////
int find_match(list_t *L, unsigned char *str, int str_len)
{
    int i;
    int loc = -1;
    for(i = 0; i < 256; i++)
    {
	if(str_comp(L->codes[i], L->lengths[i], str, str_len))
	{
	    loc = i;
	    break;
	}
    }
	
    return loc;
}

////////////////////////////////////////////////////////////////////////////////
int str_comp(unsigned char *a, int a_len, unsigned char *b, int b_len)
{
    if(a_len == b_len)
    {
	int i;
	for(i=0; i<a_len; i++)
	    if((int)a[i] != (int)b[i])
		return 0;
	return 1;
    }
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
int get_freq_sum(int *freq_list)
{
    int i;
    int sum = 0;
    for(i=0; i<256; i++)
    {
	sum += freq_list[i];
    }
    return sum;
}
