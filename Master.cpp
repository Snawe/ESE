/*
QUELLEN:
	https://www.tutorialspoint.com/cprogramming/c_file_io.htm // Read / Write rechte
	http://www.dreamincode.net/forums/topic/170054-understanding-and-reading-binary-files-in-c/ // File öffnen + in Hexa
	https://visuellegedanken.de/wp-content/uploads/2009/11/wolf_20091112_029.jpg // jpg img
	https://www.johndcook.com/blog/2009/08/24/algorithms-convert-color-grayscale/ // Schwarz / Weiß rechnen

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

  Dazwischen i.wann (stunde)
  565 auf 888 wandeln.


  1.4.
	30 min Debug prints einfügen für leichteres error handeln
	2h versucht herauszufinden warum ein fread und fwrite gleich hintereinander unterschiedliche Bilder ausgeben. Kläglich gescheitert

  10.4
	2h Konzept für Statistik ausdenken (recherche)
		Grundgedanke: 
		* Nehme 2 Array's (Old and New)
		* Old ist ein 2 dimensionaler Array und hat alle Werte des zuletzt gespeicherten Bildes + µ und sigma
		* New ist ein 1 dimensionaler Array und hat die Werte des Aktuell zu überprüfenden Bildes.
		* Passt Wert von New zu Old +- sigma 
			=> Keine Änderung vorhanden.
			=> Wert von Old mit New überschreiben.
		* Passt der Wert von New nicht zu Old
			=> Änderung vorhanden.
			=> µ anpassen.
			=> Wert von Old mit New überschreiben.
	2h Grundgerüst der Funktion aufbauen.
		Versucht von Verschiedenen Möglichkeiten um µ und sigma einzulesen für jedes Pixel (2D + Struct oder 3D)
			Entscheidung ist erstmal gefallen auf 2 Arrays, einen für die Pixel und einen 2D für µ und Sigma.
		Konzept verfeinert um die einzelnen Werte abzuspeichern.
  11.4.
	2h für die Rohfassung und Fehlerbehandlung ob sich die Abweichung richtig berechnet
		if (count == 1) {
		int **sValue = (BYTE*)malloc(fileSize * sizeof(int));// Rows
		for (int i = 0; i < fileSize; i++) { //Colums
		sValue[i] = malloc(2);
		}

		// µ und sigma setzen. (erste Aufruf)
		for (int i = 0; i < fileSize; i++) {
		sValue[i][0] = fileBufOld[i]; // my = fileBufOld[i];	=> Beim Initialisieren wird µ als erster Wert des Pixels angenommen.
		sValue[i][1] = sigma; // Std. Value von 25 (empirisch)
		}

		// CALCULATION
		for (int i = 0; i < fileSize; i++) {
		if ((fileBufNew[i] <= (sValue[i][0] - sValue[i][1])) || (fileBufNew[i] >= (sValue[i][0] + sValue[i][1]))) {
		if (DEBUG_PRINT)
		printf("fileBufNew %i ist größer oder kleiner µ +- sigma\n");
		statDif++;
		sValue[i][0] = changeMy(count, fileBufNew[i], sValue[i][0]);
		}
		//fileBufOld[i] = fileBufNew[i];
		}
	2h Bug behebung, dass nach dem ersten Bild danach immer 0% steht, bzw. nach der "Lösung" eine pointer exception aufgetreten ist.
		Behoben, indem ich die ganze Initialisierung aus der Methode in eine andere gehoben habe. Somit überschreibt sich nicht ständig der Inhalt was zu falschen Ergebnissen geführt hat.

	30min Visual Studio mit Git verbinden^^

	12.04.
	~45min Recheche ob es eine bessere Methode als nur µ anpassen gibt.
	1h Quellen suchen zu gescheiter moving standard deviation:
	http://stackoverflow.com/questions/14635735/how-to-efficiently-calculate-a-moving-standard-deviation C#
	http://jonisalonen.com/2014/efficient-and-accurate-rolling-standard-deviation/ Pyphon
	+ versuchen umzusetzen
		

*/
/*
TODO:
	Fail: Dif-Bilder schauen jetzt unterschiedlich aus. bei 100x100 fehlen teile des Bildes, bei 10x10 schreibt er was, was nicht da ist...
	Check: % Anzeige stimmt nicht. Glaube das liegt am Filenamen usw, was vorm ersten Pixel kommt.
*/

