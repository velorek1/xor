// JUST FOR FUN XOR EXPANDED FOR LINUX

#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <dirent.h>

#define MAX_TEXTBOX 255
#define K_BACKSPACE 127
#define K_ENTER 13
#define K_ENTER2 10
#define K_TAB 9
#define K_UP_TRAIL "\e[A\0\0"
#define K_DOWN_TRAIL "\e[B\0\0"

#define text0 "___  ___ ________  ______\n"  
#define text1 "|  |_|  ||       ||    _ |\n"  
#define text2 "|       ||   _   ||   | ||\n"  
#define text3 "|       ||  | |  ||   |_||\n"  
#define text4 " |     | |  |_|  ||    __  |\n"
#define text5 "|   _   ||       ||   |  | |\n"
#define text6 "|__| |__||_______||___|  |_|\n"

#define AYEL "\x1b[1;33m"	//Yellow
#define AWHT "\x1b[1;37m"	//White
#define ARST "\x1b[0m"		//Reset

//CONSTANTS
#define XOR_KEY 0x11110011


//INFO MESSAGES
#define NFO_MSG1_EN AYEL "\rSuccess!! "ARST "Binary file [%s] with %ld bytes.\n\rNew file [%s] written with %ld bytes.\n\r"
#define NFO_MSG2_EN AYEL "\rSuccess!! "ARST "Encoded File [%s] with %ld bytes.\n\rNew file [%s] written with %ld bytes.\n\r"

//ERROR MESSAGES
#define ERR_MSG1_EN "\rError opening file.\n"
#define ERR_MSG2_EN "\rError: Destination file must be different from source file.\n"
#define ERR_MSG3_EN "\rIncorrect mode.\n"
//Scroll Control values.
#define SCROLL_ACTIVE 1
#define SCROLL_INACTIVE 0
#define CONTINUE_SCROLL -1
#define DOWN_SCROLL 1
#define UP_SCROLL 0
#define SELECT_ITEM 1
#define UNSELECT_ITEM 0
#define CIRCULAR_ACTIVE 1
#define CIRCULAR_INACTIVE 0

// Colors used.                                                                         
#define B_BLACK 40
#define B_WHITE 47
#define B_BLUE 44
#define F_BLACK 30
#define F_WHITE 37
#define F_BLUE 34
#define FH_WHITE 97
#define FILL_CHAR ' '
//Keys used.
//#define K_ENTER 10
#define K_ESCAPE 27
#define K_UP_ARROW 'A'		// K_ESCAPE + 'A' -> UP_ARROW
#define K_DOWN_ARROW 'B'	// K_ESCAPE + 'B' -> DOWN_ARROW
//Directories
#define CURRENTDIR "."
#define CHANGEDIR ".."
#define MAX_ITEM_LENGTH 15
#define DIRECTORY 1
#define FILEITEM 0
#define MAX 1024

/*====================================================================*/
/* TYPEDEF STRUCTS DEFINITIONS */
/*====================================================================*/
typedef struct _listchoice {
  unsigned index;		// Item number
  char   *item;			// Item string
  char   *path;			// Item path
  unsigned isDirectory;		// Kind of item
  struct _listchoice *next;	// Pointer to next item
  struct _listchoice *back;	// Pointer to previous item
} LISTCHOICE;

typedef struct _scrolldata {
  unsigned scrollActive;	//To know whether scroll is active or not.
  unsigned scrollLimit;		//Last index for scroll.
  unsigned listLength;		//Total no. of items in the list
  unsigned currentListIndex;	//Pointer to new sublist of items when scrolling.
  unsigned displayLimit;	//No. of elements to be displayed.
  unsigned scrollDirection;	//To keep track of scrolling Direction.
  unsigned selector;		//Y++
  unsigned wherex;		
  unsigned wherey;		
  unsigned backColor0;		//0 unselected; 1 selected
  unsigned foreColor0;
  unsigned backColor1;
  unsigned foreColor1;
  unsigned isDirectory;		// Kind of item
  char   *item;
  char   *path;
  unsigned itemIndex;
} SCROLLDATA;

               

struct winsize max;
static struct termios newp, failsafe;
static int peek_character = -1;
int rows,columns;
LISTCHOICE *listBox1 = NULL;	//Head pointer.
char filex[MAX_TEXTBOX];

  SCROLLDATA scrollData;
//Terminal
int kbhit();
int read_keytrail(char chartrail[5]);
int readch();
void resetch();
void gotoxy(int x, int y);
void outputcolor(int foreground, int background);
int get_terminal_dimensions(int *rows, int *columns);
int get_pos(int *y, int *x);
void hidecursor();
void showcursor();
void resetAnsi(int x);
void pushTerm();
int resetTerm();

//Textbox
void write_str(int wherex, int wherey, char *str, int backcolor, int forecolor);
int textbox(int wherex, int wherey, int displayLength,char label[MAX_TEXTBOX], char text[MAX_TEXTBOX], int backcolor,
	    int labelcolor, int textcolor);
void write_ch(int wherex, int wherey, char ch, int backcolor, int forecolor);



void write_str(int wherex, int wherey, char *str, int backcolor, int forecolor){
  gotoxy(wherex,wherey);
  outputcolor(backcolor,forecolor);
  printf("%s", str);
}

