/*******************************************************
	
	Sample Minesweeper
	by oWeRQ
	
	Version: 0.0.5
	
	Compile: gcc mine.c -o mine -lncurses

*******************************************************/

#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>

#define VERSION "0.0.5"
#define AUTOR "oWeRQ"

enum {
	PAIR_DEFAULT,
	PAIR_CLOSE,
	PAIR_FLAG,
	PAIR_OPEN,
	PAIR_BOMB,
	PAIR_CUR,
	PAIR_STAT,
};

int area_lines=16, area_cols=32, mines_count=64;
int cur_x, cur_y, mines_set;
size_t area_size;

WINDOW *win_stat, *win_mines, *win_keys;

time_t time_start, time_end;

typedef struct {
	bool open;
	bool flag;
	bool bomb;
	int count;
} mine;

mine* mines;

mine* mineyx(int y, int x)
{
	return &mines[x+y*area_cols];
}

void drowDot(int x, int y, bool cur)
{
	chtype ch;
	
	if (mineyx(y, x)->flag)
		ch='F'|COLOR_PAIR(PAIR_FLAG);
	else if (mineyx(y, x)->open)
	{
		if (mineyx(y, x)->bomb)
			ch='*'|COLOR_PAIR(PAIR_BOMB);
		else if (mineyx(y, x)->count>0)
			ch=mineyx(y, x)->count+'0'|COLOR_PAIR(PAIR_OPEN);
		else
			ch=' '|COLOR_PAIR(PAIR_OPEN);
	} 
	else
		ch=' ';
	
	if (cur)
		ch=(char)ch|COLOR_PAIR(PAIR_CUR);

	mvwaddch(win_mines, y+1, x+1, ch);
}

void drowStat()
{
	mvwprintw(win_stat, 0, 1, "Size: %3dx%2d | Mines: %3d /%3d | Time: %.2d:%.2d | Version: %s by %s",
			  area_cols, area_lines, mines_set, mines_count, time_end/60, time_end%60, VERSION, AUTOR);
	wrefresh(win_stat);
}

void drowKeys()
{
	mvwprintw(win_keys, 0, 1, "F - Set Flag | O - Open Dot | N - New Game | Q - Quit");
	wrefresh(win_keys);
}

void drowMines()
{	
	int i, j;

	for (i=0; i<area_lines; i++)
	for (j=0; j<area_cols; j++)
	{
		drowDot(j, i, (j==cur_x && i==cur_y));
	}
	
	wborder(win_mines, 0, 0, 0, 0, 0, 0, 0, 0);
	wrefresh(win_mines);
}

void openDot(int x, int y)
{
	int i, j, bcount=0, fcount=0;
	
	if (mineyx(y, x)->flag)
		return;
	else if (mineyx(y, x)->bomb)
	{
		for (i=0; i<area_lines; i++)
		for (j=0; j<area_cols; j++)
		{
			if (mineyx(i, j)->bomb)
				mineyx(i, j)->open=TRUE;
		}
		return;
	}

	int x_min = (x>0)?x-1:x,
		x_max = (x<area_cols-1)?x+1:x;
	
	int y_min = (y>0)?y-1:y,
		y_max = (y<area_lines-1)?y+1:y;

	for (i=y_min; i<=y_max; i++)
	for (j=x_min; j<=x_max; j++)
	{
		if (mineyx(i, j)->bomb)
			bcount++;
		if (mineyx(i, j)->flag)
			fcount++;
	}

	mineyx(y, x)->count=bcount;
	
	if (bcount==0) {
		mineyx(y, x)->open=TRUE;
		for (i=y_min; i<=y_max; i++)
		for (j=x_min; j<=x_max; j++)
		{
			if (!(x==j && y==i) && !mineyx(i, j)->flag && !mineyx(i, j)->open)
				openDot(j, i);
		}
	} else if (bcount==fcount && mineyx(y, x)->open) {
		for (i=y_min; i<=y_max; i++)
		for (j=x_min; j<=x_max; j++)
		{
			if (!(x==j && y==i) && !mineyx(i, j)->flag && !mineyx(i, j)->open)
				openDot(j, i);
		}
	} else {
		mineyx(y, x)->open=TRUE;
	}
}

void flagDot(int x, int y)
{
	if (mineyx(y, x)->open || mines_set==mines_count)
		return;
	
	mineyx(y, x)->flag=!mineyx(y, x)->flag;
	
	if (mineyx(y, x)->flag)
		mines_set++;
	else
		mines_set--;
}