/* unsafe warning wird ausgeblendet */
#define _CRT_SECURE_NO_WARNINGS 
#define HEIGHT 144
#define WIDTH 176
#define DEBUG_PRINT 0
#define DEBUG_WAIT 0
#define PPM 0
#define PNM 1

#include <stdio.h>
//#include <iostream>
//#include<conio.h>
#include<stdlib.h>
#include <stdint.h>
#include <math.h>

// An unsigned char can store 1 Bytes (8bits) of data (0-255)
typedef unsigned char BYTE;
void printImg(long fileSize, BYTE *fileBuf, int width);
void convert(BYTE *frmBuffer);
void getKoordinates(long fileSize, BYTE *fileBuf);
int calcStat(long fileSize, BYTE *fileBufOld, BYTE *fileBufNew, int offset);

struct metadata{
	int width;
	int height;
	int color;
}metad;

//struct statistic {
//	int N;
//	int average;
//	int variance;
//	int stddev;
//	int old;
//}statistic;

struct statistic {
	int N;
	int average;
	int variance;
	int stddev;
	int old;
};

int main()
{
	int lengh = 0;
	double dif = 0;
	int width = 30;

	const char *filePath[10] = {
		"Grayscale_Static\\131351633974511983.pnm" ,
		"Grayscale_Static\\131351633985001584.pnm" ,
		"Grayscale_Static\\131351633995532702.pnm" ,
		"Grayscale_Static\\131351634006337327.pnm" ,
		"Grayscale_Static\\131351634016712236.pnm" ,
		"Grayscale_Static\\131351634026567246.pnm" ,
		"Grayscale_Static\\131351634036888307.pnm" ,
		"Grayscale_Static\\131351634047768552.pnm" ,
		"Grayscale_Static\\131351634062397230.pnm" ,
		"Grayscale_Static\\131351634074522928.pnm" };
	const char *filePathChange[10] = {
		"Grayscale_Static\\_131351633974511983.pnm" ,
		"Grayscale_Static\\_131351633985001584.pnm" ,
		"Grayscale_Static\\_131351633995532702.pnm" ,
		"Grayscale_Static\\_131351634006337327.pnm" ,
		"Grayscale_Static\\_131351634016712236.pnm" ,
		"Grayscale_Static\\_131351634026567246.pnm" ,
		"Grayscale_Static\\_131351634036888307.pnm" ,
		"Grayscale_Static\\_131351634047768552.pnm" ,
		"Grayscale_Static\\_131351634062397230.pnm" ,
		"Grayscale_Static\\_131351634074522928.pnm" };
	// 2 Bilder einlesen und speichern in neuen
	const char *filePathOld = "Grayscale_Static\\131351633974511983.pnm";
	const char *filePathNew = "Grayscale_Static\\131351634006337327.pnm"; // Kopf
	const char *filePathDif = "Grayscale_Static\\testa.pnm";

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

	// Get the size of the file in bytes
	long fileSizeOld = getFileSize(fpOld);
	long fileSizeNew = getFileSize(fpNew);

	// Allocate space in the buffer for the whole file	
	// fileBuf = new BYTE[fileSize];
	fileBufOld = (BYTE*)malloc(fileSizeOld); // umgeschrieben auf malloc
	fileBufNew = (BYTE*)malloc(fileSizeNew); // umgeschrieben auf malloc

	// Now that we have the entire file buffered, we can take a look at some binary infomation
	// Lets take a look in hexadecimal
	printf("BILD 1\n");
	printf(" "); // Damit die Formatierung von Bild 1 und 2 gleich aussieht (NUR FÜR KONSOLE)

	// Read the file in to the buffer
	fread(fileBufOld, fileSizeOld, 1, fpOld);
	if (DEBUG_PRINT)
		printImg(fileSizeOld, fileBufOld, width);


	printf("\n\nBILD 2\n ");
	// Read the file in to the buffer
	fread(fileBufNew, fileSizeNew, 1, fpNew);
	if (DEBUG_PRINT)
		printImg(fileSizeNew, fileBufNew, width);
	// TODO: WTF???? Ich lese nur aus, schreibe sofort wieder rein und das bild schaut anders aus........
	//
	//fwrite(fileBufNew, fileSizeNew, 1, fpDif);

	/*convert(fileBufNew);
	convert(fileBufOld);*/

	printf("\n\nBILDER VERGLEICH\n ");
	// TODO: Unterschiedliche Länge hat sich (glaub ich) gelöst. Muss noch checken.
	// Aus irgend einem Grund ist die FileSizeNew und FileSizeOld unterschiedlich
	// Darum wird hier verglichen welche länger ist, und die längere genommen.
	if (fileSizeOld >= fileSizeNew)
		lengh = fileSizeOld;
	else lengh = fileSizeNew;

	for (int i = 0; i < lengh; i++) {
		if (fileBufNew[i] == fileBufOld[i]) {
			if (DEBUG_PRINT)
				printf("   "); // 3 leere Zeichen um Formatierung zu halten
			// fileBufNew[i] = fileBufOld[i] - fileBufNew[i];
		}
		else {
			int j = fileBufOld[i] - fileBufNew[i]; // TODO: Voneinander Abziehen um den Hexa-Wert als unterschied zu bekommen
													// Nicht unbedingt sinnhaft.
			if (DEBUG_PRINT)
				printf("%X ", j);
			dif++;
		}
		if (i % width == 0 && i != 0)
			if (DEBUG_PRINT)
				printf("\n ");
	}
   
	int metad = metadata(fileBufNew);
	dif = dif * 100 / (lengh - metad);
	printf("\nProzentuelle Abweichung (Pixel gegen Pixel): %.2f %%\n", dif);

	//int **sValue = (BYTE*)malloc(fileSizeNew * sizeof(int));// Rows
	//for (int i = 0; i < fileSizeNew; i++) { //Colums
	//	sValue[i] = malloc(2);
	//}

	//for (int i = 0; i < 10; i++) {
	//	double calcdif = calcStat(fileSizeNew, fileBufOld, fileBufNew, metad, sValue);
	//	calcdif = calcdif * 100 / (lengh - metad);
	//	printf("Prozentuelle Abweichung (Statistisch): %.2f %%\n", calcdif);
	//}
	/*statistic.N = 10;
	statistic.average = 128;
	statistic.variance = 128;
	statistic.stddev = sqrt(statistic.variance);
	int first = 128;
	statistic.old = first;	
	int val = fileBufNew;*/
	int first = 128;
	struct statistic *statistic = (BYTE*)malloc(fileSizeNew);
	

	for (int j = 0; j < 10; j++) {
		fpNew = fopen(filePath[j], "r");
		fpDif = fopen(filePathChange[j], "w");
		fread(fileBufNew, fileSizeNew, 1, fpNew);
		for (int i = metad; i < fileSizeNew; i++) {
			rollingStatistic(i, fileBufNew[i]);
			if (fileBufNew[i] > (statistic[i].average + statistic[i].stddev * 3))
				fileBufNew[i] = 0xff;
			else if (fileBufNew[i] < (statistic[i].average + statistic[i].stddev * 3))
				fileBufNew[i] = 0xFF;	
		}
		fwrite(fileBufNew, fileSizeNew, 1, fpDif);
		fclose(fpDif);
		fclose(fpNew);
	}
	//for (int o = metad; o < fileSizeNew; o++){
	//	//if (fileBufNew[o] == EOF)
	//	//	break;
	//	/*if (o == 52)
	//		o = 53;*/
	//	char c = fileBufNew[o];
	//	fprintf(fpDif, "%c", c);
	//}
	// fwrite(fileBufNew, fileSizeNew, 1, fpDif);
	// getKoordinates(fileSizeNew, fileBufNew);

	if (DEBUG_WAIT) {
		char wait = '0';
		while (wait == '0') {
			scanf("%c", &wait);
		}
	}
	
	/*cin.get();
	delete[]fileBuf;*/
	fclose(fpOld);
	fclose(fpNew);
	fclose(fpDif);
	return 0;
}
int rollingStatistic(int i, int val) {
	static int count = 0;
	struct statistic *statistic;
	if (count == 0) {
		statistic[i].N = 10;
		statistic[i].average = 128;
		statistic[i].variance = 128;
		statistic[i].stddev = sqrt(statistic[i].variance);
		statistic[i].old = 128;
		count++;
	}

	int oldavg = statistic[i].average;
	int newavg = oldavg + (val - statistic[i].old) / statistic[i].N;
	// print(str(oldavg) + " " + str(newavg))
	/*printf("oldavg = %i\n", oldavg);
	printf("newavg = %i\n", newavg);*/
	statistic[i].average = newavg;
	int newvar = (val - statistic[i].old)*(val - newavg + statistic[i].old - oldavg) / (statistic[i].N - 1);
	if ((statistic[i].variance + newvar) < 0)
		newvar = statistic[i].variance;
	statistic[i].variance = newvar;
	//print("V:" + str(self.variance))
	/*printf("variance = %i\n", statistic.variance);*/
	statistic[i].stddev = sqrt(statistic[i].variance);
	statistic[i].old = val;

	//count++;
}

