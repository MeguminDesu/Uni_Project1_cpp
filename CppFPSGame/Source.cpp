#pragma comment(lib, "winmm.lib")
#include <iostream>

#include <string>
#include <vector>

#include <Windows.h>

#include <chrono>
#include <stdio.h>
#include <time.h>

#include <mmsystem.h>

////////////////////////////////////////

const int nScreenWidth = 120;
const int nScreenHeight = 40;

////////////////////////////////////////

struct pix_coord;

wchar_t* pix_drawPixel(wchar_t*, int, int, char);
wchar_t* pix_drawPixel(wchar_t*, int, int);

wchar_t* pix_background(wchar_t*);
wchar_t* pix_background(wchar_t*, char);

wchar_t* pix_rect(wchar_t*, int, int, int, int, char, char);

wchar_t* pix_text(wchar_t*, int, int, std::string);
wchar_t* pix_textBlock(wchar_t*, int, int, std::string);

wchar_t* title_screen(wchar_t*, int);

void gameSetup();
wchar_t* gameFrame(wchar_t*);

////////////////////////////////////////

enum class egame_code { title, options, ingame };

////////////////////////////////////////

struct pix_Keyboard_Inputs {
	/* Movement */
	bool w = false;
	bool a = false;
	bool s = false;
	bool d = false;

	bool e = false;
	bool p = false;
} pix_kb;

struct pix_coord {
	int x = 0;
	int y = 0;

	char c = ' ';
};

struct _s_title {
	int selected = 0;
	int selected_m = 2;
	int change_delay = 0;
	int change_delay_m = 150;
} s_title;

struct _s_options {
	int selected = 0;
	int selected_m = 0;
	int change_delay = 0;
	int change_delay_m = 100;
} s_options;

struct _s_game {
	int change_delay = 0;
	int change_delay_m = 150;
} s_game;

////////////////////////////////////////

class Board
{
private:
	int get_random_num(int pMin, int pMax)
	{
		int randNum = rand() % (pMax - pMin + 1) + pMin;
		return randNum;
	}


public:

	int board_offsetx = 11,
		board_offsety = 2;

	int boardSizex = 32;
	int boardSizey = 12;

	int timer = 30;
	int timer_m = 30;

	int gamescore = 0;
	int gametimer = 30; // Seconds
	int gametimer_m = 30;
	int gamelastsecond = 0;

	struct Player {
		int posx = 0;
		int posy = 0;
		char c = '^';
	} player;

	class Cell {
	public:
		int x, y;
		char c = ' ';
		char wallchar = '#';

		bool walltop = true;
		bool wallrig = true;
		bool walllef = true;
		bool wallbot = true;

		bool visited = false;

		Cell() {}
		Cell(int x, int y) { this->x = x; this->y = y; }
		void render(wchar_t* pScreen, int offx, int offy)
		{
			pix_rect(pScreen, this->x * 3 + offx, this->y * 3 + offy, 3, 3, this->wallchar, ' ');
			if (!this->walltop) pix_drawPixel(pScreen, this->x * 3 + offx + 1, this->y * 3 + offy, ' ');
			if (!this->wallrig) pix_drawPixel(pScreen, this->x * 3 + offx + 2, this->y * 3 + offy + 1, ' ');
			if (!this->walllef) pix_drawPixel(pScreen, this->x * 3 + offx, this->y * 3 + offy + 1, ' ');
			if (!this->wallbot) pix_drawPixel(pScreen, this->x * 3 + offx + 1, this->y * 3 + offy + 2, ' ');
		}
	};

	Cell cells[32][12];

	Board() {
		return;
	}

	void fill()
	{
		for (int i = 0; i < this->boardSizex; i++)
		{
			for (int j = 0; j < this->boardSizey; j++)
			{
				cells[i][j] = Cell(i, j);
			}
		}
	}

	struct coord {
		int x = 0;
		int y = 0;
		coord(int x, int y)
		{
			this->x = x;
			this->y = y;
		}
	};

