/*
 * Name: Katherine Haldane & Jared Lerner
 * Date: April 21, 2014
 * Description: Server validates all interactions the player has with the connect four game board. 
 */
#include <iostream>
#include <map>
#include <SocketLib.hpp>
#include <thread>
#include <vector>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

using namespace std;

//Global Variables
vector <vector<char>> gameState;
Socket s;
map<int, string> playerColour;
int readyCount = 0;
int playerTurn;
int playerWaiting;

/*
 * Purpose: Swap the player turns
 * Parameters: none
 * Return Value: void
 */
void swapTurns() {
	int temp = playerTurn;
	playerTurn = playerWaiting;
	playerWaiting = temp;
}

/*
 * Purpose: Picks a random player to start the game.
 * Parameters: none
 * Return Value: void
 */
void startGame() {
	//Stores the player colour and player ID in a map
	auto it = playerColour.begin();
	playerTurn = it->first;
	++it;
	playerWaiting = it->first;
	// Sends the player colour your opponent chose to the client
	s.send("opponentColour:" + it->second, playerTurn);
	--it;
	s.send("opponentColour:" + it->second, playerWaiting);
	s.send("yourTurn", playerTurn);
}

/*
 * Purpose: Checks the connect four board to check if a player has connected
 *   		four chip horizontally, vertically, or diagonally.
 * Parameters: int
 * Return Value: bool
 */
bool checkForWinner(int vectorNum) {
	int count = 1;
	int rowToCheck = gameState[vectorNum].size() - 1;
	char playedChip = gameState[vectorNum][rowToCheck];

	//Check to see if the same four colours match up vertically
	for(int i = rowToCheck - 1; i >=0; i--)
	{
		if(playedChip == gameState[vectorNum][i]) {
			count++;
		}
		else break;
	}
	if(count == 4){ return true;}

	//Check to see if the same four colours match up horizontally
	count = 1;
	
	for(int i = vectorNum - 1; i >= 0; i--) {
		//check the vector to see if any colours exists to the left of the played chip 
		if(gameState[i].size() > (size_t)rowToCheck && playedChip == gameState[i][rowToCheck]) {
			count++;
		}
		else break;
	}
	//check the vector to see if any colours exists to the right of the played chip 
	for(int i = vectorNum + 1; i < 7; i++) {
		if(gameState[i].size() > (size_t)rowToCheck && playedChip == gameState[i][rowToCheck]) {
			count++;
		}
		else break;
	}
	if(count == 4){ return true;}

	count = 1;

	//Diag --'/'
	int k = vectorNum - 1;
	for(int i = rowToCheck - 1; i >=0 && k >= 0; i--,k--) {
		if(gameState[k].size() > (size_t)i && playedChip == gameState[k][i]) {
			count++;
		}
		else break; 
	}
	// Check to see if the same four colours match up diagonally in this direction:  ++'/'
	k = vectorNum + 1;
	for(int i = rowToCheck + 1; i < 6 && k < 7; i++, k++) {
		if(gameState[k].size() > (size_t)i && playedChip == gameState[k][i]) {
			count++;
		}
		else break;
	}
	if(count == 4){ return true;}

	count = 1;
	//Check to see if the same four colours match up diagonally in this direction: --'\'
	k =vectorNum + 1;
	for(int i = rowToCheck - 1; i >=0 && k < 7; i--,k++) {
		if(gameState[k].size() > (size_t)i && playedChip == gameState[k][i]) {
			count++;
		}
		else break; 
	}
	//Check to see if the same four colours match up diagonally in this direction: ++'\'
	k = vectorNum - 1;
	for(int i = rowToCheck + 1; i < 6 && k >= 0; i++, k--) {
		if(gameState[k].size() > (size_t)i && playedChip == gameState[k][i]) {
			count++;
		}
		else break;
	}
	if(count == 4){ return true;}

	return false;
}

/*
 * Purpose: Save the column number and the user colour to the board state vector.
 *			After it checks to see if a player has won the game.
 * Parameters: int
 * Return Value: void
 */
void play(int column) {
	s.send("valid", playerTurn);
	gameState[column].push_back((char)atoi(playerColour[playerTurn].c_str()));
	s.send("update:" + to_string(column), playerWaiting);
	//Return to each client if they won or lost the game
	if (checkForWinner(column)) { 
		s.send("youWin", playerTurn);
		s.send("youLose", playerWaiting);
	}
	else {
		//Otherwise swap the player turns and inform the client whose turn it is
		swapTurns();
		s.send("yourTurn", playerTurn);
	}
}

/*
 * Purpose: Recieves a message from the client determining if the game is has started,
			the player colour chosen, and the column chosen.
 * Parameters: int
 * Return Value: void
 */
void messageReciever(int id) {
	string instruction;
	string message;
	string value;
	for (;;) {
		//Search the string to find the value to the left of ":"
		message = s.recv(id);
		size_t found = message.find(":");
		if( found != message.npos){
			instruction = message.substr(0, found);
			value = message.substr(found+1);
		}
		else {
			instruction = message;
		}
		//Check if the insturction is "whatIsTaken" and determine what colours have been taken by the users
		if(instruction == "whatIsTaken") {
			if(playerColour.size() != 0) {
				for(auto i = playerColour.begin(); i != playerColour.end(); ++i) {
					s.send(i->second, id);
				}
			}
			else {
				s.send("-1", id);
			}
		}
		//Check if the insturction is "pickColour" and validate if the colour number is valid
		else if (instruction == "pickColour") {
			//Validate if that color exists
			if(value == "1" || value == "2" || value == "3" || value == "4" || value == "5" || value == "6" || value == "7") {
				if(playerColour.size() != 0 && playerColour.begin()->second == value) {
					s.send("notValid", id);
				}
				else{
					playerColour[id] = value;
					s.send("valid", id);
				}
			}
			else {
				s.send("notValid", id);
			}
		}
		////Check if the insturction is "ready", when there is a count of two players start the game
		else if (instruction == "ready") {
			++readyCount;
			if (readyCount == 2) {
				startGame();
			}
		}
		//Check if the insturction is "play". If there column chosen is valid between 1 and 7 check to see if a player has won
		else if (instruction == "play") {
			int column = atoi(value.c_str());
			if ( column >= 1 && column <= 7  && gameState[column-1].size() != 6 ) {
				play(column-1);
			}
			else {
				s.send("invalid", playerTurn);
				s.send("yourTurn", playerTurn);
			}
		}
		else if (instruction == "")
		{
			s.close(id);
			break;
		}
	}
}

/*
 * Purpose: Creates the gameState vector by pushing back seven vectors into the vector.
			Each vector acts as a column on the connect four grid.
 * Parameters: none
 * Return Value: void
 */
void boardSetUp() {
	for (int i = 0; i < 7; ++i) {
		gameState.push_back(vector<char>());
	}
}

int main() {
	//Check for Memory Leaks
#if defined(_DEBUG)
	int dbgFlags = ::_CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	dbgFlags |= _CRTDBG_CHECK_ALWAYS_DF;	// check block integrity on every memory call
	dbgFlags |= _CRTDBG_DELAY_FREE_MEM_DF;	// don't remove block on delete
	dbgFlags |= _CRTDBG_LEAK_CHECK_DF;		// check for memory leak at process termination
	_CrtSetDbgFlag( dbgFlags );
#endif
	boardSetUp();
	// listen for connections, give them each a thread
	s.bind();
	for(;;){
		s.listen();
		std::cout << "Waiting for connection" << std::endl;
		int id = s.accept();
		thread (messageReciever, id).detach();

	}
}
