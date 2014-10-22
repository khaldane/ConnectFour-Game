/*
 * Name: Katherine Haldane & Jared Lerner
 * Date: April 21, 2014
 * Description: Client that displays the connect four board to the user, and manages the gameplay.
 */
#include <SocketLib.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

using namespace std;

/*
 * Description: Structure that manages the console height, and width.
 *				It also manages the colour of console text.
 */
struct consoleSize {
	consoleSize(unsigned width, unsigned height) {
		HWND console = GetConsoleWindow();
		COORD      c;
		auto hConOut = GetStdHandle( STD_OUTPUT_HANDLE );

		c.X = (short)width;
		c.Y = (short)height;
		SetConsoleScreenBufferSize( hConOut, c );
		RECT r;
		GetWindowRect(console, &r); //stores the console's current dimensions
		MoveWindow(console, r.left, r.top, width, height, TRUE);
		
	}

	~consoleSize() {

	}

	//Changes the colour of the console text
	void colour (WORD colour = 0x1B) {
		auto hConOut = GetStdHandle( STD_OUTPUT_HANDLE );
		SetConsoleTextAttribute (hConOut, colour);
	}
};

//Global Variables
consoleSize _con (700, 600);
Socket _s;
vector<vector<int>> _gameState;
int _playerColour;
int _opponentColour;
const int _WHITE = 7;


/*
 * Purpose: Connects the clients IP address to the socket server
 * Parameters: none
 * Return Value: void
 */
void connectToServer(string ipAddress = "localhost") {
	if (ipAddress == "localhost") {
		_s.connect();
	}
	else {
		_s.connect(ipAddress);
	}
}

/*
 * Purpose: Allows a player to pick a colour from the list, and determine if it has already been by another player
 * Parameters: none
 * Return Value: void
 */
void pickColour() {
	cout << "                          YOU ARE PLAYING CONNECT FOUR! :)\n" << endl;
	//Vector of colours that the users can choose from
	vector<string> chipColour;
	chipColour.push_back("Blue");
	chipColour.push_back("Green");
	chipColour.push_back("Aqua");
	chipColour.push_back("Red");
	chipColour.push_back("Purple");
	chipColour.push_back("Yellow");
	//Pick a chip piece colour
	for ( int i = 0; i < 10; ++i ) {
		cout << "\n";
	}
	_con.colour(_WHITE);
	cout << "                         Pick a colour for your Chip Piece\n\n";

	_s.send("whatIsTaken");
	string message = _s.recv();
	int removeColour = atoi(message.c_str());
	
	//Loop through the vector
	for(size_t i = 0; i < chipColour.size(); i++) {
		if(i != (size_t)removeColour-1)
		{
			_con.colour((WORD)i + 9);
			cout << "                                    " << chipColour[i] << ": " << i + 1 << endl;
		}
	}
	//Set the colour back to white
	_con.colour(_WHITE);
	string getColour;
	cout << "\n                         Your Colour: ";
	getline(cin, getColour);
	int colourNumber = atoi(getColour.c_str());
	_s.send("pickColour:" + getColour);
	string isValid = _s.recv();
	if(isValid != "valid") {
		system("cls");
		_con.colour(12);
		cout << "\t\t*****Colour is not valid please pick again!*****" << endl;
		_con.colour(_WHITE);
		pickColour();
	}
	else {
		_playerColour = colourNumber+8;
	}
}

/*
 * Purpose: Sets the board state into a vector of vectors. Each vector represents a column number.
 * Parameters: none
 * Return Value: void
 */
void boardSetup() {
	for (int i = 0; i < 7; ++i) {
		_gameState.push_back(vector<int>());
	}
}

/*
 * Purpose: Prints the board game to each player after they have ended their turn
 * Parameters: none
 * Return Value: void
 */