	void generate(int x, int y)
	{
		if  (cells[x][y].visited)  return;
		else cells[x][y].visited = true;

		std::cout << "> " << x << "|" << y << std::endl;
		std::vector<coord> valid = std::vector<coord>();
		// Get all VALID neighbours
		if (!(x - 1 < 0                )) { if (this->cells[x - 1][y].visited == false) { valid.push_back(coord(x - 1, y)); } }
		if (!(x + 1 >= this->boardSizex)) { if (this->cells[x + 1][y].visited == false) { valid.push_back(coord(x + 1, y)); } }
		if (!(y - 1 < 0                )) { if (this->cells[x][y - 1].visited == false) { valid.push_back(coord(x, y - 1)); } }
		if (!(y + 1 >= this->boardSizey)) { if (this->cells[x][y + 1].visited == false) { valid.push_back(coord(x, y + 1)); } }
		for (coord c : valid) std::cout << c.x << "|" << c.y << std::endl;
		std::cout << "-----" << std::endl;
		
		// Pick Random VALID
		while (valid.size() > 0)
		{
			int r = this->get_random_num(0, valid.size() - 1);
			coord c = valid.at(r);

			int xd = x - c.x;
			int yd = y - c.y;

			// REMOVE WALLS BETWEEN THIS AND VALID
			if (xd == 1) {
				cells[c.x][c.y].c = '>';
			}
			else if (xd == -1) {
				cells[c.x][c.y].c = '<';
			}

			else if (yd == 1) {
				cells[c.x][c.y].c = 'v';
			}
			else if (yd == -1) {
				cells[c.x][c.y].c = '^';
			}
			
			
			// Generate(VALID.x, valid.Y)
			valid.erase(r + valid.begin());
			this->generate(c.x, c.y);
		}
	}

	void breakWalls()
	{
		++this->gamescore;
		for (int i = 0; i < this->boardSizex; i++)
		{
			for (int j = 0; j < this->boardSizey; j++)
			{
				char c = this->cells[i][j].c;

				if (c == '^')
				{
					try {
						this->cells[i][j].walltop = false;
						this->cells[i][j - 1].wallbot = false;
					}
					catch (std::exception ex) {}
				}
				if (c == 'v')
				{
					try {
						this->cells[i][j    ].wallbot = false;
						this->cells[i][j + 1].walltop = false;
					}
					catch (std::exception ex) {}
				}

				if (c == '<')
				{
					try {
						this->cells[i][j].walllef = false;
						this->cells[i - 1][j].wallrig = false;
					}
					catch (std::exception ex) {}
				}
				if (c == '>')
				{
					try {
						this->cells[i][j].wallrig = false;
						this->cells[i + 1][j].walllef = false;
					}
					catch (std::exception ex) {}
				}
			}
		}

		cells[this->boardSizex - 1][this->boardSizey - 1].wallrig = false;
	}

	void reset()
	{
		this->fill();
		this->generate(0, 0);
		this->breakWalls();
	}

	void frame(wchar_t* pScreen, int dx, int dy)
	{
		Cell cc = this->cells[this->player.posx][this->player.posy];
		if(dy == -1 // top
		&& cc.walltop == false)
		{
			this->player.posy -= 1;
		}
		if (dy == 1 // top
			&& cc.wallbot == false)
		{
			this->player.posy += 1;
		}

		if (dx == -1 // top
			&& cc.walllef == false)
		{
			this->player.posx -= 1;
		}
		if (dx == 1 // top
			&& cc.wallrig == false)
		{
			this->player.posx += 1;
		}

		if (this->player.posx == this->boardSizex && this->player.posy == this->boardSizey - 1)
		{
			this->player.posx = 0;
			this->player.posy = 0;
			this->reset();
		}

		this->timer = this->timer_m;
		return;
	}