int calcStat(long fileSize, BYTE *fileBufOld, BYTE *fileBufNew, int offset, int **sValue) {
	static int count = 1;
	int sigma = 20;
	int my = 0;
	int statDif = 0;
	
	if (count == 1) {	
		// µ und sigma setzen. (erste Aufruf)
		for (int i = 0; i < fileSize; i++) {
			sValue[i][0] = fileBufOld[i]; // my = fileBufOld[i];	=> Beim Initialisieren wird µ als erster Wert des Pixels angenommen.
			sValue[i][1] = sigma; // Std. Value von 25 (empirisch)
		}

		// CALCULATION
		for (int i = 0; i < fileSize; i++) {
			if ((fileBufNew[i] <= (sValue[i][0] - sValue[i][1])) || (fileBufNew[i] >= (sValue[i][0] + sValue[i][1]))) {
				if (DEBUG_PRINT)
					printf("fileBufNew %i ist größer oder kleiner µ +- sigma\n");
				statDif++;
				sValue[i][0] = changeMy(count, fileBufNew[i], sValue[i][0]);
			}
			//fileBufOld[i] = fileBufNew[i];
		}
	}
	else {
		// CALCULATION
		for (int i = 1; i < fileSize; i++) {
			if ((fileBufNew[i] <= (sValue[i][0] - sValue[i][1])) || (fileBufNew[i] >= (sValue[i][0] + sValue[i][1]))) {
				if (DEBUG_PRINT)
					printf("fileBufNew %i ist größer oder kleiner µ +- sigma\n");
				statDif++;
				sValue[i][0] = changeMy(count, fileBufNew[i], sValue[i][0]);
			}
			//fileBufOld[i] = fileBufNew[i];
		}
	}
	//printf("sigma = %i\n", sigma);
	count++;
	return statDif;
}

