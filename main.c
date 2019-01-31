#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>
#include <string.h>
#include <form.h>

#define APP "STDM"
#define N_FIELDS 4
#define HEIGHT 20
#define WIDTH 76

WINDOW *create_newwin(int height, int width, int starty, int startx);
char * intprtkey(int ch);
// void destroy_win(WINDOW *local_win);

int main(void) {

    FIELD * field[N_FIELDS];
    FORM * loginform;
    WINDOW * initwin;
    WINDOW * mainwin;
	int startx, starty;
    int ch, rows, cols;
	
    /*  Initialize ncurses  */
    initwin = initscr();
    
    if ( initwin == NULL ) {
	    fprintf(stderr, "Error initializing ncurses.\n");
	    exit(EXIT_FAILURE);
    }

    noecho();                  /*  Turn off key echoing                 */
    cbreak();
	keypad(stdscr, TRUE);     /*  Enable the keypad for non-char keys  */
	start_color();
	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	starty = (LINES - HEIGHT) / 2;	/* Calculating for a center placement */
	startx = (COLS - WIDTH) / 2;	/* of the window		*/

	/* Initialize the fields */ 
    field[0] = new_field(1, 32, starty+2, startx+15, 0, 0);
    field[1] = new_field(1, 32, starty+4, startx+15, 0, 0);
    field[2] = new_field(1, 25, starty+10, startx+15, 0, 0);
	field[3] = NULL;

	/* Set field options */
	set_field_back(field[0], A_UNDERLINE); 	/* Print a line for the option 	*/
	set_field_back(field[1], A_UNDERLINE); 	/* Print a line for the option 	*/
	field_opts_off(field[1], O_PUBLIC); 	/* This filed is like a password field*/

	
	/* Create the form and post it */
	loginform = new_form(field);

	/* Create window */
    mainwin = create_newwin(HEIGHT, WIDTH, starty, startx);

//  /!\ Je sais pas trop comment me servir de ça encore,
//  on verra plus tard si on en a vraiment besoin /!\
//
//
// 	/* Set main window and sub window */
//     set_form_win(loginform, mainwin);
//     set_form_sub(loginform, derwin(mainwin, rows, cols, 2, 2));

	post_form(loginform);

	attron(COLOR_PAIR(1));
	mvprintw(starty-2, startx, "Press F1 to exit");
	attroff(COLOR_PAIR(1));

    mvprintw(starty+20, startx+60, "Welcome in %s!", APP);
	mvprintw(LINES-2, COLS-50, "Use UP, DOWN arrow keys to switch between fields");
    refresh();

	mvwprintw(mainwin, 2, 2, "Login: ");
	mvwprintw(mainwin, 4, 2, "Password: ");
    wrefresh(mainwin);

    /*  Loop until user presses 'F1'  */
    while ( (ch = getch()) != KEY_F(1) ) {

		switch(ch)
			{	case KEY_DOWN:
					/* Go to next field */
					form_driver(loginform, REQ_NEXT_FIELD);
					/* Go to the end of the present buffer */
					/* Leaves nicely at the last character */
					form_driver(loginform, REQ_END_LINE);
					break;
				case KEY_UP:
					/* Go to previous field */
					form_driver(loginform, REQ_PREV_FIELD);
					form_driver(loginform, REQ_END_LINE);
					break;
				default:
					/* If this is a normal character, it gets */
					/* Printed				  */	
					form_driver(loginform, ch);
					break;
			}
	    
        mvwprintw(mainwin, 2, 2, "Login: ");
	    mvwprintw(mainwin, 4, 2, "Password: ");
        wrefresh(mainwin);
	    
    }
    
	/* Un post form and free the memory */
	unpost_form(loginform);
	free_form(loginform);
	free_field(field[0]);
	free_field(field[1]); 
	free_field(field[2]); 

    /*  Clean up after ourselves  */
    delwin(mainwin);
    endwin();
    refresh();

    return EXIT_SUCCESS;
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
    wborder(local_win, '|', '|', '-', '-', '+', '+', '+', '+'); 

	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}


//  /!\ Pas sur qu'on ai besoin de ça non plus, vu qu'on 
// remplace pas la fenetre, on garde toujours la meme /!\
//
//
// void destroy_win(WINDOW *local_win)
// {	
// 	/* box(local_win, ' ', ' '); : This won't produce the desired
// 	 * result of erasing the window. It will leave it's four corners 
// 	 * and so an ugly remnant of window. 
// 	 */
// 	wborder(local_win, '|', '|', '-', '-', '+', '+', '+', '+');
// 	/* The parameters taken are 
// 	 * 1. win: the window on which to operate
// 	 * 2. ls: character to be used for the left side of the window 
// 	 * 3. rs: character to be used for the right side of the window 
// 	 * 4. ts: character to be used for the top side of the window 
// 	 * 5. bs: character to be used for the bottom side of the window 
// 	 * 6. tl: character to be used for the top left corner of the window 
// 	 * 7. tr: character to be used for the top right corner of the window 
// 	 * 8. bl: character to be used for the bottom left corner of the window 
// 	 * 9. br: character to be used for the bottom right corner of the window
// 	 */
// 	wrefresh(local_win);
// 	delwin(local_win);
// }
// 