	void render(wchar_t* pScreen)
	{
		if (pix_kb.w == true) {
			this->player.c = '^';
			if (this->timer > 0) { this->timer--; }
			else { this->frame(pScreen, 0, -1); }
		}
		else if (pix_kb.a == true) {
			this->player.c = '<';
			if (this->timer > 0) { this->timer--; }
			else { this->frame(pScreen, -1, 0); }
		}
		else if (pix_kb.s == true) {
			this->player.c = 'v';
			if (this->timer > 0) { this->timer--; }
			else { this->frame(pScreen, 0, 1); }
		}
		else if (pix_kb.d == true) {
			this->player.c = '>';
			if (this->timer > 0) { this->timer--; }
			else { this->frame(pScreen, 1, 0); }
		}


		// pix_rect(pScreen, this->board_offsetx, this->board_offsety, this->boardSizex, this->boardSizey, '#', ' ');

		std::string gs = std::to_string(this->gamescore);
		if (gs.length() < 2) gs = "0" + gs;

		pix_textBlock(pScreen, 3, 1, "L");
		pix_textBlock(pScreen, 3, 7, "E");
		pix_textBlock(pScreen, 3, 13, "V");
		pix_textBlock(pScreen, 3, 19, "E");
		pix_textBlock(pScreen, 3, 25, "L");
		pix_textBlock(pScreen, 1, 31, gs);


		// Draw Cells & Player
		for (int i = 0; i < this->boardSizex; i++)
		{
			for (int j = 0; j < this->boardSizey; j++)
			{
				cells[i][j].render(pScreen, this->board_offsetx, this->board_offsety);
			}
		}

		pix_drawPixel(pScreen, (this->player.posx * 3) + this->board_offsetx + 1, (this->player.posy * 3) + this->board_offsety + 1, this->player.c);
	}
};

////////////////////////////////////////

Board board;

////////////////////////////////////////

int main()
{
	// Game Setup
	board = Board();
	gameSetup();

	// Setup Screen
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Common
	egame_code game_code = egame_code::title;

	// Game Loop
	bool isRunning = true;
	while (isRunning)
	{
		/* Reset Screen */
		pix_background(screen);

		/* Handle Inputs */
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) pix_kb.w = true; else pix_kb.w = false;
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000) pix_kb.a = true; else pix_kb.a = false;
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) pix_kb.s = true; else pix_kb.s = false;
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000) pix_kb.d = true; else pix_kb.d = false;

		if (GetAsyncKeyState((unsigned short)'E') & 0x8000) pix_kb.e = true; else pix_kb.e = false;
		if (GetAsyncKeyState((unsigned short)'P') & 0x8000) pix_kb.p = true; else pix_kb.p = false;

		/* Main Game Loop */
		switch (game_code)
		{
		case egame_code::title:
			if (s_title.change_delay > 0) --s_title.change_delay; else
			{
				if (pix_kb.a) { s_title.selected--; if (s_title.selected < 0) s_title.selected = 0; s_title.change_delay += s_title.change_delay_m; }
				if (pix_kb.d) { s_title.selected++; if (s_title.selected > s_title.selected_m) s_title.selected = s_title.selected_m; s_title.change_delay += s_title.change_delay_m; }
			}

			title_screen(screen, s_title.selected);

			if (pix_kb.e)
			{
				// SwitchGameCode
				switch (s_title.selected)
				{
				case 0:
					game_code = egame_code::ingame;
					break;
				case 1:
					game_code = egame_code::options;
					break;
				case 2:
					isRunning = false;
				}
			}
			break;

		case egame_code::options:
			// No Options ATM
			game_code = egame_code::title;
			break;

		case egame_code::ingame:
			gameFrame(screen);
			break;

		default:
			isRunning = false;
			break;
		}

		/* Print Screen/Frame To Console */
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	return 0;
}

////////////////////////////////////////

wchar_t* pix_drawPixel(wchar_t* pScreen, int pX, int pY)
{
	return pix_drawPixel(pScreen, pX, pY, ' ');
}
wchar_t* pix_drawPixel(wchar_t* pScreen, int pX, int pY, char pC)
{
	int nLoc = pY * nScreenWidth + pX;
	if (pX > nScreenWidth
		|| pY > nScreenHeight
		|| nLoc >= nScreenWidth * nScreenHeight)
		return pScreen;
	else
		pScreen[pY * nScreenWidth + pX] = pC;

	return pScreen;
}

