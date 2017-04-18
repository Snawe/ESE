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

	17.04.
	2h Umsetzen der statistischen Verfahren + Fehler testen + herausfinden wie genau das Verfahren funktioniert und warum die Bilder nicht richtig ausschauen
	1h cleanup code nachdem die ausgabe Bilder eindeutig falsch sind und man dadurch vllt auf fehler drauf kommt
	2h Warnings fixen + Error's fixen die durch das aufräumen zu Tageslicht gekommen sind 

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
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// An unsigned char can store 1 Bytes (8bits) of data (0-255)
typedef struct stats {
	int N;
	int average;
	int variance;
	double stddev;
	int old;
} statistic;

struct metadata {
	int width;
	int height;
	int color;
};

typedef unsigned char BYTE; 
long getFileSize(FILE *file);
int metadata(FILE *fp);
int rollingStatistic(int val, statistic *stat);

const char *filePath[20] = {
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\11.pnm",
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" ,
	"testM\\1.pnm" };
	//"testM\\2.pnm" ,
	//"testM\\3.pnm" ,
	//"testM\\4.pnm" ,
	//"testM\\5.pnm" ,
	//"testM\\6.pnm" ,
	//"testM\\7.pnm" ,
	//"testM\\8.pnm" ,
	//"testM\\9.pnm" ,
	//"testM\\10.pnm" ,
	//"testM\\11.pnm" ,
	//"testM\\12.pnm" ,
	//"testM\\13.pnm",
	//"testM\\14.pnm",
	//"testM\\15.pnm" };

const char *filePathChange[20] = {
	"testM\\_01.pnm" ,
	"testM\\_02.pnm" ,
	"testM\\_03.pnm" ,
	"testM\\_04.pnm" ,
	"testM\\_05.pnm" ,
	"testM\\_06.pnm" ,
	"testM\\_07.pnm" ,
	"testM\\_08.pnm" ,
	"testM\\_09.pnm" ,
	"testM\\_10.pnm" ,
	"testM\\_11.pnm" ,
	"testM\\_12.pnm" ,
	"testM\\_13.pnm",
	"testM\\_14.pnm",
	"testM\\_16.pnm",
	"testM\\_17.pnm" ,
	"testM\\_18.pnm" ,
	"testM\\_19.pnm" ,
	"testM\\_20.pnm" ,
	"testM\\_21.pnm" };

statistic stat[64000];

int main()
{
	BYTE fileBufNew;		// Pointer to our buffered data
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

	// Get the size of the file in bytes
	long fileSizeNew = 0;
	int metad = 0;

	for (int i = 0; i < 64000; i++) {
		stat[i].N = 5;
		stat[i].average = 128;
		stat[i].variance = 200;
		stat[i].stddev = sqrt(stat[i].variance);
		stat[i].old = 128;
	}

	for (int j = 0; j < (sizeof(filePath)/sizeof(filePath[0])); j++) {
		fpNew = fopen(filePath[j], "rb");
		fpDif = fopen(filePathChange[j], "wb");
		metad = metadata(fpNew);
		fileSizeNew = getFileSize(fpNew);
		rewind(fpNew);
		for (int i = 0; i < metad; i++) {
			fread(&fileBufNew, 1, 1, fpNew);
			fwrite(&fileBufNew, 1, 1, fpDif);
		}
		printf("%s\n", filePath[j]);
		for (int i = metad; i < fileSizeNew; i++) {
			fread(&fileBufNew, 1, 1, fpNew);
			rollingStatistic(fileBufNew, &(stat[i]));
			
			if (fileBufNew > (stat[i].average + stat[i].stddev * 3))
				fileBufNew = 0xFF;
			else if (fileBufNew < (stat[i].average - stat[i].stddev * 3))
				fileBufNew = 0xFF;
			else
				fileBufNew = 0x00;

			//fileBufNew = (BYTE)stat[i].average;
			int c = fwrite(&fileBufNew, 1, 1, fpDif);
			if(c != 1)
				printf("%d", c);			
		}		
		fclose(fpDif);
		fclose(fpNew);
	}	
	if (DEBUG_WAIT) {
		char wait = '0';
		while (wait == '0') {
			scanf("%c", &wait);
		}
	}
	fclose(fpNew);
	fclose(fpDif);

	return 0;
}

int rollingStatistic(int val, statistic *stat) {
	int oldavg = stat->average;
	int newavg = (oldavg - oldavg / stat->N) + (val / stat->N);
	stat->average = newavg;
	//( NEU - ATL ) * ( NEU - AVG + ALT - AltAVT
	//int newvar = (abs(val - statistic[i].old))*(abs(val - newavg + statistic[i].old - oldavg)) / (statistic[i].N - 1);
	// int newvar = (val - stat->old)*(val - newavg + stat->old - oldavg) / (stat->N - 1);
	int newvar = stat->N * pow(newavg - oldavg, 2);
	if ((stat->variance + newvar) < 0)
		newvar = stat->variance;
	stat->variance = newvar;
	//print("V:" + str(self.variance))
	/*printf("variance = %i\n", statistic.variance);*/
	stat->stddev = sqrt(stat->variance);
	stat->old = val;

	//int oldavg = statistic[i].average;
	//int newavg = oldavg + (val - statistic[i].old) / statistic[i].N;
	//statistic[i].average = newavg;
	//statistic[i].variance += (val - statistic[i].old)*(val - newavg + statistic[i].old - oldavg) / (statistic[i].N - 1);
	//statistic[i].stddev = sqrt(statistic[i].variance);
	//statistic[i].old = val;

	//return statistic;
	//count++;

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

int metadata(FILE *fp) {
	struct metadata metad;
	// TODO: ERROR Glaube das Bildverzerren kommt weil wir nach der erste if nicht 3 mal hochzählen

	char magic[16];
	char width[16];
	char height[16];
	char color[16];

	char buffer[16];
	char *pBuffer = buffer;

	char *pState[4] = { magic, width, height, color };
	int state = 0;

	while ((fread(pBuffer, 1, 1, fp) != 0)) {
		//comments
		if (pBuffer == buffer && *pBuffer == '#') {
			while (fread(buffer, 1, 1, fp) == 1) 
				if (buffer[0] == '\n')
					break;
			
			pBuffer = buffer;
			continue;
		}

		if (*pBuffer == '\n' || *pBuffer == ' ') {
			*pBuffer = '\0';

			char *pActualState = pState[state];
			for (int i = 0; i < sizeof(buffer) - 1; i++) {
				pActualState[i] = buffer[i];
			}

			state++;
			if (state >= 4) break;

			pBuffer = buffer;
			continue;
		}

		pBuffer++;
	}

	metad.color = strtol(color, NULL, 10);
	metad.height = strtol(height, NULL, 10);
	metad.width = strtol(width, NULL, 10);

	return ftell(fp);
}