void write_ch(int wherex, int wherey, char ch, int backcolor, int forecolor){
  gotoxy(wherex,wherey);
  outputcolor(backcolor,forecolor);
  printf("%c", ch);
}
//File

void    processOptions(char *sourceFileStr,
		       char *destinationFileStr);
long    fileSize(FILE * fileHandler);
int     openFile(FILE ** fileHandler, char *fileName, char *mode);
int     closeFile(FILE * fileHandler);
long    encodeDecodeFile(FILE * fileHandler, FILE * fileHandler2);   

// LISTBOX & LISTFILEs
//CONSOLE DISPLAY FUNCTIONS 
void    cleanLine(int line, int backcolor, int forecolor);
void    outputcolor(int foreground, int background);
//char    getch();
void    draw_window(int x1, int y1, int x2, int y2, int backcolor);

//DYNAMIC LINKED LIST FUNCTIONS
void    deleteList(LISTCHOICE ** head);
LISTCHOICE *addend(LISTCHOICE * head, LISTCHOICE * newp);
LISTCHOICE *newelement(char *text, char *itemPath, unsigned itemType);

//LISTBOX FUNCTIONS
char    listBox(LISTCHOICE * selector, unsigned whereX, unsigned whereY,
		SCROLLDATA * scrollData, unsigned bColor0,
		unsigned fColor0, unsigned bColor1, unsigned fColor1,
		unsigned displayLimit);
void    loadlist(LISTCHOICE * head, SCROLLDATA * scrollData,
		 unsigned indexAt);

void    gotoIndex(LISTCHOICE ** aux, SCROLLDATA * scrollData,
		  unsigned indexAt);
int     query_length(LISTCHOICE ** head);
int     move_selector(LISTCHOICE ** head, SCROLLDATA * scrollData);
char    selectorMenu(LISTCHOICE * aux, SCROLLDATA * scrollData);
void    displayItem(LISTCHOICE * aux, SCROLLDATA * scrollData, int select);

//LISTFILES FUNCTIONS
int     listFiles(LISTCHOICE ** listBox1, char *directory);
int     addSpaces(char temp[MAX_ITEM_LENGTH]);
void    cleanString(char *string, int max);
void    changeDir(SCROLLDATA * scrollData, char fullPath[MAX],
		  char newDir[MAX]);

int opfiledialog(int wherey, char filex[MAX_TEXTBOX]);

//KEYBOARD
/*----------------------------------*/
/* Read ESC-key char with its trail */
/*----------------------------------*/

int read_keytrail(char chartrail[5]){
/* 
   New implementation: Trail of chars found in keyboard.c
   If K_ESCAPE is captured read a trail up to 5 characters from the console.
   This is to control the fact that some keys may change
   according to the terminal and expand the editor's possibilities.
   Eg: F2 can be either 27 79 81 or 27 91 91 82.  
*/
char ch;
int i;
   chartrail[0] = K_ESCAPE;
   for(i = 1; i < 5; i++) {
     if(kbhit() == 1) {
        ch=readch();
        chartrail[i] = ch;
     } else {
        chartrail[i] = 0;
     }
   }
   resetch();		    
   return 1;
}


//LISTBOX

//draw window area with shadow

void draw_window(int x1, int y1, int x2, int y2, int backcolor) {
  int     i, j;
  i = x1;
  j = y1;

  //window
  for(j = y1; j <= y2; j++)
    for(i = x1; i <= x2; i++) {
      gotoxy(i, j);
      outputcolor(F_WHITE, backcolor);
      printf("%c", FILL_CHAR);
    }
}

void cleanLine(int line, int backcolor, int forecolor) {
//Cleans line of console.
  int     i;
  for(i = 0; i < 80; i++) {
    //clean line where path is displayed.
    outputcolor(forecolor, backcolor);
    gotoxy(i, line);
    printf("%c", FILL_CHAR);	//space
  }
}

/* --------------------- */
/* Dynamic List routines */
/* --------------------- */

// create new list element of type LISTCHOICE from the supplied text string
LISTCHOICE *newelement(char *text, char *itemPath, unsigned itemType) {
  LISTCHOICE *newp;
  newp = (LISTCHOICE *) malloc(sizeof(LISTCHOICE));
  newp->item = (char *)malloc(strlen(text) + 1);
  newp->path = (char *)malloc(strlen(itemPath) + 1);
  strcpy(newp->item, text);
  strcpy(newp->path, itemPath);
  newp->isDirectory = itemType;
  newp->next = NULL;
  newp->back = NULL;
  return newp;
}

// deleleteList: remove list from memory
/* Function to delete the entire linked list */
void deleteList(LISTCHOICE **head) 
{ 
   /* deref head_ref to get the real head */
   LISTCHOICE *current = *head; 
   LISTCHOICE *next = NULL; 
  
   while (current != NULL)  
   { 
       next = current->next; 
       free(current->item);
       free(current->path);
       free(current);
       current = next; 
   } 
    
   /* deref head_ref to affect the real head back 
      in the caller. */
   *head = NULL; 
} 