wchar_t* pix_background(wchar_t* pScreen)
{
	return pix_background(pScreen, ' ');
}
wchar_t* pix_background(wchar_t* pScreen, char pC)
{
	for (int i = 0; i < nScreenWidth; i++) {
		for (int j = 0; j < nScreenHeight; j++) {
			pix_drawPixel(pScreen, i, j, pC);
		}
	}
	return pScreen;
}

wchar_t* pix_rect(wchar_t* pScreen, int pX, int pY, int pSx, int pSy, char pO, char pF)
{
	/* Calculate all Coords of the rectangle:
	*....If val >= nScreenWidth * nScreenHeight
	*........Skip it
	*........Draw Pixel at each coordinate
	*/

	std::vector<pix_coord> coor = std::vector<pix_coord>();
	for (int i = pX; i < pX + pSx; i++) {
		if (i >= nScreenWidth) continue;
		for (int j = pY; j < pY + pSy; j++) {
			if (j >= nScreenHeight) continue;
			pix_coord px = pix_coord();
			px.x = i;
			px.y = j;
			px.c = ((i == pX || i == pX + pSx - 1)
				|| (j == pY || j == pY + pSy - 1) ? pO : pF);
			coor.push_back(px);
		}
	}

	for (const pix_coord pc : coor)
		pix_drawPixel(pScreen, pc.x, pc.y, pc.c);

	return pScreen;
}

wchar_t* pix_text(wchar_t* pScreen, int pX, int pY, std::string pText)
{
	for (char c : pText)
	{
		pix_drawPixel(pScreen, pX++, pY, c);
	}
	return pScreen;
}

namespace pixCharBlock {
	std::vector<std::string> A = std::vector<std::string>{
		" ## ",
		"#  #",
		"####",
		"#  #",
		"#  #"
	};

	std::vector<std::string> B = std::vector<std::string>{
		"### ",
		"#  #",
		"###",
		"#  #",
		"### "
	};

	std::vector<std::string> C = std::vector<std::string>{
		" ###",
		"#   ",
		"#   ",
		"#   ",
		" ###"
	};

	std::vector<std::string> D = std::vector<std::string>{
		"### ",
		"#  #",
		"#  #",
		"#  #",
		"### "
	};

	std::vector<std::string> E = std::vector<std::string>{
		"####",
		"#   ",
		"####",
		"#   ",
		"####"
	};

	std::vector<std::string> F = std::vector<std::string>{
		"####",
		"#   ",
		"####",
		"#   ",
		"#   "
	};

	std::vector<std::string> G = std::vector<std::string>{
		"####",
		"#   ",
		"# ##",
		"#  #",
		"####"
	};

	std::vector<std::string> H = std::vector<std::string>{
		"#  #",
		"#  #",
		"####",
		"#  #",
		"#  #"
	};

	std::vector<std::string> I = std::vector<std::string>{
		"####",
		" ## ",
		" ## ",
		" ## ",
		"####"
	};

	std::vector<std::string> J = std::vector<std::string>{
		"  ##",
		"   #",
		"   #",
		"#  #",
		" ## "
	};

	std::vector<std::string> K = std::vector<std::string>{
		"#  #",
		"# # ",
		"##  ",
		"# # ",
		"#  #"
	};

	std::vector<std::string> L = std::vector<std::string>{
		"#   ",
		"#   ",
		"#   ",
		"#   ",
		"####"
	};

	std::vector<std::string> M = std::vector<std::string>{
		" # # ",
		"# # #",
		"# # #",
		"# # #",
		"# # #"
	};

	std::vector<std::string> N = std::vector<std::string>{
		"#   #",
		"##  #",
		"# # #",
		"#  ##",
		"#   #"
	};