int changeMy(int count, BYTE fileBufNew, int my) {
	int myDif = 0;
	// Check ob neuer Wert größer/kleiner my
	if (my < fileBufNew)
		myDif = fileBufNew - my;
	else 
		myDif = my - fileBufNew;

	// Dividiere die Differenz durch count um bei einer Änderung nicht gleich total auszuschlagen.
	// 10 als maximum gesetzt um nicht zu unsensibel zu Werten, wenn das Programm länger läuft.
	if (count < 10)
		myDif /= 10;
	else
		myDif /= 10;

	// my neu setzen
	if (my < fileBufNew)
		my += myDif;
	else
		my -= myDif;
	
	return my;
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
	int i = metadata(fileBuf);

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

int metadata(BYTE *fileBuf) {
	int i = 0;
	if (PPM)
		i = metappm(fileBuf, i);
	if (PNM)
		i = metapnm(fileBuf, i);
	return i;
}

int metappm(BYTE *fileBuf, int i) {
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
	// Check und speichern der Breite
	while (fileBuf[i] != ' ') {
		*(pwidth++) = fileBuf[i++];
	}
	*pwidth = '\0';
	i += 1; // Da sonst auf leerzeichen
	// Check und speichern der Höhe
	while (fileBuf[i] != '\n') {
		*(pheight++) = fileBuf[i++];
	}
	*pheight = '\0';
	i += 1; // Da wir schon auf den newline stehen
	// Check und speichern des Farbcodes
	while (fileBuf[i] != '\n') {
		*(pcolor++) = fileBuf[i++];
	}
	*pcolor = '\0';

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

int metapnm(BYTE *fileBuf, int i) {
	// TODO: ERROR Glaube das Bildverzerren kommt weil wir nach der erste if nicht 3 mal hochzählen
	int j = 0; // Zum zählen der newlines (4 in den Metadaten)
	char width[10];
	char height[10];
	char color[10];
	char *pwidth = width;
	char *pheight = height;
	char *pcolor = color;

	if (!(fileBuf[0] == 'P' && fileBuf[1] == '5' && fileBuf[2] == '\n'))
		return 0;
	i = 3;
	// Check auf Kommentare
	if (fileBuf[3] == '#') {
		i = 4;
		while (fileBuf[i] != '\n') {
			i++;
		}
		i += 1;
	}
	// Check und speichern der Breite
	while (fileBuf[i] != ' ') {
		*(pwidth++) = fileBuf[i++];
	}
	*pwidth = '\0';
	i += 1; // Da sonst auf leerzeichen
	// Check und speichern der Höhe
	while (fileBuf[i] != '\n') {
		*(pheight++) = fileBuf[i++];
	}
	*pheight = '\0';
	i += 1; // Da wir schon auf den newline stehen
	// Check und speichern des Farbcodes
	while (fileBuf[i] != ' ') {
		*(pcolor++) = fileBuf[i++];
	}
	*pcolor = '\0';

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

//union
//	byteval {
//	struct rgb_565 {
//		int r : 5;
//		int g : 6;
//		int b : 5;
//	}__attribute__(packed);
//	struct rgb_888 {
//		int r : 8;
//		int g : 8;
//		int b : 8;
//	}__attribute__(packed);
//};

void convert(BYTE *frmBuffer) {
	//BYTE r[] = (BYTE*)malloc(length / 3);
	//BYTE g[] = (BYTE*)malloc(length / 3);
	//BYTE b[] = (BYTE*)malloc(length / 3);

	//for (int i = 0; i < length; i++) {
	//	r[i] = ((((frmBuffer[i] >> 11) & 0x1F) * 527) + 23) >> 6;
	//	g[i] = ((((frmBuffer[i] >> 5) & 0x3F) * 259) + 33) >> 6;
	//	b[i] = ((((frmBuffer[i]) & 0x1F) * 527) + 23) >> 6;
	//}

	//for (int i = 0; i < length; i++) {
	//	frmbuffer[i] = r[i] << 16 | g[i] << 8 | b;
	//}


	/*BYTE r, g, b;
	for (int i = 0; i < length; i++) {
		r = ((((frmBuffer[i] >> 11) & 0x1F) * 527) + 23) >> 6;
		g = ((((frmBuffer[i] >> 5) & 0x3F) * 259) + 33) >> 6;
		b = ((((frmBuffer[i]) & 0x1F) * 527) + 23) >> 6;

		frmBuffer[i] = r << 16 | g << 8 | b;
	}*/

	int i = 0;
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			uint16_t rgb = (frmBuffer[(y * WIDTH * 2) + x * 2 + 1] << 8)
				| frmBuffer[(y * WIDTH * 2) + x * 2];
			uint8_t r = (rgb & 0xF800) >> 8;
			uint8_t g = (rgb & 0x07E0) >> 3;
			uint8_t b = (rgb & 0x001F) << 3;
			frmBuffer[i++] = (r << 11) | (g << 5) | (b);
			frmBuffer[i++] = (r << 11) | (g << 5) | (b);
		}
	}
}