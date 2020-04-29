/*
===========================================================
Program that applies a XOR operation to a binary file 
so it can be sent safely through the internet without 
getting blocked by antivirus software. 
The original file can be then 
reinstated. 
@author: Velorek
@version: 0.1
@last modified: MAY 2020
===========================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
//ANSI CODES
# define AYEL "\x1b[1;33m"	//Yellow
# define AWHT "\x1b[1;37m"	//White
# define ARST "\x1b[0m"		//Reset

#elif _WIN32
# define AYEL ""
# define AWHT ""
# define ARST ""
#else
# define AYEL "\x1b[1;33m"	//Yellow
# define AWHT "\x1b[1;37m"	//White
# define ARST "\x1b[0m"		//Reset
#endif

//CONSTANTS
#define XOR_KEY 0x11110011

//HELP MESSAGES

#define HELPMSG  "[+] " AWHT "x0r" ARST " your binary files\n"\
  AYEL "ENCODE/DECODE:" ARST " xor" " [input_file] " "[output_file]\n"

//INFO MESSAGES
#define NFO_MSG1_EN AYEL "Success!! "ARST "Binary file [%s] with %ld bytes.\nNew file [%s] written with %ld bytes.\n"
#define NFO_MSG2_EN AYEL "Success!! "ARST "Encoded File [%s] with %ld bytes.\nNew file [%s] written with %ld bytes.\n"

//ERROR MESSAGES
#define ERR_MSG1_EN "Error opening file.\n"
#define ERR_MSG2_EN "Error: Destination file must be different from source file.\n"
#define ERR_MSG3_EN "Incorrect mode.\n"

//CONSTANT METHODS DECLARATIONS
#define displayHelp() printf(HELPMSG);

/* FUNCTION DECLARATIONS */

void    processOptions(char *sourceFileStr,
		       char *destinationFileStr);
long fileSize(FILE * fileHandler);
int     openFile(FILE ** fileHandler, char *fileName, char *mode);
int     closeFile(FILE * fileHandler);
long    encodeDecodeFile(FILE * fileHandler, FILE * fileHandler2);

int main(int argc, char *argv[]) {
  int     processMode;

  if(argc == 3) {
    //Number of arguments is ok!
    if(strcmp(argv[1], argv[2]) == 0) {
      //Error : same name for source and destination files
      fprintf(stderr, ERR_MSG2_EN);
    } else {
      //Open files? 
      processOptions(argv[1], argv[2]);
    }
  } else {
    //Error in no. of arguments. Display help and exit.
    displayHelp();
  }
  return 0;
}

void processOptions(char *sourceFileStr,
		    char *destinationFileStr) {
  long    newFileSize;
  int     okFile, okFile2;
  FILE   *fileSource, *fileDestination;

      okFile = openFile(&fileSource, sourceFileStr, "rb");	//read only
      okFile2 = openFile(&fileDestination, destinationFileStr, "w");	//create destination file   
      if(okFile == 1 && okFile2 == 1) {
	//Success!
	newFileSize = encodeDecodeFile(fileSource, fileDestination);
	printf(NFO_MSG1_EN, sourceFileStr, fileSize(fileSource), destinationFileStr, newFileSize);	//Info.
	closeFile(fileSource);
	closeFile(fileDestination);
      } else {
	//Error opening files.
	fprintf(stderr, ERR_MSG1_EN);
      }
}
long fileSize(FILE * fileHandler) {
  long    tempSize;
  //We put the pointer at the end and 
  //query position and return it.
  if(fileHandler != NULL) {
    fseek(fileHandler, 0, SEEK_END);
    tempSize = ftell(fileHandler);
  }
  return tempSize;
}

int closeFile(FILE * fileHandler) {
  int     ok;
  ok = fclose(fileHandler);
  return ok;
}

int openFile(FILE ** fileHandler, char *fileName, char *mode) {
  int     ok;
  *fileHandler = fopen(fileName, mode);
  //check whether buffer is assigned
  //and return value
  if(*fileHandler != NULL)
    ok = 1;
  else
    ok = 0;
  return ok;
}

long encodeDecodeFile(FILE * fileHandler, FILE * fileHandler2) {
  long    byteCount = 0;
  char    ch,xorch;

  //Read char by char
  if(fileHandler != NULL && fileHandler2 != NULL) {
    rewind(fileHandler);	//Go to start of file
    fread(&ch, sizeof(ch), 1, fileHandler);	// Peek into file
    while(!feof(fileHandler)) {
      xorch = ch ^ XOR_KEY;
      byteCount += fprintf(fileHandler2, "%c", xorch);
      fread(&ch, sizeof(ch), 1, fileHandler);
    }
  }
  return byteCount;
}

