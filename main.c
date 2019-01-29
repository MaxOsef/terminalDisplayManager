#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>


WINDOW *create_newwin(int height, int width, int starty, int startx);
char * intprtkey(int ch);
void destroy_win(WINDOW *local_win);


int main(void) {

    WINDOW * mainwin;
    WINDOW * loginwin;
	int startx, starty, width, height;
    int ch;
	
    /*  Initialize ncurses  */

    if ( (mainwin = initscr()) == NULL ) {
	fprintf(stderr, "Error initializing ncurses.\n");
	exit(EXIT_FAILURE);
    }

    noecho();                  /*  Turn off key echoing                 */
    cbreak();
	keypad(mainwin, TRUE);     /*  Enable the keypad for non-char keys  */
	start_color();

	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	height = 10;
	width = 30;
	starty = (LINES - height) / 2;	/* Calculating for a center placement */
	startx = (COLS - width) / 2;	/* of the window		*/

    /*  Print a prompt and refresh() the screen  */
    
	attron(COLOR_PAIR(1));
	printw("Press F1 to exit");
	refresh();
	attroff(COLOR_PAIR(1));

	loginwin = create_newwin(height, width, starty, startx);

    /*  Loop until user presses 'F1'  */
    while ( (ch = getch()) != KEY_F(1) ) {

		/*  Delete the win, and print a new one  */
		destroy_win(loginwin);
		loginwin = create_newwin(height, width, starty, startx);
		refresh();
    }

    /*  Clean up after ourselves  */
    delwin(mainwin);
    delwin(loginwin);
    endwin();
    refresh();

    return EXIT_SUCCESS;
}


/*  Struct to hold keycode/keyname information  */
struct keydesc {
    int  code;
    char name[20];
};


/*  Returns a string describing a character passed to it  */
char * intprtkey(int ch) {

    /*  Define a selection of keys we will handle
     *
     *\/!\ C'est pas inutile de garder tout ca? On a pas vrmt besoin
           d'afficher tout ces caractère a l'écran de toute facon, 
           jpense pas que tout soit nécessaire \/!\  */
    static struct keydesc keys[] = { { KEY_UP,        "Up arrow"        },
				     { KEY_DOWN,      "Down arrow"      },
				     { KEY_LEFT,      "Left arrow"      },
				     { KEY_RIGHT,     "Right arrow"     },
				     { KEY_HOME,      "Home"            },
				     { KEY_END,       "End"             },
				     { KEY_BACKSPACE, "Backspace"       },
				     { KEY_IC,        "Insert"          },
				     { KEY_DC,        "Delete"          },
				     { KEY_NPAGE,     "Page down"       },
				     { KEY_PPAGE,     "Page up"         },
				     { KEY_F(1),      "Function key 1"  },
				     { KEY_F(2),      "Function key 2"  },
				     { KEY_F(3),      "Function key 3"  },
				     { KEY_F(4),      "Function key 4"  },
				     { KEY_F(5),      "Function key 5"  },
				     { KEY_F(6),      "Function key 6"  },
				     { KEY_F(7),      "Function key 7"  },
				     { KEY_F(8),      "Function key 8"  },
				     { KEY_F(9),      "Function key 9"  },
				     { KEY_F(10),     "Function key 10" },
				     { KEY_F(11),     "Function key 11" },
				     { KEY_F(12),     "Function key 12" },
				     { -1,            "<unsupported>"   }
    };
    static char keych[2] = {0};
    
    if ( isprint(ch) && !(ch & KEY_CODE_YES)) {

	/*  If a printable character  */

	keych[0] = ch;
	return keych;
    }
    else {

	/*  Non-printable, so loop through our array of structs  */

	int n = 0;
	
	do {
	    if ( keys[n].code == ch )
		return keys[n].name;
	    n++;
	} while ( keys[n].code != -1 );

	return keys[n].name;
    }    
    
    return NULL;        /*  We shouldn't get here  */
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

void destroy_win(WINDOW *local_win)
{	
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners 
	 * and so an ugly remnant of window. 
	 */
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	/* The parameters taken are 
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window 
	 * 3. rs: character to be used for the right side of the window 
	 * 4. ts: character to be used for the top side of the window 
	 * 5. bs: character to be used for the bottom side of the window 
	 * 6. tl: character to be used for the top left corner of the window 
	 * 7. tr: character to be used for the top right corner of the window 
	 * 8. bl: character to be used for the bottom left corner of the window 
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wrefresh(local_win);
	delwin(local_win);
}
