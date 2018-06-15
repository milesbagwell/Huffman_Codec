/*******************************************************************************
** Miles Bagwell
**
** ECE 4680 Spring 2016
** Lab 4 Huffman Codec
*******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "functions.h"


int main(int argc, char **argv){
	char *fileName = NULL, *fileOut = NULL;
	FILE *fin,*fout;
	float diff;
	int *occurances;
	int i;
	int comp = 0, decomp = 0;
	int csize, fsize;
	list_t *List;


	// Argument Processing
	if (argc != 4){
			printf("Usage: lab2 -c <file to compress>   <output file name>\n");
			printf("       lab2 -d <file to decompress> <output file name>\n");
			exit(1);
	}

	i = getopt(argc, argv, "c:d");

		switch (i){
			case 'c':
				comp = 1;
				fileName = argv[2];
				fileOut  = argv[3];
				break;
			case 'd':
				decomp = 1;
				fileName = argv[2];
				fileOut  = argv[3];
				break;
			case '?':
				if (isprint(optopt))
					fprintf(stderr, "Unknown option %c.\n", optopt);
				else
					fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			default:
				printf("Usage: lab3 -c <file to compress> <output file name>\n");
				printf("       lab3 -d <file to decompress> <output file name>\n");
				exit(1);
		}


	fin = fopen(fileName, "rb");
	fout = fopen(fileOut, "wb");


	fseek(fin,0,SEEK_END);
	fsize = ftell(fin);
	fseek(fin,0,SEEK_SET);


	if (fin == NULL){
		printf("Error Reading File\n");
		exit(1);
	}


	// Compression
	if (comp == 1){ 
		printf("Compressing %s\n",fileName);

		// Find Frequencies
		occurances = get_frequencies(fin);

	        fseek(fin,0,SEEK_SET);
		int temp, i;
		for(i=0;i<256;i++)
		{
		    temp = occurances[i];
		    fwrite(&temp, sizeof(int), 1, fout);
		    //fprintf(stdout, "occurances[%d] = (%d).\n", i, occurances[i]);
		}

		// Make List
		List = list_construct(occurances);

		// Make Tree
		tree_construct(List);

		// Find Code Values
		get_codes(List);

		// Just a test print of letter 137
		// Result should be '0001'
		/*for(i=0;i<256;i++)
		{
		     if(List->codes[i] != NULL)
		     	printf("codes[%d] = (%s)\n", i, List->codes[i]);
		}
		*/
		// Write out original file size
		//fwrite(fsize, sizeof(int), 1, fout);

		// Encode the file
                encode(fin, fout, List);

		// Print Compression Info
		csize = ftell(fout);
		printf("Original Size   = %d\n",fsize);
		printf("Compressed Size = %d\n",csize);
		diff = (((float)(csize-fsize))/(float)(fsize))*100;
		printf("%.2f%% Compression\n",diff);
	}



	// Decompression
	else if (decomp == 1){ 
		printf("Decompressing %s\n",fileName);


		//int original_fsize;
		//fread(&original_fsize, sizeof(int), 1, fin);
		
		occurances = (int *)calloc(256, sizeof(int));
		// Get frequency list
		int temp, i;
		for(i=0;i<256;i++)
		{
		    fread(&temp, sizeof(int), 1, fin);
		    occurances[i] = temp;
		    //fprintf(stdout, "occurances[%d] = (%d).\n", i, occurances[i]);
		}
		int limit = get_freq_sum(occurances);		
	

		// Make List
		List = list_construct(occurances);

		// Make Tree
		tree_construct(List);

		// Find Code Values
		get_codes(List);
		

		// Just a test print of letter 137
		// Result should be '0001'
		/*
		for(i=0;i<256;i++)
		{
		     if(List->codes[i] != NULL)
		     {
		     	 printf("codes[%d] = (%s)\n", i, List->codes[i]);
			 //printf("lengths[%d] = (%d).\n", i, List->lengths[i]);
		     }
		}
		*/
		// Decode file
		decode(fin, fout, List, limit);
	

	}
	fclose(fin);
	fclose(fout);
	return 0;
}

