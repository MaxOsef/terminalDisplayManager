#include <ctype.h>
#include <form.h>
#include <ncurses.h>
#include <paths.h>
#include <pwd.h>
#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SERVICE_NAME "display_manager"

#define APP "STDM"
#define VERSION "v0.0.1"
#define N_FIELDS 7
#define HEIGHT 10
#define WIDTH 60
// #define KEY_ENTER 65293

#define err(name)                                   \
    do {                                            \
        fprintf(stderr, "%s: %s\n", name,           \
                pam_strerror(pam_handle, result));  \
        end(result);                                \
        return false;                               \
    } while (1);                                    \

static FORM *form;
static FIELD *fields[N_FIELDS];
static WINDOW *win_body, *win_form;

bool login(char *username, char *userpw, char *userdesk, pid_t *child_pid);
bool logout(void);

static void login_func(char *username, char *userpw, char *userdesk);
static void driver(char *username, char *userpw, char *userdesk, int ch);
// static char loaduser(void);

static void init_env(struct passwd *pw);
static void set_env(char *name, char *value);
static int end(int last_result);

static int conv(int num_msg, const struct pam_message **msg,
                struct pam_response **resp, void *appdata_ptr);

static pam_handle_t *pam_handle;

static char* trim_whitespaces(char *str);

int main(void)
{
    int ch, starty, startx;
	char username[32], userpw[32], userdesk[32];

    // Initialize ncurses
    initscr();
    
    noecho();                  // Turn off key echoing
    cbreak();
    keypad(stdscr, TRUE);      // Enable the keypad for non-char keys

    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);

    starty = (LINES - HEIGHT) / 2;	// Calculating for a center placement
    startx = (COLS - WIDTH) / 2;	// of the window

    // create windows
    win_body = newwin(LINES, COLS, 0, 0);
    win_form = derwin(win_body, HEIGHT, WIDTH, starty, startx);
    box(win_form, 0, 0);

    // Print greetings
    mvwprintw(win_body, 1, 2, "Welcome in %s %s !", APP, VERSION);

    wattron(win_body,COLOR_PAIR(1));
    mvwprintw(win_body, 2, 2, "Press F1 to exit");
    wattroff(win_body, COLOR_PAIR(1));

    // Initialize the fields
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

    // Create the form and post it
    form = new_form(fields);

    set_form_win(form, win_form);
    set_form_sub(form, derwin(win_form, 8, 55, 1, 1));

    post_form(form);

    refresh();
    wrefresh(win_body);
    wrefresh(win_form);

    // Moving to the first field
    form_driver(form, REQ_FIRST_FIELD);
    form_driver(form, REQ_INS_MODE);

    // Loop until user presses 'F1'
    while ((ch = getch()) != KEY_F(1)) {
        driver(username, userpw, userdesk, ch);
    }

    
	// Unpost form and free the memory
    unpost_form(form);

    free_form(form);

    free_field(fields[0]);
    free_field(fields[1]);
    free_field(fields[2]);
    free_field(fields[3]);
    free_field(fields[4]);
    free_field(fields[5]);

    // Clean up after ourselves
    endwin();
    refresh();

    return EXIT_SUCCESS;
}

static void driver(char *username, char *userpw, char *userdesk, int ch)
{
    switch(ch)
    {
        case KEY_STAB:
        case 9:
        case KEY_DOWN:

            // Update field buffer
            if (current_field(form) != NULL) { 
                form_driver(form, REQ_VALIDATION);
                
                if (current_field(form) == fields[1]) {
                    strcpy(username, field_buffer(fields[1], 0));
					username = trim_whitespaces(username);
                }
                
                if (current_field(form) == fields[3]) {
                    strcpy(userpw, field_buffer(fields[3], 0));
					userpw = trim_whitespaces(userpw);
                } 
                
                if (current_field(form) == fields[5]) {
                    strcpy(userdesk, field_buffer(fields[5], 0));
					userdesk = trim_whitespaces(userdesk);
                }

            } else {
                //Handle error
            }
            // Go to next field
            form_driver(form, REQ_SNEXT_FIELD);
            form_driver(form, REQ_END_LINE);
            form_driver(form, REQ_INS_MODE);
            break;
        case KEY_UP:
            // Go to previous field
            form_driver(form, REQ_SPREV_FIELD);
            form_driver(form, REQ_END_LINE);
            form_driver(form, REQ_INS_MODE);
            break;
        case KEY_LEFT:
            // Go to previous char
            form_driver(form, REQ_PREV_CHAR);
            break;
        case KEY_RIGHT:
            // Go to next char
            form_driver(form, REQ_NEXT_CHAR);
            break;
        case KEY_BACKSPACE:
        case 127:
            // Delete the char before cursor
            form_driver(form, REQ_DEL_PREV);
            break;
        case KEY_DC:
            // Delete the char under the cursor
            form_driver(form, REQ_DEL_CHAR);
            break;
		case KEY_ENTER:
		case '\n':
            mvprintw(LINES-3, COLS-25, "%s", username);
            mvprintw(LINES-2, COLS-25, "%s", userpw);
            mvprintw(LINES-1, COLS-25, "%s", userdesk);
            getch();
			login_func(username, userpw, userdesk);
			break;
        default:
            // Print normal charaters
            form_driver(form, ch);
            break;
    }

    wrefresh(win_form);
}