void printGameBoard() {
	system("cls");
	cout << "                          WELCOME TO CONNECT FOUR! :)\n" << endl;
	//Grid walls
	const string fullLine = "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||";
	const string lineBufferTopBottom = "|||||" ;
	const string lineBufferUpperLower = "|||";
	const string lineBufferMiddleEdging = "|";

	//Circle Empty
	const string circleTop		    = ",-''-.";
	const string circleUpper		= "/      \\";
	const string circleMiddle		= "|        |";
	const string circleLower		= "\\      /";
	const string circleBottom		 = "'-..-'";

	//Circle Full
	const string circleTopFilled	 = ",-++-.";
	const string circleUpperFilled	= "/++++++\\";
	const string circleMiddleFilled	= "|++++++++|";
	const string circleLowerFilled	= "\\++++++/";
	const string circleBottomFilled	 = "'-++-'";
	
	_con.colour(3);
	for(int i = 5; i >= 0; --i) {
		cout << fullLine << endl;
		// print Top of circles
		for (int j = 0; j < 7; ++j) {
			cout << lineBufferTopBottom;
			if(_gameState[j].size() <= (size_t)i) {
				cout << circleTop;
			}
			else {
				_con.colour((WORD)_gameState[j][i]);
				cout << circleTopFilled;
				_con.colour(3);
			}
		}
		cout << lineBufferTopBottom << endl;
		// print Upper section of circles
		cout << lineBufferMiddleEdging;
		for (int j = 0; j < 7; ++j) {
			cout << lineBufferUpperLower;
			if(_gameState[j].size() <= (size_t)i) {
				cout << circleUpper;
			}
			else {
				_con.colour((WORD)_gameState[j][i]);
				cout << circleUpperFilled;
				_con.colour(3);
			}
		}
		cout << lineBufferMiddleEdging;
		cout << lineBufferUpperLower << endl;
		// print Top of circles
		cout << lineBufferMiddleEdging;
		cout << lineBufferMiddleEdging;
		for (int j = 0; j < 7; ++j) {
			cout << lineBufferMiddleEdging;
			if(_gameState[j].size() <= (size_t)i) {
				cout << circleMiddle;
			}
			else {
				_con.colour((WORD)_gameState[j][i]);
				cout << circleMiddleFilled;
				_con.colour(3);
			}
		}
		cout << lineBufferUpperLower << endl;
		// print Lower section of circles
		cout << lineBufferMiddleEdging;
		for (int j = 0; j < 7; ++j) {
			cout << lineBufferUpperLower;
			if(_gameState[j].size() <= (size_t)i) {
				cout << circleLower;
			}
			else {
				_con.colour((WORD)_gameState[j][i]);
				cout << circleLowerFilled;
				_con.colour(3);
			}
		}
		cout << lineBufferMiddleEdging;
		cout << lineBufferUpperLower << endl;
		// print Bottom of circles
		for (int j = 0; j < 7; ++j) {
			cout << lineBufferTopBottom;
			if(_gameState[j].size() <= (size_t)i) {
				cout << circleBottom;
			}
			else {
				_con.colour((WORD)_gameState[j][i]);
				cout << circleBottomFilled;
				_con.colour(3);
			}
		}
		cout << lineBufferTopBottom << endl;
	}
	cout << fullLine << endl;
	cout << "	1	   2	      3		 4	    5	       6	  7\n";
	_con.colour(_WHITE);
}

/*
 * Purpose: Game play of connect four; it waits for two players to connect then 
 *			randomly picks which player to start the game. Next it allows the player to choose a 
 *			column on the board. It will update both clients after a player has ended their turn.
 *			It also tells both clients when the game is over, and which player has won.
 * Parameters: none
 * Return Value: void
 */
void playGame() {
	// Show the initial board
	boardSetup();
	printGameBoard();
	_con.colour(_WHITE);
	string instruction;
	string message;
	string value;
	//Wait for player two to connect
	cout << "Waiting for player 2\n";
	_s.send("ready");
	for (;;) {
		message = _s.recv();
		size_t found = message.find(":");
		if( found != message.npos) {
			instruction = message.substr(0, found);
			value = message.substr(found+1);
		}
		else {
			instruction = message;
		}
		//Ask the user to pick a column for their player chip
		if (instruction == "yourTurn") {
			_con.colour((WORD)_playerColour);
			cout << "Pick a Column Number: ";
			_con.colour(_WHITE);
			//Pick a column!
			string pickCol = "";
			getline(cin, pickCol);
			_s.send("play:" + pickCol);
			string msg = _s.recv();
			if ( msg == "valid" ) {
				int column = atoi(pickCol.c_str());
				_gameState[column-1].push_back(_playerColour);
				printGameBoard();
			    _con.colour((WORD)_playerColour);
				cout << "You played column " << pickCol << ".\n";
				_con.colour(_WHITE);
			}
			else {
				//Tell the user their column picked is invalid
				printGameBoard();
				_con.colour(_WHITE);
				cout << "Column Number is invalid!" << endl;
			}
		}
		//Tell the client the which column the opponent played
		else if (instruction == "update") {
			int column = atoi(value.c_str());
			_gameState[column].push_back(_opponentColour);
			printGameBoard();
			_con.colour((WORD)_opponentColour);
			cout << "Opponent played column " << atoi(value.c_str())+1 << ".\n";
			_con.colour(_WHITE);
		}
		//Inform the player you won!
		else if (instruction == "youWin") {
			printGameBoard();
			_con.colour((WORD)_playerColour);
			cout << "Game over you WIN.\n";
			_con.colour(_WHITE);
		}
		//Inform the player you lose!
		else if (instruction == "youLose") {
			printGameBoard();
			_con.colour((WORD)_playerColour);
			cout << "Game over sorry, you lost.\n";
			_con.colour(_WHITE);
		}
		//Tell the client your opponents colour
		else if (instruction == "opponentColour") {
			_opponentColour = atoi(value.c_str()) + 8;
		}
	}
}


int main() {
	//Check for Memory Leaks
#if defined(_DEBUG)
	int dbgFlags = ::_CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	dbgFlags |= _CRTDBG_CHECK_ALWAYS_DF;	// check block integrity on every memory call
	dbgFlags |= _CRTDBG_DELAY_FREE_MEM_DF;	// don't remove block on delete
	dbgFlags |= _CRTDBG_LEAK_CHECK_DF;		// check for mammory leak at process termination
	_CrtSetDbgFlag( dbgFlags );
#endif
	connectToServer();
	pickColour();
	playGame();
}
