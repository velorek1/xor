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
#define ENCODE 1
#define DECODE 0

//HELP MESSAGES

#define HELPMSG  "[+] " AWHT "x0r" ARST " your binary files\n"\
  AYEL "ENCODE:" ARST " xor " AYEL "1" ARST" [input_file] [output_file]\n"\
AYEL "DECODE:" ARST " xor " AYEL "0" ARST" [input_file] [output_file]\n"

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

void    processOptions(int processMode, char *sourceFileStr,
		       char *destinationFileStr);
long    fileSize(FILE * fileHandler);
int     openFile(FILE ** fileHandler, char *fileName, char *mode);
int     closeFile(FILE * fileHandler);
long    encodeFile(FILE * fileHandler, FILE * fileHandler2);
long    decodeFile(FILE * fileHandler, FILE * fileHandler2);

int main(int argc, char *argv[]) {
  int     processMode;

  if(argc == 4) {
    //Number of arguments is ok!
    if(strcmp(argv[2], argv[3]) == 0) {
      //Error : same name for source and destination files
      fprintf(stderr, ERR_MSG2_EN);
    } else {
      //Open files? Encode or Decode?
      processMode = atoi(argv[1]);	//Argument one to int: Encode 1 or Decode 0?
      processOptions(processMode, argv[2], argv[3]);
    }
  } else {
    //Error in no. of arguments. Display help and exit.
    displayHelp();
  }
  return 0;
}

void processOptions(int processMode, char *sourceFileStr,
		    char *destinationFileStr) {
  long    newFileSize;
  int     okFile, okFile2;
  FILE   *fileSource, *fileDestination;

  switch (processMode) {
    case ENCODE:		//Encode
      okFile = openFile(&fileSource, sourceFileStr, "rb");	//read only
      okFile2 = openFile(&fileDestination, destinationFileStr, "w");	//create destination file   
      if(okFile == 1 && okFile2 == 1) {
	//Success!
	newFileSize = encodeFile(fileSource, fileDestination);
	printf(NFO_MSG1_EN, sourceFileStr, fileSize(fileSource), destinationFileStr, newFileSize);	//Info.
	closeFile(fileSource);
	closeFile(fileDestination);
      } else {
	//Error opening files.
	fprintf(stderr, ERR_MSG1_EN);
      }
      break;
    case DECODE:		//Decode
      okFile = openFile(&fileSource, sourceFileStr, "r");	//read only
      okFile2 = openFile(&fileDestination, destinationFileStr, "wb");	//create destination file   
      if(okFile == 1 && okFile2 == 1) {
	//Success!
	newFileSize = decodeFile(fileSource, fileDestination);
	printf(NFO_MSG2_EN, sourceFileStr, fileSize(fileSource), destinationFileStr, newFileSize);	//Info.
	closeFile(fileSource);
	closeFile(fileDestination);
      } else {
	//Error opening files.
	fprintf(stderr, ERR_MSG1_EN);
      }
      break;
    default:			//Incorrect option              
      fprintf(stderr, ERR_MSG3_EN);
      break;
  }
  return;
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

long encodeFile(FILE * fileHandler, FILE * fileHandler2) {
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

long decodeFile(FILE * fileHandler, FILE * fileHandler2) {
  long    i = 0;
  long    byteCount = 0;
  char    ch;
  char    xorch;

  //Read char by char
  if(fileHandler != NULL && fileHandler2 != NULL) {
    rewind(fileHandler);	//Go to start of file
    ch = getc(fileHandler);	//peek into file
    while(!feof(fileHandler)) {
      //Read until SEPARATOR '.'
      xorch = ch ^ XOR_KEY;
      i++;
      fwrite(&xorch, sizeof(xorch), 1, fileHandler2);	// Write data to file
      byteCount++;
      ch = getc(fileHandler);
    }
  }

  return byteCount;
}