/* addend: add new LISTCHOICE to the end of a list  */
/* usage example: listBox1 = (addend(listBox1, newelement("Item")); */
LISTCHOICE *addend(LISTCHOICE * head, LISTCHOICE * newp) {
  LISTCHOICE *p2;
  if(head == NULL) {
    newp->index = 0;
    newp->back = NULL;
    return newp;
  }
// now find the end of list
  for(p2 = head; p2->next != NULL; p2 = p2->next) ;
  p2->next = newp;
  newp->back = p2;
  newp->index = newp->back->index + 1;
  return head;
}

/* ---------------- */
/* Listbox routines */
/* ---------------- */

void gotoIndex(LISTCHOICE ** aux, SCROLLDATA * scrollData,
	       unsigned indexAt)
//Go to a specific location on the list.
{
  LISTCHOICE *aux2;
  unsigned counter = 0;
  *aux = listBox1;
  aux2 = *aux;
  while(counter != indexAt) {
    aux2 = aux2->next;
    counter++;
  }
  //Highlight current item

  displayItem(aux2, scrollData, SELECT_ITEM);

  //Update pointer
  *aux = aux2;
}

void loadlist(LISTCHOICE * head, SCROLLDATA * scrollData, unsigned indexAt) {
/*
Displays the items contained in the list with the properties specified
in scrollData.
*/

  LISTCHOICE *aux;
  unsigned wherey, counter = 0;

  aux = head;
  gotoIndex(&aux, scrollData, indexAt);
  /* Save values */
  //wherex = scrollData->wherex;
  wherey = scrollData->wherey;
  do {
    displayItem(aux, scrollData, UNSELECT_ITEM);
    aux = aux->next;
    counter++;
    scrollData->selector++;	// wherey++
  } while(counter != scrollData->displayLimit);
  scrollData->selector = wherey;	//restore value
}

int query_length(LISTCHOICE ** head) {
//Return no. items in a list.
  {
    LISTCHOICE *aux;

    unsigned itemCount = 0;
    aux = *head;
    while(aux->next != NULL) {
      aux = aux->next;
      itemCount++;
    }
    return itemCount;
  }

}

void displayItem(LISTCHOICE * aux, SCROLLDATA * scrollData, int select)
//Select or unselect item animation
{
  switch (select) {

    case SELECT_ITEM:
      gotoxy(scrollData->wherex, scrollData->selector);
      outputcolor(scrollData->foreColor1, scrollData->backColor1);
      printf("%s\n", aux->item);
      break;

    case UNSELECT_ITEM:
      gotoxy(scrollData->wherex, scrollData->selector);
      outputcolor(scrollData->foreColor0, scrollData->backColor0);
      printf("%s\n", aux->item);
      break;
  }
}
int move_selector(LISTCHOICE ** selector, SCROLLDATA * scrollData) {
/* 
Creates animation by moving a selector highlighting next item and
unselecting previous item
*/

  LISTCHOICE *aux;
  unsigned scrollControl = 0, continueScroll = 0, circular =
      CIRCULAR_INACTIVE;
  //Auxiliary pointer points to selector.
  aux = *selector;

  //Circular list animation when not scrolling.
  if(aux->index == scrollData->listLength - 1
     && scrollData->scrollActive == SCROLL_INACTIVE
     && scrollData->scrollDirection == DOWN_SCROLL) {
    //After last item go back to the top.
    displayItem(aux, scrollData, UNSELECT_ITEM);
    scrollData->selector = scrollData->wherey;
    gotoIndex(&aux, scrollData, 0);
    *selector = aux;
    circular = CIRCULAR_ACTIVE;
  }

  if(aux->index == 0 && scrollData->scrollActive == SCROLL_INACTIVE
     && scrollData->scrollDirection == UP_SCROLL) {
    //Before first item go back to the bottom.
    displayItem(aux, scrollData, UNSELECT_ITEM);
    scrollData->selector = scrollData->wherey + scrollData->listLength - 1;
    gotoIndex(&aux, scrollData, scrollData->listLength - 1);
    *selector = aux;
    circular = CIRCULAR_ACTIVE;
  }
  //Check if we do the circular list animation.
  //If active, we skip the code one time.

  if(circular == CIRCULAR_INACTIVE) {

    //Check if we are within boundaries.
    if((aux->next != NULL && scrollData->scrollDirection == DOWN_SCROLL)
       || (aux->back != NULL && scrollData->scrollDirection == UP_SCROLL)) {

      //Unselect previous item
      displayItem(aux, scrollData, UNSELECT_ITEM);

      //Check whether we move UP or Down
      switch (scrollData->scrollDirection) {

	case UP_SCROLL:
	  //Calculate new top index if scroll is active 
	  //otherwise it defaults to 0 (top)
	  if(scrollData->scrollActive == SCROLL_ACTIVE)
	    scrollControl = scrollData->currentListIndex;
	  else
	    scrollControl = 0;

	  //Move selector
	  if(aux->back->index >= scrollControl) {
	    scrollData->selector--;	//whereY--
	    aux = aux->back;	//Go to previous item
	  } else {
	    if(scrollData->scrollActive == SCROLL_ACTIVE)
	      continueScroll = 1;
	    else
	      continueScroll = 0;
	  }
	  break;

	case DOWN_SCROLL:
	  //Calculate bottom index limit if scroll is ACTIVE
	  //Otherwise it defaults to scrollData->ListLength-1

	  if(scrollData->scrollActive == SCROLL_ACTIVE)
	    scrollControl =
		scrollData->currentListIndex + (scrollData->displayLimit -
						1);
	  else
	    scrollControl = scrollData->listLength - 1;

	  //Move selector
	  if(aux->next->index <= scrollControl) {
	    aux = aux->next;	//Go to next item
	    scrollData->selector++;	//whereY++;
	  } else {
	    if(scrollData->scrollActive == SCROLL_ACTIVE)
	      continueScroll = 1;
	    else
	      continueScroll = 0;
	  }
	  break;
      }


      //Highlight new item
      displayItem(aux, scrollData, SELECT_ITEM);

      //Update selector pointer
      *selector = aux;
    }
  }
  circular = CIRCULAR_INACTIVE;
  return continueScroll;
}