void newGame()
{
	int i, j, c;
	
	time_start=time(NULL);

	mines_set=0;
	
	memset(mines, 0, area_size);

	if (mines_count>area_lines*area_cols)
		mines_count=area_lines*area_cols/8;

	for (c=0; c<mines_count; c++)
	{
		i=rand()%area_lines;
		j=rand()%area_cols;
		if (mineyx(i, j)->bomb)
			c--;
		else
			mineyx(i, j)->bomb=TRUE;
	}
}

void resize(int lines, int columns)
{
	resizeterm(lines, columns);

	wresize(win_stat, 1, columns);
	mvwin(win_mines, (lines-area_lines-2)/2, (columns-area_cols-2)/2);
	wresize(win_keys, 1, columns);
	mvwin(win_keys, lines-1, 0);

	clear();
	refresh();
	drowStat();
	drowMines();
	drowKeys();
}

void sig_winch(int signo)
{
	struct winsize size;
	ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
	resize(size.ws_row, size.ws_col);
}

int main(int argc,char *argv[])
{
	int i, j;
	chtype ch;

	printf("area cols: ");
	scanf("%d", &area_cols);
	printf("area lines: ");
	scanf("%d", &area_lines);
	printf("mines count: ");
	scanf("%d", &mines_count);

	cur_x=area_cols/2-1;
	cur_y=area_lines/2-1;

	area_size=area_lines*area_cols*sizeof(mine);
	mines=(mine*)malloc(area_size);

	srand(time(NULL));

	initscr();
	signal(SIGWINCH, sig_winch);
	curs_set(FALSE);

	nonl();
	cbreak();
	noecho();

	win_stat=newwin(1, COLS, 0, 0);
	win_mines=newwin(area_lines+2, area_cols+2, (LINES-area_lines-2)/2, (COLS-area_cols-2)/2);
	win_keys=newwin(1, COLS, LINES-1, 0);

	keypad(win_mines, TRUE);

	if (!has_colors())
	{
		endwin();
		printf("Error: Colors not support\n");
		return 1;
	}
	
	start_color();
	init_pair(PAIR_CLOSE, COLOR_WHITE, COLOR_BLACK);
	init_pair(PAIR_OPEN, COLOR_BLACK, COLOR_WHITE);
	init_pair(PAIR_FLAG, COLOR_RED, COLOR_WHITE);
	init_pair(PAIR_BOMB, COLOR_RED, COLOR_BLACK);
	init_pair(PAIR_CUR, COLOR_WHITE, COLOR_GREEN);
	init_pair(PAIR_STAT, COLOR_WHITE, COLOR_BLUE);
	
	wbkgd(stdscr, COLOR_PAIR(PAIR_CLOSE));
	wbkgd(win_stat, COLOR_PAIR(PAIR_STAT));
	wbkgd(win_mines, COLOR_PAIR(PAIR_CLOSE));
	wbkgd(win_keys, COLOR_PAIR(PAIR_STAT));

	newGame();
	drowStat();
	drowMines();
	drowKeys();
	
	while ((ch=wgetch(win_mines))!='q')
	{
		switch (ch)
		{
			case KEY_UP:
				cur_y--;
				if (cur_y<0) cur_y=area_lines-1;
				break;
			case KEY_DOWN:
				cur_y++;
				if (cur_y>=area_lines) cur_y=0;
				break;
			case KEY_LEFT:
				cur_x--;
				if (cur_x<0) cur_x=area_cols-1;
				break;
			case KEY_RIGHT:
				cur_x++;
				if (cur_x>=area_cols) cur_x=0;
				break;
			case '\r':
			case 'o':
				openDot(cur_x, cur_y);
				break;
			case ' ':
			case 'f':
				flagDot(cur_x, cur_y);
				break;
			case 'n':
				newGame();
				break;
			case 'c':
				for (i=0; i<area_lines; i++)
				for (j=0; j<area_cols; j++)
				{
					if (mineyx(i, j)->bomb && !mineyx(i, j)->flag)
						mineyx(i, j)->open=FALSE;
				}
				break;
		}

		if (mines_set!=mines_count)
		{
			time_end=time(NULL)-time_start;
		}

		drowStat();
		drowMines();
	}

	endwin();
	
	free(mines);
	
	return 0;
}