	std::vector<std::string> O = std::vector<std::string>{
		"####",
		"#  #",
		"#  #",
		"#  #",
		"####"
	};

	std::vector<std::string> P = std::vector<std::string>{
		"####",
		"#  #",
		"####",
		"#   ",
		"#   "
	};

	std::vector<std::string> Q = std::vector<std::string>{
		" ## ",
		"#  #",
		"#  #",
		"#  #",
		" ## ",
		"   #"
	};

	std::vector<std::string> R = std::vector<std::string>{
		"### ",
		"#  #",
		"### ",
		"#  #",
		"#  #"
	};

	std::vector<std::string> S = std::vector<std::string>{
		" ###",
		"#   ",
		" ## ",
		"   #",
		"### "
	};

	std::vector<std::string> T = std::vector<std::string>{
		"####",
		" ## ",
		" ## ",
		" ## ",
		" ## "
	};

	std::vector<std::string> U = std::vector<std::string>{
		"#  #",
		"#  #",
		"#  #",
		"#  #",
		" ## "
	};

	std::vector<std::string> V = std::vector<std::string>{
		"#   #",
		"#   #",
		"#   #",
		" # # ",
		"  #  "
	};

	std::vector<std::string> W = std::vector<std::string>{
		"#   #",
		"#   #",
		"# # #",
		"# # #",
		" ### "
	};

	std::vector<std::string> X = std::vector<std::string>{
		"#   #",
		" # # ",
		"  #  ",
		" # # ",
		"#   #"
	};

	std::vector<std::string> Y = std::vector<std::string>{
		"#  #",
		"#  #",
		" ## ",
		" ## ",
		" ## "
	};

	std::vector<std::string> Z = std::vector<std::string>{
		"####",
		"   #",
		" ## ",
		"#   ",
		"####"
	};

	std::vector<std::string> one = std::vector<std::string>{
		"  # ",
		" ## ",
		"  # ",
		"  # ",
		" ###"
	};

	std::vector<std::string> two = std::vector<std::string>{
		" ## ",
		"   #",
		" ## ",
		"#   ",
		" ## "
	};

	std::vector<std::string> three = std::vector<std::string>{
		"### ",
		"   #",
		" ## ",
		"   #",
		"### "
	};

	std::vector<std::string> four = std::vector<std::string>{
		"  ##",
		" # #",
		"####",
		"   #",
		"   #"
	};

	std::vector<std::string> five = std::vector<std::string>{
		" ## ",
		"#   ",
		" ## ",
		"   #",
		" ## "
	};

	std::vector<std::string> six = std::vector<std::string>{
		" ## ",
		"#   ",
		" ## ",
		"#  #",
		" ## "
	};

	std::vector<std::string> seven = std::vector<std::string>{
		"####",
		"   #",
		"  # ",
		" #  ",
		"#   "
	};

	std::vector<std::string> eight = std::vector<std::string>{
		" ## ",
		"#  #",
		" ## ",
		"#  #",
		" ## "
	};

	std::vector<std::string> nine = std::vector<std::string>{
		" ## ",
		"#  #",
		" ## ",
		"   #",
		" ## "
	};