char selectorMenu(LISTCHOICE * aux, SCROLLDATA * scrollData) {
  char    ch;
  unsigned control = 0;
  unsigned continueScroll;
  unsigned counter = 0;
  unsigned keypressed = 0;
  char chartrail[5];
  //Go to and select expected item at the beginning

  gotoIndex(&aux, scrollData, scrollData->currentListIndex);
  //Metrics
  //cleanLine(window_y1 + 1, MENU_PANEL, MENU_FOREGROUND0, window_x1 + 1, window_x2);
  //outputcolor(MENU_FOREGROUND0, MENU_PANEL);
  //gotoxy(window_x1 + 2, window_y1 + 1);
  //printf("Open File: w/s ^/v");
 // gotoxy(window_x1 + 3, window_y2 - 1);
  //outputcolor(MENU_FOREGROUND0, B_CYAN);
 // if (aux->index != 0 && aux->index != 1) printf("    [%d/%d]    ", aux->index-1, scrollData->listLength - 2);
  //else printf("               ");

  if(scrollData->scrollDirection == DOWN_SCROLL
     && scrollData->currentListIndex != 0) {
    //If we are going down we'll select the last item 
    //to create a better scrolling transition (animation)
    for(counter = 0; counter < scrollData->displayLimit; counter++) {
      scrollData->scrollDirection = DOWN_SCROLL;
      move_selector(&aux, scrollData);
    }
  } else {
    //Do nothing if we are going up. Selector is always at the top item.
  }

  //It break the loop everytime the boundaries are reached.
  //to reload a new list to show the scroll animation.
  while(control != CONTINUE_SCROLL) {
    keypressed = kbhit();
   if (keypressed == 1){
      ch = readch();
      keypressed = 0;
      //if enter key pressed - break loop
      if(ch == K_ENTER)
        control = CONTINUE_SCROLL;	//Break the loop
      //fail-safe keys
      if (ch == 'w'){
	  //Move selector up
	  scrollData->scrollDirection = UP_SCROLL;
	  continueScroll = move_selector(&aux, scrollData);
	  //Break the loop if we are scrolling
	  if(scrollData->scrollActive == SCROLL_ACTIVE
	     && continueScroll == 1) {
	    control = CONTINUE_SCROLL;
	    //Update data
	    scrollData->currentListIndex =
		scrollData->currentListIndex - 1;
	    scrollData->selector = scrollData->wherey;
	    scrollData->itemIndex = aux->index;
	    //Return value
	    ch = control;
	  }
       }
      if (ch == 's'){
	  //Move selector down
	  scrollData->scrollDirection = DOWN_SCROLL;
	  continueScroll = move_selector(&aux, scrollData);
	  //Break the loop if we are scrolling
	  if(scrollData->scrollActive == SCROLL_ACTIVE
	     && continueScroll == 1) {
	    control = CONTINUE_SCROLL;
	    //Update data  
	    scrollData->currentListIndex =
		scrollData->currentListIndex + 1;
	    scrollData->selector = scrollData->wherey;
	    scrollData->itemIndex = aux->index;
	    scrollData->scrollDirection = DOWN_SCROLL;
	  }
	  //Return value  
          ch = control;
       }
      //Check arrow keys
      if(ch == K_ESCAPE)		// escape key
      {
        read_keytrail(chartrail);	// read key again for arrow key combinations
        if(strcmp(chartrail, K_UP_TRAIL) == 0) {
	  // escape key + A => arrow key up
	  //Move selector up
	  scrollData->scrollDirection = UP_SCROLL;
	  continueScroll = move_selector(&aux, scrollData);
	  //Break the loop if we are scrolling
	  if(scrollData->scrollActive == SCROLL_ACTIVE
	     && continueScroll == 1) {
	    control = CONTINUE_SCROLL;
	    //Update data
	    scrollData->currentListIndex =
		scrollData->currentListIndex - 1;
	    scrollData->selector = scrollData->wherey;
	    scrollData->itemIndex = aux->index;
	    //Return value
	    ch = control;
	  }
       }
      if(strcmp(chartrail, K_DOWN_TRAIL) == 0) {
	// escape key + B => arrow key down
	  //Move selector down
	  scrollData->scrollDirection = DOWN_SCROLL;
	  continueScroll = move_selector(&aux, scrollData);
	  //Break the loop if we are scrolling
	  if(scrollData->scrollActive == SCROLL_ACTIVE
	     && continueScroll == 1) {
	    control = CONTINUE_SCROLL;
	    //Update data  
	    scrollData->currentListIndex =
		scrollData->currentListIndex + 1;
	    scrollData->selector = scrollData->wherey;
	    scrollData->itemIndex = aux->index;
	    scrollData->scrollDirection = DOWN_SCROLL;
	  }
	  //Return value  
          ch = control;         
      }
      }
 // if (ch == K_TAB) break;
  if(ch == K_ENTER || ch == K_ENTER2)		// enter key
  {
    //Pass data of last item selected.
    //scrollData->item =
	//(char *)imalloc(sizeof(char) * strlen(aux->item) + 1);
    //scrollData->path = (char *)malloc(sizeof(char) *strlen(aux->path) + 1);
    scrollData->item = aux->item;
    scrollData->itemIndex = aux->index;

    //strcpy(scrollData->path, aux->path);
    scrollData->path = aux->path;
    scrollData->isDirectory = aux->isDirectory;
  }
  }
  }
  return ch;
}
char listBox(LISTCHOICE * head,
	     unsigned whereX, unsigned whereY,
	     SCROLLDATA * scrollData, unsigned bColor0,
	     unsigned fColor0, unsigned bColor1, unsigned fColor1,
	     unsigned displayLimit) {

  unsigned list_length = 0;
  //unsigned currentIndex = 0;
  int     scrollLimit = 0;
  unsigned currentListIndex = 0;
  char    ch=0;
  LISTCHOICE *aux=NULL;

  // Query size of the list
  list_length = query_length(&head) + 1;

  //Save calculations for SCROLL and store DATA
  scrollData->displayLimit = displayLimit;
  scrollLimit = list_length - scrollData->displayLimit;	//Careful with negative integers

  if(scrollLimit < 0)
    scrollData->displayLimit = list_length;	//Failsafe for overboard values

  scrollData->scrollLimit = scrollLimit;
  scrollData->listLength = list_length;
  scrollData->wherex = whereX;
  scrollData->wherey = whereY;
  scrollData->selector = whereY;
  scrollData->backColor0 = bColor0;
  scrollData->backColor1 = bColor1;
  scrollData->foreColor0 = fColor0;
  scrollData->foreColor1 = fColor1;

  //Check whether we have to activate scroll or not 
  //and if we are within bounds. [1,list_length)

  if(list_length > scrollData->displayLimit && scrollLimit > 0
     && displayLimit > 0) {
    //Scroll is possible  

    scrollData->scrollActive = SCROLL_ACTIVE;
    aux = head;

    currentListIndex = 0;	//We listBox1 the scroll at the top index.
    scrollData->currentListIndex = currentListIndex;

    //Scroll loop animation. Finish with ENTER.
    do {
      currentListIndex = scrollData->currentListIndex;
      loadlist(aux, scrollData, currentListIndex);
      gotoIndex(&aux, scrollData, currentListIndex);
      ch = selectorMenu(aux, scrollData);
    } while(ch != K_ENTER);

  } else {
    //Scroll is not possible.
    //Display all the elements and create selector.
    scrollData->scrollActive = SCROLL_INACTIVE;
    scrollData->currentListIndex = 0;
    scrollData->displayLimit = list_length;	//Default to list_length
    loadlist(head, scrollData, 0);
    ch = selectorMenu(head, scrollData);
  }
  return ch;
}

