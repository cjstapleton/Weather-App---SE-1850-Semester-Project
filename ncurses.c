#include <ncurses/ncurses.h> 
#include <stdio.h>


#define PLAYER_PAIR    5

int x,y;

void sunny() {
	start_color();
	init_pair(PLAYER_PAIR, COLOR_YELLOW, COLOR_BLACK);
	attron(COLOR_PAIR(PLAYER_PAIR));
	mvprintw(2, 3, "'.|.'");
	mvprintw(3, 3, "~~O~~");
	mvprintw(4, 3, ".'|'.");
	attroff(COLOR_PAIR(PLAYER_PAIR));
	mvprintw(2, 10, "High : %d", x);
	mvprintw(3, 10, "Low : %d", y);
	mvprintw(6, 3, "Current : %d", x/2);
}

void cloudy() {
	start_color();
	//init_pair(PLAYER_PAIR, COLOR_RED, COLOR_MAGENTA);
	//attron(COLOR_PAIR(PLAYER_PAIR));
	mvprintw(1, 3, "        .~~.");
	mvprintw(2, 3, "      _(    )");
	mvprintw(3, 3, " .-._(       '.");
	mvprintw(4, 3, "(______________)");
	mvprintw(2, 22, "High : %d", x);
	mvprintw(3, 22, "Low : %d", y);
	mvprintw(6, 5, "Current : %d", x/2);
}

void rainy() {
	start_color();
	mvprintw(1, 3, "        .~~.");
	mvprintw(2, 3, " .--.__(    )._");
	mvprintw(3, 3, "(______________')");
	init_pair(PLAYER_PAIR, COLOR_BLUE, COLOR_BLACK);
	attron(COLOR_PAIR(PLAYER_PAIR));
	mvprintw(4, 3, "  :;|;||:||:|;|");
	attroff(COLOR_PAIR(PLAYER_PAIR));
	mvprintw(2, 22, "High : %d", x);
	mvprintw(3, 22, "Low : %d", y);
	mvprintw(6, 5, "Current : %d", x/2);
	
}

void snowy() {
	start_color();
	mvprintw(4, 3, "        .~~.");
	mvprintw(5, 3, " .--.__(    )._");
	mvprintw(6, 3, "(______________')");
	init_pair(PLAYER_PAIR, COLOR_CYAN, COLOR_BLACK);
	attron(COLOR_PAIR(PLAYER_PAIR));
	mvprintw(7, 3, "  +*#*#+*##*+*#");
	attroff(COLOR_PAIR(PLAYER_PAIR));
	mvprintw(4, 22, "High : %d", x);
	mvprintw(5, 22, "Low : %d", y);
	mvprintw(6, 22, "Feels Like : %d", y);
	mvprintw(7, 22, "Current : %d", x/2);
	mvprintw(2, 3, "Location");
	mvprintw(2, 12, "Description");
	mvprintw(8, 22, "Precipitation : %d", y);
}

void er() { 
	for(int i = 0; i<7; i++){
		for(int j = 0; j<35; j++){
			mvaddch(i, j, ' ');
		}
	}
}


int main(int argc, char ** argv)
{
	
	
    // init screen and sets up screen
    initscr();
    // print to screen
    //printw("Hello World");
	getmaxyx(stdscr,y,x);
	//printw("%d, %d", x, y);

	if (has_colors() == FALSE) {
        printw("Your terminal does not support color\n");
    }
	snowy();
    // refreshes the screen
    refresh();
	//er();
	refresh();
    // pause the screen output
    getch();
    // deallocates memory and ends ncurses
    // endwin();
    return 0;
}