	std::vector<std::string> zero = std::vector<std::string>{
		" ## ",
		"#  #",
		"#  #",
		"#  #",
		" ## "
	};
};
std::vector<std::string> pix_getCharBlock(char pChar)
{
	switch (pChar) {
	case 'a':
	case 'A':
		return pixCharBlock::A;
	case 'b':
	case 'B':
		return pixCharBlock::B;
	case 'c':
	case 'C':
		return pixCharBlock::C;
	case 'd':
	case 'D':
		return pixCharBlock::D;
	case 'e':
	case 'E':
		return pixCharBlock::E;
	case 'f':
	case 'F':
		return pixCharBlock::F;
	case 'g':
	case 'G':
		return pixCharBlock::G;
	case 'h':
	case 'H':
		return pixCharBlock::H;
	case 'i':
	case 'I':
		return pixCharBlock::I;
	case 'j':
	case 'J':
		return pixCharBlock::J;
	case 'k':
	case 'K':
		return pixCharBlock::K;
	case 'l':
	case 'L':
		return pixCharBlock::L;
	case 'm':
	case 'M':
		return pixCharBlock::M;
	case 'n':
	case 'N':
		return pixCharBlock::N;
	case 'o':
	case 'O':
		return pixCharBlock::O;
	case 'p':
	case 'P':
		return pixCharBlock::P;
	case 'q':
	case 'Q':
		return pixCharBlock::Q;
	case 'r':
	case 'R':
		return pixCharBlock::R;
	case 's':
	case 'S':
		return pixCharBlock::S;
	case 't':
	case 'T':
		return pixCharBlock::T;
	case 'u':
	case 'U':
		return pixCharBlock::U;
	case 'v':
	case 'V':
		return pixCharBlock::V;
	case 'w':
	case 'W':
		return pixCharBlock::W;
	case 'x':
	case 'X':
		return pixCharBlock::X;
	case 'y':
	case 'Y':
		return pixCharBlock::Y;
	case 'z':
	case 'Z':
		return pixCharBlock::Z;
	case '1':
		return pixCharBlock::one;
	case '2':
		return pixCharBlock::two;
	case '3':
		return pixCharBlock::three;
	case '4':
		return pixCharBlock::four;
	case '5':
		return pixCharBlock::five;
	case '6':
		return pixCharBlock::six;
	case '7':
		return pixCharBlock::seven;
	case '8':
		return pixCharBlock::eight;
	case '9':
		return pixCharBlock::nine;
	case '0':
		return pixCharBlock::zero;

	default: return std::vector<std::string>{"    ", "    ", "    ", "    ", "    "};
	}
}

wchar_t* pix_textBlock(wchar_t* pScreen, int pX, int pY, std::string pText)
{
	int yStart = pY;
	for (char s : pText) {
		std::vector<std::string> pcb = pix_getCharBlock(s);
		for (std::string c : pcb) {
			pix_text(pScreen, pX, pY++, c);
		}
		pY = yStart;
		if (s == 'M' || s == 'm'
			|| s == 'N' || s == 'n'
			|| s == 'V' || s == 'v'
			|| s == 'W' || s == 'w'
			|| s == 'X' || s == 'x')
			++pX;

		pX += 5;
	}


	return pScreen;
}