/* ---------------- */
/* List files       */
/* ---------------- */

int addSpaces(char temp[MAX_ITEM_LENGTH]) {
  int     i;
  for(i = strlen(temp); i < MAX_ITEM_LENGTH; i++) {
    strcat(temp, " ");
  }
  return 0;
}

void cleanString(char *string, int max) {
  int     i;
  for(i = 0; i < max; i++) {
    string[i] = ' ';
  }
}
int listFiles(LISTCHOICE ** listBox1, char *directory) {
  DIR    *d=NULL;
  struct dirent *dir=NULL;
  int     i;
  char    temp[MAX_ITEM_LENGTH];
  int     lenDir;		//length of directory

  //Add elements to switch directory at the beginning for convenience.
  strcpy(temp, "[INPUT FILE]");
  //Add spaces
  addSpaces(temp);
  *listBox1 = addend(*listBox1, newelement(temp, "[INPUT FILE]", DIRECTORY));	// "."
  strcpy(temp, CHANGEDIR);
  //Add spaces
  addSpaces(temp);
  *listBox1 = addend(*listBox1, newelement(temp, CHANGEDIR, DIRECTORY));	// ".."

  //Start at current directory
  d = opendir(directory);
  //Find directories and add them to list first
  if(d) {
    while((dir = readdir(d)) != NULL) {
      if(dir->d_type == DT_DIR) {

	lenDir = strlen(dir->d_name);

	//Check length of directory
	//Directories are displayed between brackets [directory]
	if(lenDir > MAX_ITEM_LENGTH - 2) {
	  //Directory name is long. CROP
	  cleanString(temp, MAX_ITEM_LENGTH);
	  strcpy(temp, "[");
	  for(i = 1; i < MAX_ITEM_LENGTH - 1; i++) {
	    temp[i] = dir->d_name[i - 1];
	  }
	  temp[MAX_ITEM_LENGTH - 1] = ']';
	} else {
	  //Directory's name is shorter than display
	  //Add spaces to item string.
	  cleanString(temp, MAX_ITEM_LENGTH);
	  strcpy(temp, "[");
	  for(i = 1; i < lenDir + 1; i++) {
	    temp[i] = dir->d_name[i - 1];
	  }
	  temp[lenDir + 1] = ']';
	  addSpaces(temp);
	}
	//Add all directories except CURRENTDIR and CHANGEDIR
	if(strcmp(dir->d_name, CURRENTDIR) != 0
	   && strcmp(dir->d_name, CHANGEDIR) != 0)
	  *listBox1 =
	      addend(*listBox1, newelement(temp, dir->d_name, DIRECTORY));
      }
    }
  }
  closedir(d);

  //Find files and add them to list after directories
  d = opendir(directory);
  if(d) {
    while((dir = readdir(d)) != NULL) {
      if(dir->d_type == DT_REG) {
	//only list valid fiels
	if(strlen(dir->d_name) > MAX_ITEM_LENGTH) {
	  for(i = 0; i < MAX_ITEM_LENGTH; i++) {
	    temp[i] = dir->d_name[i];
	  }
	} else {
	  strcpy(temp, dir->d_name);
	  //Add spaces
	  addSpaces(temp);
	}
	*listBox1 =
	    addend(*listBox1, newelement(temp, dir->d_name, FILEITEM));
      }
    }
    closedir(d);
  }
  return 0;
}

