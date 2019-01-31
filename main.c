#include <stdlib.h>
#include <ctype.h>
#include <ncurses.h>
#include <string.h>
#include <form.h>

#define APP "STDM"
#define VERSION "v0.0.1"
#define N_FIELDS 7
#define HEIGHT 10
#define WIDTH 60

static FORM *form;
static FIELD *fields[N_FIELDS];
static WINDOW *win_body, *win_form;

static void driver(int ch);

int main(void)
{
    int ch, starty, startx;
	
    /*  Initialize ncurses  */
    initscr();
    
    noecho();                  /*  Turn off key echoing                 */
    cbreak();
	keypad(stdscr, TRUE);      /*  Enable the keypad for non-char keys  */

	start_color();
	init_pair(1, COLOR_CYAN, COLOR_BLACK);

	starty = (LINES - HEIGHT) / 2;	/* Calculating for a center placement */
	startx = (COLS - WIDTH) / 2;	/* of the window		*/

    // create windows
    win_body = newwin(LINES, COLS, 0, 0);
	win_form = derwin(win_body, HEIGHT, WIDTH, starty, startx);
    box(win_form, 0, 0);

    // Print greetings
    mvwprintw(win_body, 1, 2, "Welcome in %s %s !", APP, VERSION);

	wattron(win_body,COLOR_PAIR(1));
	mvwprintw(win_body, 2, 2, "Press F1 to exit");
	wattroff(win_body, COLOR_PAIR(1));

	/* Initialize the fields */ 
    fields[0] = new_field(1, 32, 1, 2, 0, 0);
    fields[1] = new_field(1, 32, 1, 15, 0, 0);
    fields[2] = new_field(1, 32, 3, 2, 0, 0);
    fields[3] = new_field(1, 32, 3, 15, 0, 0);
    fields[4] = new_field(1, 32, 5, 2, 0, 0);
    fields[5] = new_field(1, 32, 5, 15, 0, 0);
    fields[6] = NULL;

	set_field_buffer(fields[0], 0, "Login : ");
	set_field_buffer(fields[2], 0, "Password : ");
	set_field_buffer(fields[4], 0, "Desktop : ");

	set_field_opts(fields[0], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
	set_field_opts(fields[1], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);
	set_field_opts(fields[2], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
	set_field_opts(fields[3], O_VISIBLE | O_EDIT | O_ACTIVE);
	set_field_opts(fields[4], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
	set_field_opts(fields[5], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);

	set_field_back(fields[1], A_UNDERLINE);
	set_field_back(fields[3], A_UNDERLINE);
	set_field_back(fields[5], A_UNDERLINE);

	/* Create the form and post it */
	form = new_form(fields);

	set_form_win(form, win_form);
	set_form_sub(form, derwin(win_form, 8, 55, 1, 1));

	post_form(form);

    refresh();
    wrefresh(win_body);
    wrefresh(win_form);

    move(starty+2, startx+16);

    /*  Loop until user presses 'F1'  */
    while ((ch = getch()) != KEY_F(1)) {
        driver(ch);
    }
    
	/* Unpost form and free the memory */
	unpost_form(form);

	free_form(form);

	free_field(fields[0]);
	free_field(fields[1]); 
	free_field(fields[2]); 
	free_field(fields[3]);
	free_field(fields[4]); 
	free_field(fields[5]); 

    /*  Clean up after ourselves  */
    endwin();
    refresh();

    return EXIT_SUCCESS;
}

static void driver(int ch)
{
    switch(ch)
    {
        case KEY_STAB:
        case 9:
        case KEY_DOWN:
            /* Go to next field */
            form_driver(form, REQ_SNEXT_FIELD);
            form_driver(form, REQ_END_LINE);
            break;
        case KEY_UP:
            /* Go to previous field */
            form_driver(form, REQ_SPREV_FIELD);
            form_driver(form, REQ_END_LINE);
            break;
        case KEY_LEFT:
            /* Go to previous char */
            form_driver(form, REQ_PREV_CHAR);
            break;
        case KEY_RIGHT:
            /* Go to next char */
            form_driver(form, REQ_NEXT_CHAR);
            break;
        // Delete the char before cursor
        case KEY_BACKSPACE:
        case 127:
            form_driver(form, REQ_DEL_PREV);
            break;
        // Delete the char under the cursor
        case KEY_DC:
            form_driver(form, REQ_DEL_CHAR);
            break;
        default:
            /* If this is a normal character, it gets */
            /* Printed				  */	
            form_driver(form, ch);
            break;
    }

    wrefresh(win_form);
}