static void login_func(char *username, char *userpw, char *userdesk) {
	
    pid_t child_pid;
    if (login(username, userpw, userdesk, &child_pid)) {
		addstr("Shelling out...");
    	def_prog_mode();           /* save current tty modes */
    	endwin();                  /* restore original tty modes */
    	system("zsh");              /* run shell */
	
        // Wait for child process to finish (wait for logout)
        int status;
        waitpid(child_pid, &status, 0);

    	addstr("returned.\n");     /* prepare return message */
    	refresh();                 /* restore save modes, repaint screen */

        logout();
    } else {
		mvaddstr(5, 5, "Error while login in...");
    }

	/*  Attention !! 
		Il faut penser à vider le 
		champ userpw avant de revenir 
		dans ncurses	*/
}

bool login(char *username, char *userpw, char *userdesk, pid_t *child_pid) {
    const char *data[2] = {username, userpw};
    struct pam_conv pam_conv = {
        conv, data
    };

    int result = pam_start(SERVICE_NAME, username, &pam_conv, &pam_handle);
    if (result != PAM_SUCCESS) {
        err("pam_start");
    }

    result = pam_authenticate(pam_handle, 0);
    if (result != PAM_SUCCESS) {
        err("pam_authenticate");
    }

    result = pam_acct_mgmt(pam_handle, 0);
    if (result != PAM_SUCCESS) {
        err("pam_acct_mgmt");
    }

    result = pam_setcred(pam_handle, PAM_ESTABLISH_CRED);
    if (result != PAM_SUCCESS) {
        err("pam_setcred");
    }

    result = pam_open_session(pam_handle, 0);
    if (result != PAM_SUCCESS) {
        pam_setcred(pam_handle, PAM_DELETE_CRED);
        err("pam_open_session");
    }

    struct passwd *pw = getpwnam(username);
    init_env(pw);

    *child_pid = fork();
    if (*child_pid == 0) {
        chdir(pw->pw_dir);
        // We don't use ~/.xinitrc because we should already be in the users home directory
        char *cmd = "exec /bin/zsh --login .xinitrc";
        execl(pw->pw_shell, pw->pw_shell, "-c", cmd, NULL);
        printf("Failed to start window manager");
        exit(1);
    }

    return true;
}

static int conv(int num_msg, const struct pam_message **msg,
                struct pam_response **resp, void *appdata_ptr) {
    int i;

    *resp = calloc(num_msg, sizeof(struct pam_response));
    if (*resp == NULL) {
        return PAM_BUF_ERR;
    }

    int result = PAM_SUCCESS;
    for (i = 0; i < num_msg; i++) {
        char *username, *userpw;
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_ON:
            username = ((char **) appdata_ptr)[0];
            (*resp)[i].resp = strdup(username);
            break;
        case PAM_PROMPT_ECHO_OFF:
            userpw = ((char **) appdata_ptr)[1];
            (*resp)[i].resp = strdup(userpw);
            break;
        case PAM_ERROR_MSG:
            fprintf(stderr, "%s\n", msg[i]->msg);
            result = PAM_CONV_ERR;
            break;
        case PAM_TEXT_INFO:
            printf("%s\n", msg[i]->msg);
            break;
        }
        if (result != PAM_SUCCESS) {
            break;
        }
    }

    if (result != PAM_SUCCESS) {
        free(*resp);
        *resp = 0;
    }

    return result;
}

static void init_env(struct passwd *pw) {
    set_env("HOME", pw->pw_dir);
    set_env("PWD", pw->pw_dir);
    set_env("SHELL", pw->pw_shell);
    set_env("USER", pw->pw_name);
    set_env("LOGNAME", pw->pw_name);
    set_env("PATH", "/usr/local/sbin:/usr/local/bin:/usr/bin");
    set_env("MAIL", _PATH_MAILDIR);

    size_t xauthority_len = strlen(pw->pw_dir) + strlen("/.Xauthority") + 1;
    char *xauthority = malloc(xauthority_len);
    snprintf(xauthority, xauthority_len, "%s/.Xauthority", pw->pw_dir);
    set_env("XAUTHORITY", xauthority);
    free(xauthority);
}

static void set_env(char *name, char *value) {
    // The `+ 2` is for the '=' and the null byte
    size_t name_value_len = strlen(name) + strlen(value) + 2;
    char *name_value = malloc(name_value_len);
    snprintf(name_value, name_value_len,  "%s=%s", name, value);
    pam_putenv(pam_handle, name_value);
    free(name_value);
}

bool logout(void) {
    int result = pam_close_session(pam_handle, 0);
    if (result != PAM_SUCCESS) {
        pam_setcred(pam_handle, PAM_DELETE_CRED);
        err("pam_close_session");
    }

    result = pam_setcred(pam_handle, PAM_DELETE_CRED);
    if (result != PAM_SUCCESS) {
        err("pam_setcred");
    }

    end(result);
    return true;
}

static int end(int last_result) {
    int result = pam_end(pam_handle, last_result);
    pam_handle = 0;
    return result;
}
 
static char* trim_whitespaces(char *str)
{
	char *end;

	// trim leading space
	while(isspace(*str))
		str++;

	if(*str == 0) // all spaces?
		return str;

	// trim trailing space
	end = str + strnlen(str, 128) - 1;

	while(end > str && isspace(*end))
		end--;

	// write new null terminator
	*(end+1) = '\0';

	return str;
}