void changeDir(SCROLLDATA * scrollData, char fullPath[MAX],
	       char newDir[MAX]) {
//Change dir
  char    oldPath[MAX];
  if(scrollData->isDirectory == DIRECTORY) {
    if(scrollData->itemIndex == 1) {
      //cd ..
      cleanString(fullPath, MAX);
      cleanString(oldPath, MAX);
      cleanString(newDir, MAX);
      chdir("..");
      getcwd(oldPath, sizeof(oldPath));
      strcpy(newDir, oldPath);
      strcpy(fullPath, oldPath);
    } else {
      //cd newDir
      cleanString(fullPath, MAX);
      cleanString(newDir, MAX);
      cleanString(oldPath, MAX);
      getcwd(oldPath, sizeof(oldPath));
      strcat(oldPath, "/");
      strcat(oldPath, scrollData->path);
      chdir(oldPath);
      strcpy(newDir, oldPath);
      strcpy(fullPath, oldPath);
    }
  }
}

//Terminal routines

/*-------------------------------------*/
/* Initialize new terminal i/o settings*/
/*-------------------------------------*/
void pushTerm() {
//Save terminal settings in failsafe to be retrived at the end
  tcgetattr(0, &failsafe);
}

/*---------------------------*/
/* Reset terminal to failsafe*/
/*---------------------------*/
int resetTerm() {
  //tcsetattr(0, TCSANOW, &failsafe);
  /* flush and reset */
  if (tcsetattr(0,TCSAFLUSH,&failsafe) < 0) return -1;
  return 0;
}


/*------------------------*/
/* Get terminal dimensions*/
/*------------------------*/
int get_terminal_dimensions(int *rows, int *columns) {
  ioctl(0, TIOCGWINSZ, &max);
  *columns = max.ws_col;
  *rows = max.ws_row;
  return 0;
}

/*--------------------------*/
/* Ansi function hide cursor*/
/*--------------------------*/
void hidecursor() {
  printf("\e[?25l");
}

/*--------------------------*/
/* Ansi function show cursor*/
/*--------------------------*/
void showcursor() {
  printf("\e[?25h");
}
int get_pos(int *y, int *x) {

 char buf[30]={0};
 int ret, i, pow;
 char ch;

*y = 0; *x = 0;

 struct termios term, restore;

 tcgetattr(0, &term);
 tcgetattr(0, &restore);
 term.c_lflag &= ~(ICANON|ECHO);
 tcsetattr(0, TCSANOW, &term);

 write(1, "\033[6n", 4);

 for( i = 0, ch = 0; ch != 'R'; i++ )
 {
    ret = read(0, &ch, 1);
    if ( !ret ) {
       tcsetattr(0, TCSANOW, &restore);
       //fprintf(stderr, "getpos: error reading response!\n");
       return 1;
    }
    buf[i] = ch;
    //printf("buf[%d]: \t%c \t%d\n", i, ch, ch);
 }

 if (i < 2) {
    tcsetattr(0, TCSANOW, &restore);
    //printf("i < 2\n");
    return(1);
 }

 for( i -= 2, pow = 1; buf[i] != ';'; i--, pow *= 10)
     *x = *x + ( buf[i] - '0' ) * pow;

 for( i-- , pow = 1; buf[i] != '['; i--, pow *= 10)
     *y = *y + ( buf[i] - '0' ) * pow;

 tcsetattr(0, TCSANOW, &restore);
 return 0;
}
/*--------------------------------------.*/
/* Detect whether a key has been pressed.*/
/*---------------------------------------*/
int kbhit() {
  unsigned char ch;
  int     nread;
  // tcgetattr(0, &old);               /* grab old terminal i/o settings */
  if(peek_character != -1)
    return 1;
  newp.c_cc[VMIN] = 0;
  tcsetattr(0, TCSANOW, &newp);
  nread = read(0, &ch, 1);
  newp.c_cc[VMIN] = 1;
  tcsetattr(0, TCSANOW, &newp);
  if(nread == 1) {
    peek_character = ch;
    return 1;
  }
  return 0;
}