wchar_t* title_screen(wchar_t* pScreen, int pOption_1)
{
	// Icon
	pix_rect(pScreen, 13, 13, 10, 10, '#', '.');
	pix_rect(pScreen, 15, 15, 10, 10, '#', ' ');

	pix_textBlock(pScreen, 35, 13, "Maze");
	pix_textBlock(pScreen, 35, 19, "Runner");

	//// B
	//pix_rect(pScreen, 35, 15, 1, 10, '#', ' ');
	//pix_rect(pScreen, 36, 15, 3, 1, '#', ' ');
	//pix_rect(pScreen, 39, 16, 1, 3, '#', ' ');
	//pix_rect(pScreen, 36, 19, 3, 1, '#', ' ');
	//pix_rect(pScreen, 39, 20, 1, 4, '#', ' ');
	//pix_rect(pScreen, 36, 24, 3, 1, '#', ' ');

	//// L
	//pix_rect(pScreen, 42, 15, 1, 10, '#', ' ');
	//pix_rect(pScreen, 43, 24, 4, 1, '#', ' ');

	//// O
	//pix_rect(pScreen, 49, 15, 1, 10, '#', ' ');
	//pix_rect(pScreen, 50, 15, 3, 1, '#', ' ');
	//pix_rect(pScreen, 50, 24, 3, 1, '#', ' ');
	//pix_rect(pScreen, 53, 15, 1, 10, '#', ' ');

	//// C
	//pix_rect(pScreen, 56, 15, 1, 10, '#', ' ');
	//pix_rect(pScreen, 57, 15, 4, 1, '#', ' ');
	//pix_rect(pScreen, 57, 24, 4, 1, '#', ' ');

	//// K
	//pix_rect(pScreen, 63, 15, 1, 10, '#', ' ');

	//int pix_j = 20;
	//for (int i = 64; pix_j > 15; i++) pix_drawPixel(pScreen, i, --pix_j, '#');
	//pix_j = 20;
	//for (int i = 64; pix_j < 25; i++) pix_drawPixel(pScreen, i, pix_j++, '#');


	//// F
	//pix_rect(pScreen, 71, 17, 1, 10, '#', ' ');
	//pix_rect(pScreen, 72, 17, 4, 1, '#', ' ');
	//pix_rect(pScreen, 72, 21, 4, 1, '#', ' ');

	//// A
	//pix_rect(pScreen, 78, 17, 1, 10, '#', ' ');
	//pix_rect(pScreen, 79, 17, 4, 1, '#', ' ');
	//pix_rect(pScreen, 79, 21, 4, 1, '#', ' ');
	//pix_rect(pScreen, 83, 17, 1, 10, '#', ' ');

	//// L
	//pix_rect(pScreen, 86, 17, 1, 10, '#', ' ');
	//pix_rect(pScreen, 87, 26, 4, 1, '#', ' ');

	//// L
	//pix_rect(pScreen, 93, 17, 1, 10, '#', ' ');
	//pix_rect(pScreen, 94, 26, 4, 1, '#', ' ');


	// Corner Deco
	pix_rect(pScreen, 0, 0, nScreenWidth, 1, '#', ' ');
	pix_rect(pScreen, 20, 1, nScreenWidth - 20, 1, '#', ' ');
	pix_rect(pScreen, 40, 2, nScreenWidth - 40, 1, '#', ' ');
	pix_rect(pScreen, 60, 3, nScreenWidth - 60, 1, '#', ' ');
	pix_rect(pScreen, 80, 4, nScreenWidth - 80, 1, '#', ' ');
	pix_rect(pScreen, 100, 5, nScreenWidth - 100, 1, '#', ' ');

	pix_rect(pScreen, 0, nScreenHeight - 1, 60, 1, '#', ' ');
	pix_rect(pScreen, 0, nScreenHeight - 2, 40, 1, '#', ' ');
	pix_rect(pScreen, 0, nScreenHeight - 3, 20, 1, '#', ' ');


	// Menu Options
	int aniframe = time(NULL) % 4;

	//	// Borders
	pix_rect(pScreen, 36, 25, 32, 1, '-', ' ');
	pix_rect(pScreen, 36, 27, 32, 1, '-', ' ');

	//	// Options

	if (pOption_1 == 0) {
		if (aniframe == 0 || aniframe == 2)
			pix_drawPixel(pScreen, 37, 26, '>');
		else
			pix_drawPixel(pScreen, 38, 26, '>');
	}
	pix_text(pScreen, 39, 26, "Play");

	if (pOption_1 == 1) {
		if (aniframe == 0 || aniframe == 2)
			pix_drawPixel(pScreen, 47, 26, '>');
		else
			pix_drawPixel(pScreen, 48, 26, '>');
	}
	pix_text(pScreen, 49, 26, "Options");

	if (pOption_1 == 2) {
		if (aniframe == 0 || aniframe == 2)
			pix_drawPixel(pScreen, 60, 26, '>');
		else
			pix_drawPixel(pScreen, 61, 26, '>');
	}
	pix_text(pScreen, 62, 26, "Exit");

	// Name
	pix_text(pScreen, nScreenWidth - 1 - 23, 39, "< Luke Poulter | 2021 >");

	return pScreen;
}

////////////////////////////////////////

void gameSetup() {
	srand(time(NULL));
	board.fill();
	board.generate(10, 10);
	board.breakWalls();
	// system("pause");
}

wchar_t* gameFrame(wchar_t* pScreen)
{
	board.render(pScreen);

	return pScreen;
}
