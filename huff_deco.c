#include <stdio.h>

#include <stdlib.h>



void huffman_decoder(FILE * ent, FILE * sal);



struct node

{

	unsigned int w; // Peso

	unsigned short p; // Padre

	unsigned short c0, c1; // 0 / 1

	unsigned char cl; // Tamaño del código

};





int main(int argv, char** argc){

	if(argv != 4){

		printf("Uso: %s [c/d] input output\n", argc[0]);

		return 0;

	}



	FILE * ent, * sal;



	if(!(ent = fopen(argc[2], "rb"))) {

		perror("ERRROR opening input file");

		return -1;

	}



    if(!(sal = fopen(argc[3], "wb"))){

		perror("ERROR opening output file");

		return -1;

	}



	if(*argc[1] == 'd')

		huffman_decoder(ent, sal);

	else

		printf("Uso: %s [d] input output\n", argc[0]);



	fclose(ent);

	fclose(sal);

	return 0;

}



void huffman_decoder(FILE * ent, FILE * sal){

	// Iterators and general purpose variables

	int i, j;

	size_t length;

	size_t rcount;



	/// Construcción de tabla de frecuencias

	unsigned int freqTable[256] = {0}; // cuenta las frecuencias de cada palabra

	unsigned char redirTab[256]; // Tabla inversa de redirección



	if(fread(&length, sizeof(size_t), 1, ent) < 1)

	{

		printf("Can't decode file.\n");

		exit(-1);

	}

	if(fread(freqTable, sizeof(unsigned int), 256, ent) < 256) // Lectura de tabla de frecuencias

	{

		printf("Can't decode file.\n");

		exit(-1);

	}



	/// Build tree

	struct node arbol[512000] = {0}; // Memoria para guardar el arbol

	unsigned short nodoLibre[256]; // Lista de nodos libres

	unsigned short a, b; // a = nodo mas pequeño; b = segundo nodo mas pequeño



	j = 0;

	for(i = 0; i < 256; i++) // Inicializar hojas y liberar nodos

	{

		if(freqTable[i])

		{

			redirTab[j] = i;

			nodoLibre[j] = j;

			arbol[j].w = freqTable[i];

			j++;

		}

	}

	unsigned short inode = j; // Siguiente nodo libre

	unsigned short lfnode = j - 1; // ultima posición ocupada en nodoLibre



	while(lfnode > 0) // Iterar por el arbol hasta que este libre

	{

		if(arbol[nodoLibre[0]].w < arbol[nodoLibre[1]].w) // Inicializar a y b

		{

			a = 0;

			b = 1;

		}

		else

		{

			b = 0;

			a = 1;

		}



		for(i = 2; i <= lfnode; i++) // Find two smallest nodes

		{

			unsigned int w = arbol[nodoLibre[i]].w;

			if(w < arbol[nodoLibre[a]].w)

			{

				b = a;

				a = i;

			}

			else

			{

				if(w < arbol[nodoLibre[b]].w) b = i;

			}

		}

		// Fill data

		unsigned short _a = nodoLibre[a];

		unsigned short _b = nodoLibre[b];

		arbol[_a].p = inode;

		arbol[_b].p = inode;

		arbol[inode].c0 = arbol[_a].cl <= arbol[_b].cl ? _a : _b;

		arbol[inode].c1 = arbol[_a].cl > arbol[_b].cl ? _a : _b;

		arbol[inode].w = arbol[_a].w + arbol[_b].w;

		arbol[inode].cl = arbol[_a].cl + 1;



		nodoLibre[a] = inode++; // Reemplaza a nodo libre con el recien creado

		nodoLibre[b] = nodoLibre[lfnode--]; // Reemplaza nodo libre b con el ultimo nodo libre



	} // Fin de la construcción del arbol



#ifdef DEBUG

	for(i = 0; i < inode; i++) printf("%d> p = %hu, c0 = %hu, c1 = %hu, w = %6u, cl = %uc\n", i, tree[i].p, tree[i].c0, tree[i].c1, tree[i].w, tree[i].cl);

#endif



	/// Decodificación



	unsigned char mask = 0x80;

	unsigned char byte = fgetc(ent);

	for(i = 0; i < length; i++) // Decodificar a tamaño original.

	{

		if(feof(sal))

		{

			printf("Can't decode file.\n");

			exit(-1);

		}

		unsigned short curr = inode - 1;

		while(arbol[curr].cl) // Seguimiento de las ramas

		{

			curr = byte & mask ? arbol[curr].c1 : arbol[curr].c0;

			mask >>= 1;

			if(!mask)

			{

				mask = 0x80;

				byte = fgetc(ent);

			}

		}

		fputc(redirTab[curr], sal);

	}

}