/*----------------------*/
/*Read char with control*/
/*----------------------*/
int readch() {
  char    ch;

  if(peek_character != -1) {
    ch = peek_character;
    peek_character = -1;
    return ch;
  }
  read(0, &ch, 1);
  return ch;
}

void resetch() {
//Clear ch  
  newp.c_cc[VMIN] = 0;
  tcsetattr(0, TCSANOW, &newp);
  peek_character = 0;
}

/*----------------------------------*/
/* Move cursor to specified position*/
/*----------------------------------*/
void gotoxy(int x, int y) {
  printf("%c[%d;%df", 0x1B, y, x);
}

/*---------------------*/
/* Change colour output*/
/*---------------------*/
void outputcolor(int foreground, int background) {
  printf("%c[%d;%dm", 0x1b, foreground, background);
}

/*-----------------------*/
void resetAnsi(int x) {
  switch (x) {
    case 0:			//reset all colors and attributes
      printf("%c[0m", 0x1b);
      break;
    case 1:			//reset only attributes
      printf("%c[20m", 0x1b);
      break;
    case 2:			//reset foreg. colors and not attrib.
      printf("%c[39m", 0x1b);
      break;
    case 3:			//reset back. colors and not attrib.
      printf("%c[49m", 0x1b);
      break;
    default:
      break;
  }
}


/*----------------------------*/
/* User Interface - Text Box. */
/*----------------------------*/

int textbox(int wherex, int wherey, int displayLength,
	    char label[MAX_TEXTBOX], char text[MAX_TEXTBOX], int backcolor,
	    int labelcolor, int textcolor) {
  int     charCount = 0;
  int     exitFlag = 0;
  int     cursorON = 1;
  long    cursorCount = 0;
  int     i;
  int     limitCursor = 0;
  int     positionx = 0;
  int     posCursor = 0;
  int     keypressed = 0;
  char    displayChar;
  char    ch;
  
  positionx = wherex + strlen(label);
  limitCursor = wherex+strlen(label)+displayLength+1;
  write_str(wherex, wherey, label, backcolor, labelcolor);

  write_ch(positionx, wherey, '[', backcolor, textcolor);
  for(i = positionx + 1; i <= positionx + displayLength; i++) {
    write_ch(i, wherey, '.', backcolor, textcolor);
  }
  write_ch(positionx + displayLength + 1, wherey, ']', backcolor,
	   textcolor);
  //Reset keyboard
  if(kbhit() == 1) ch = readch();
  ch = 0;

  do {
      keypressed = kbhit();
    //Cursor Animation
   if (keypressed == 0){
    cursorCount++;
    if(cursorCount == 100) {
      cursorCount = 0;
      switch (cursorON) {
	case 1:
	  posCursor = positionx + 1;
          displayChar = '.';
          if (posCursor == limitCursor) {
            posCursor = posCursor - 1;
            displayChar = ch;
          }
          write_ch(posCursor, wherey, displayChar, backcolor, textcolor);
          cursorON = 0;
	  break;
	case 0:
          posCursor = positionx + 1;
          if (posCursor == limitCursor) posCursor = posCursor - 1;
	  write_ch(posCursor, wherey, '|', backcolor, textcolor);
          cursorON = 1;
	  break;
      }
     } 
    }
    //Process keys     
    if(keypressed == 1) {
      ch = readch();
      keypressed = 0;
      
      if(charCount < displayLength) {
	if(ch > 31 && ch < 127) {
	  write_ch(positionx + 1, wherey, ch, backcolor, textcolor);
	  text[charCount] = ch;
	  positionx++;
	  charCount++;
	}
      }
    }

    if (ch==K_BACKSPACE){
      if (positionx>0 && charCount>0){
       positionx--;
       charCount--;
       write_ch(positionx + 1, wherey, '.', backcolor, textcolor);
       if (positionx < limitCursor-2) write_ch(positionx + 2, wherey, '.', backcolor, textcolor);
       resetch();
      }
    }
    if(ch == K_ENTER || ch == K_TAB)
      exitFlag = 1;

    //ENTER OR TAB FINISH LOOP
  } while(exitFlag != 1);
  //clear cursor
  write_ch(posCursor, wherey, displayChar, backcolor, textcolor);
  resetch();
  return charCount;
}
//File

