/*
QUELLEN:
	https://www.tutorialspoint.com/cprogramming/c_file_io.htm // Read / Write rechte
	http://www.dreamincode.net/forums/topic/170054-understanding-and-reading-binary-files-in-c/ // File öffnen + in Hexa
	https://visuellegedanken.de/wp-content/uploads/2009/11/wolf_20091112_029.jpg // jpg img
	// raw img?

*/
/*
DOKU:
	6.3.
  1h Codesuchen für fileopen mit raw für gutes format (Hexa kam raus)
  45min für umschreiben von den C++ eigenheiten auf C
  1h um raw bilder überhaupt bearbeiten zu können...
  1h code umgeschrieben um unterschiede im bild zu erkennen (Prozentanzeige noch Buggy. Vielleicht weil Filename?)
	Ja, wegen Metadaten => http://netpbm.sourceforge.net/doc/ppm.html
  
	9.3.
  30 min Bugbehebung, dass größere Files korrupt werden.
  30 min Metadaten herausfinden und auslesen/abfragen http://netpbm.sourceforge.net/doc/ppm.html
  20 min Bugbehebung, dass %-Abweichung falsch angezeigt werden (Metadaten benötigt)
  30 min 3D-Array erstellen und jeder koordinate ihre R,G,B Werte zuweisen.

*/
/*
TODO:
	Fail: Dif-Bilder schauen jetzt unterschiedlich aus. bei 100x100 fehlen teile des Bildes, bei 10x10 schreibt er was, was nicht da ist...
	Check: % Anzeige stimmt nicht. Glaube das liegt am Filenamen usw, was vorm ersten Pixel kommt.
*/

/* unsafe warning wird ausgeblendet */
#define _CRT_SECURE_NO_WARNINGS 
#define HEIGHT 100
#define WIDTH 67

#include <stdio.h>
//#include <iostream>
//#include<conio.h>
#include<stdlib.h>

// An unsigned char can store 1 Bytes (8bits) of data (0-255)
typedef unsigned char BYTE;
void printImg(long fileSize, BYTE *fileBuf, int width);
void getKoordinates(long fileSize, BYTE *fileBuf);

struct metadata{
	int width;
	int height;
	int color;
}metad;

int main()
{
	int lengh = 0;
	double dif = 0;
	int width = 30;

	// 2 Bilder einlesen
	//const char *filePathOld = "auto100_1.ppm";
	//const char *filePathNew = "auto100_kopf.ppm"; // Kopf
	//const char *filePathOld = "auto100_1.ppm";
	//const char *filePathNew = "auto100_2.ppm";
	const char *filePathOld = "auto100_1.ppm";
	const char *filePathNew = "auto100_kopf.ppm"; // Kopf
	const char *filePathDif = "auto100_dif100.ppm";

	BYTE *fileBufOld;		// Pointer to our buffered data
	BYTE *fileBufNew;		// Pointer to our buffered data
	FILE *fpOld;			// File pointer
	FILE *fpNew;			// File pointer
	FILE *fpDif;			// File pointer für Dif im Bild

	// Open the file in binary mode using the "rb" format string
	// This also checks if the file exists and/or can be opened for reading correctly
		// TODO: Check habe ich derzeit rausgenommen 
	/*
	"r"		Opens a file for reading. The file must exist.
	"w"		Creates an empty file for writing. If a file with the same name already exists, its content is erased and the file is considered as a new empty file.
	"a"		Appends to a file. Writing operations, append data at the end of the file. The file is created if it does not exist.
	"r+"	Opens a file to update both reading and writing. The file must exist.
	"w+"	Creates an empty file for both reading and writing.
	"a+"	Opens a file for reading and appending.
	*/
	fpOld = fopen(filePathOld, "r");
	fpNew = fopen(filePathNew, "r");
	fpDif = fopen(filePathDif, "w");


	//fseek(fpNew, 0, SEEK_END); // seek to end of file
	//long fileSizeNew = ftell(fpNew); // get current file pointer
	//fseek(fpNew, 0, SEEK_SET); // seek back to beginning of file
	//					   // proceed with allocating memory and reading the file
	//fseek(fpOld, 0, SEEK_END); // seek to end of file
	//long fileSizeOld = ftell(fpOld); // get current file pointer
	//fseek(fpOld, 0, SEEK_SET); // seek back to beginning of file
	//						   // proceed with allocating memory and reading the file

	// Get the size of the file in bytes
	long fileSizeOld = getFileSize(fpOld);
	long fileSizeNew = getFileSize(fpNew) + 1000;

	// Allocate space in the buffer for the whole file	
	// fileBuf = new BYTE[fileSize];
	fileBufOld = (BYTE*)malloc(fileSizeOld); // umgeschrieben auf malloc
	fileBufNew = (BYTE*)malloc(fileSizeNew); // umgeschrieben auf malloc

	// Now that we have the entire file buffered, we can take a look at some binary infomation
	// Lets take a look in hexadecimal
	printf("BILDER 1\n");
	printf(" "); // Damit die Formatierung von Bild 1 und 2 gleich aussieht (NUR FÜR KONSOLE)

	// Read the file in to the buffer
	fread(fileBufOld, fileSizeOld, 1, fpOld);
	//printImg(fileSizeOld, fileBufOld, width);


	printf("\n\nBILDER 2\n ");
	// Read the file in to the buffer
	fread(fileBufNew, fileSizeNew, 1, fpNew);
	//printImg(fileSizeNew, fileBufNew, width);


	printf("\n\nBILDER VERGLEICH\n ");
	// TODO: Unterschiedliche Länge hat sich (glaub ich) gelöst. Muss noch checken.
	// Aus irgend einem Grund ist die FileSizeNew und FileSizeOld unterschiedlich
	// Darum wird hier verglichen welche länger ist, und die längere genommen.
	if (fileSizeOld >= fileSizeNew)
		lengh = fileSizeOld;
	else lengh = fileSizeNew;

	/*
	for (int i = 0; i < lengh; i++) {
	   if (fileBufNew[i] == fileBufOld[i]) {
		   printf("   "); // 3 leere Zeichen um Formatierung zu halten
		   // fileBufNew[i] = fileBufOld[i] - fileBufNew[i];
	   }
	   else {
		   int j = fileBufOld[i] - fileBufNew[i]; // TODO: Voneinander Abziehen um den Hexa-Wert als unterschied zu bekommen
												  // Nicht unbedingt sinnhaft.
		   printf("%X ", j);
		   dif++;
	   }
	   if (i % width == 0 && i != 0)
		   printf("\n ");
   }
   */

	dif = dif * 100 / (lengh - metadata(fileSizeNew, fileBufNew));
	printf("\nProzentuelle Abweichung: %.2f %%", dif);

	for (int o = 0; o < fileSizeNew; o++){
		//if (fileBufNew[o] == EOF)
		//	break;
		if (o == 52)
			o = 53;
		char c = fileBufNew[o];
		fprintf(fpDif, "%c", c);
	}
	// fwrite(fileBufNew, fileSizeNew, 1, fpDif);
	// getKoordinates(fileSizeNew, fileBufNew);

	//while (1) {
		// damit sich konsole nicht schließt
	//}
	
	/*cin.get();
	delete[]fileBuf;*/
	fclose(fpOld);
	fclose(fpNew);
	fclose(fpDif);
	return 0;
}