void processOptions(char *sourceFileStr,
		    char *destinationFileStr) {
  long    newFileSize;
  int     okFile, okFile2;
  FILE   *fileSource, *fileDestination;

   if (strcmp(sourceFileStr, destinationFileStr) != 0){
      okFile = openFile(&fileSource, sourceFileStr, "rb");	//read only
      okFile2 = openFile(&fileDestination, destinationFileStr, "wb");	//create destination file   
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
   else{
  	fprintf(stderr, "ERROR: Destination and source files cannot be the same\n");
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
int opfiledialog(int wherey, char filex[MAX_TEXTBOX]) {
  char    ch;
  char    fullPath[MAX];
  char    newDir[MAX];
  int     exitFlag = 0;
  //Change background color

  strcpy(newDir, ".");		//We start at current dir
  getcwd(fullPath, sizeof(fullPath));	//Get path
  scrollData.scrollActive=0;	//To know whether scroll is active or not.
  scrollData.scrollLimit=0;		//Last index for scroll.
  scrollData.listLength=0;		//Total no. of items in the list
  scrollData.currentListIndex=0;	//Pointer to new sublist of items when scrolling.
  scrollData.displayLimit=0;	//No. of elements to be displayed.
  scrollData.scrollDirection=0;	//To keep track of scrolling Direction.
  scrollData.selector=0;		//Y++
  scrollData.wherex=0;		
  scrollData.wherey=0;		
  scrollData.backColor0=0;		//0 unselected; 1 selected
  scrollData.foreColor0=0;
  scrollData.backColor1=0;
  scrollData.foreColor1=0;
  scrollData.isDirectory=0;		// Kind of item
  scrollData.item =NULL;
  scrollData.path =NULL;
  scrollData.itemIndex=0;
  //LISTCHOICE *head;		//store head of the list

  //Directories loop
  do {
    //draw_window(30, wherey+2, 50, wherey+4, 47);	//shadow
    //draw_window(50, 6, columns-10, 15, 41);	//window

    //Add items to list
    if(listBox1 == NULL) 
      listFiles(&listBox1, newDir);
    ch = listBox(listBox1, 30, wherey+1, &scrollData, 47, 30, 40,
		 97, 3);

    //Change Dir. New directory is copied in newDir
    if (scrollData.itemIndex!=0) changeDir(&scrollData, fullPath, newDir);
    //filex = scrollData.path;

    //Scroll Loop exit conditions
    if(scrollData.itemIndex == 0)
      exitFlag = 1;		//First item is selected

    if(ch == K_ENTER && scrollData.isDirectory == FILEITEM)
      exitFlag = 1;		//File selected
    if(scrollData.itemIndex != 0){
        if (strcmp(scrollData.path,"..") != 0)
           strcpy(filex, scrollData.path);
        //printf(AYEL "\n%s", filex);
    }
 
    if(listBox1 != NULL && exitFlag != 1) {
		deleteList(&listBox1);
		listBox1 = NULL;
    } 
  } while(exitFlag != 1);
 if (scrollData.itemIndex == 0) strcpy(filex,"");
 deleteList(&listBox1);

  return 0;

}
int main(){
 char textbox1[MAX_TEXTBOX];
 char textbox2[MAX_TEXTBOX];

 int wherey, wherex;
 get_pos(&wherey,&wherex);
 get_terminal_dimensions(&rows, &columns);
 pushTerm();
 hidecursor();
 resetch();
 if (wherey>20) {wherey = 3 ; system("clear");} 
 printf("\r" AWHT "x0r v1.0"ARST " - Coded by " AWHT "v3l0rek\r\n" ARST);
 printf("\r> Select " AYEL "<source file>" ARST"\n");
 printf("\r");
 printf(text0);
 printf("\r");
 printf(text1);
 printf("\r");
 printf(text2);
 printf("\r");
 printf(text3);
 printf("\r");
 printf(text4);
 printf("\r");
 printf(text5);
 printf("\r");
 printf(text6);
 printf("\n");
 //textbox(30,wherey+2,25,"Source File:",textbox1,37,37,37);
 //if (strcmp(textbox1, "open") ==0){
 opfiledialog(wherey, filex);
 strcpy(textbox1, filex);
 resetAnsi(0);
 if (strcmp(textbox1, "") == 0) {
   textbox(30,wherey+5,25,"Input File:",textbox1,37,37,37);
   textbox(30,wherey+6,25,"Output File:",textbox2,37,37,37);
   if (strcmp(textbox1, "") == 0 || strcmp(textbox2, "") == 0) {
     resetAnsi(0);
     write_str(30,wherey+7,"File not selected!. Exiting!",40,97);
     resetTerm();
     gotoxy(wherex,wherey+9);
     printf("\n");
     printf("\r");
     showcursor();

   } else{
     resetAnsi(0);
     resetTerm();
     gotoxy(wherex,wherey+9);
     printf("\n");
     printf("\r");
     showcursor();
     processOptions(textbox1,textbox2);
  }
} else{
   write_str(30,wherey+5,textbox1,40,97);
   resetAnsi(0);
   textbox(30,wherey+6,25,"Output File:",textbox2,37,37,37);
   if (strcmp(textbox2, "") != 0) {
     resetAnsi(0);
     resetTerm();
     gotoxy(wherex,wherey+9);
     printf("\n");
     printf("\r");
     showcursor();
     processOptions(textbox1,textbox2);
    } else
    {
     resetAnsi(0);
     write_str(30,wherey+7,"File not selected!. Exiting!",40,97);
     resetTerm();
     gotoxy(wherex,wherey+9);
     printf("\n");
     printf("\r");
     showcursor();
    }
  }
 resetAnsi(0);
//resetTerm();

 return 0;
}