// Get the size of a file
long getFileSize(FILE *file)
{
	long lCurPos, lEndPos;
	lCurPos = ftell(file);
	fseek(file, 0, 2);
	lEndPos = ftell(file);
	fseek(file, lCurPos, 0);
	return lEndPos;
}

void printImg(long fileSize, BYTE *fileBuf, int width) {
	for (int i = 0; i < fileSize; i++) {
		printf("%X ", fileBuf[i]);
		if (i % width == 0 && i != 0)
			printf("\n ");
	}
}

void getKoordinates(long fileSize, BYTE *fileBuf) {
	int x, y;	// Koordinatensystem
	int image[WIDTH][HEIGHT][3]; // höhe, breite, pixel

	// Index der Metadaten holen, da diese nicht gemappt werden sollen
	int i = metadata(fileSize, fileBuf);

	// Zuweisung der r, g, b Werte auf die jeweiligen Koordinaten
	for (x = 0; x < metad.width; x++) {
		for (y = 0; y < metad.height; y++) {
			image[x][y][0] = fileBuf[i++]; // r
			image[x][y][1] = fileBuf[i++]; // g
			image[x][y][2] = fileBuf[i++]; // b
		}
	}
	// Kompremierung 

}

int metadata(long fileSize, BYTE *fileBuf) {
	int i = 0;
	int j = 0; // Zum zählen der newlines (4 in den Metadaten)
	char width[10];
	char height[10];
	char color[10];
	char *pwidth = width;
	char *pheight = height;
	char *pcolor = color;

	if (!(fileBuf[0] == 'P' && fileBuf[1] == '6' && fileBuf[2] == '\n'))
		return 0;

	// Check auf Kommentare
	if (fileBuf[3] == '#') {
		i = 4;
		while (fileBuf[i] != '\n') {
			i++;
		}
		i += 1;
	}

	while (fileBuf[i] != ' ') {
		*(pwidth++) = fileBuf[i++];
	}
	*pwidth = '\0';
	i += 1; // Da sonst auf leerzeichen
	while (fileBuf[i] != '\n') {
		*(pheight++) = fileBuf[i++];
	}
	*pheight = '\0';
	i += 1; // Da wir schon auf den newline stehen
	while (fileBuf[i] != '\n') {
		*(pcolor++) = fileBuf[i++];
	}
	*pcolor= '\0';
	
	metad.color = strtol(color, NULL, 10);
	metad.height = strtol(height, NULL, 10);
	metad.width = strtol(width, NULL, 10);
	
	/*for (i; i < fileSize; i++) {
			
		if (fileBuf[i] == 0x0a)
			j++;
		if (j == 4)
			break;
	}*/
	return i + 1; // Da ich sonst auf \n stehe
}