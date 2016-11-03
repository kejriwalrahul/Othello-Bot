/*
* @file botTemplate.cpp
* @author Arun Tejasvi Chaganty <arunchaganty@gmail.com>
* @date 2010-02-04
* Template for users to create their own bots
*/

#include "Othello.h"
#include "OthelloBoard.h"
#include "OthelloPlayer.h"
#include <cstdlib>
#include <climits>
using namespace std;
using namespace Desdemona;

class MyBot: public OthelloPlayer
{
    public:
        /**
         * Initialisation routines here
         * This could do anything from open up a cache of "best moves" to
         * spawning a background processing thread. 
         */
        MyBot( Turn turn );

        /**
         * Play something 
         */
        virtual Move play( const OthelloBoard& board );
    private:
};

MyBot::MyBot( Turn turn )
    : OthelloPlayer( turn )
{
}

#define ply_depth 5

int eval(const OthelloBoard board, Move m, Turn turn){
    board.makeMove(m);
    if(turn == RED)
        return board.getRedCount() - board.getBlackCount();
    else
        return board.getBlackCount() - board.getRedCount();
}

int eval(const OthelloBoard board, Turn turn){
    if(turn == RED)
        return board.getRedCount() - board.getBlackCount();
    else
        return board.getBlackCount() - board.getRedCount();    
}

class CompareDist{
    public:
        bool operator()(GameState n1, GameState n2) {
            return n1.h - n2.h;
        }
};

int min(int a, int b){
    if(a<b) return a;
    else    return b;    
}

int getOppTurn(Turn turn){
    if(turn == RED) return BLACK;
    else            return RED;
}

enum status{
    LIVE,
    SOLVED
}

class GameState{
    public:
        OthelloBoard board;
        Turn turn;
        int depth;
        int status;
        int h;

    GameState(OthelloBoard pboard, Turn pturn, int pdepth, int pstatus, int ph){
        board  = pboard;
        turn   = pturn;
        depth  = pdepth;
        status = pstatus;
        h      = ph;
    }

    GameState(OthelloBoard pboard, Turn pturn){
        board  = pboard;
        turn   = pturn;
        depth  = -1;
        status = -1;
        h      = -1;
    }
}

typedef GameState TreeNode;

Move MyBot::play( const OthelloBoard& board )
{
    list<Move> moves = board.getValidMoves( turn );
    /**
     *   int randNo = rand() % moves.size();
     *   list<Move>::iterator it = moves.begin();
     */

    /**
     * Tuple contains:
     *      Board State
     *      Turn - RED or BLACK
     *      int  - depth below search node
     *      int  - live=0 or solved=1
     *      int  - eval fn val
     */

    priority_queue<GameState, vector<GameState>, CompareDist> open;
    open.push(GameState(board, turn, 0, 0, INT_MAX));

    while(true){
        
        GameState &current = open.pop();
        
        if(current.board == board && current.status == SOLVED) {
            // retrieve best move
            break;
        } else {
            // Process the current state
            OthelloBoard &currBoard = current.board;
            Turn &currTurn = current.turn;
               
            if(current.status == 0){
                // Downward Pass
                if(current.depth >= ply_depth){
                    open.push(GameState(currBoard, currTurn, current.depth, 1, min(current.h, eval(currBoard, currTurn)) ));
                }
                else if(current.turn != turn){
                    list<Move> currMoves = currBoard.getValidMoves(currTurn);
                    currBoard.makeMove(*(currMoves.begin()));
                    open.push(GameState(currBoard, getOppTurn(currTurn), current.depth+1, 0, current.h));
                }
                else{
                    list<Move> currMoves = currBoard.getValidMoves(currTurn);
                    list<Move>::iterator it = moves.begin();
                    for(; it != currMoves.end(); it++){
                        OthelloBoard newBoard = currBoard;
                        newBoard.makeMove(*it);
                        open.push(GameState(newBoard, getOppTurn(currTurn), current.depth+1, 0, current.h));       
                    }   
                }
            }
            else{
                // Upward Pass
                if(currTurn != turn){
                    tuple<OthelloBoard, turn> = parent()
                }
                else if(){}
            }
        }
    }
    return *it;
}

// The following lines are _very_ important to create a bot module for Desdemona

extern "C" {
    OthelloPlayer* createBot( Turn turn )
    {
        return new MyBot( turn );
    }

    void destroyBot( OthelloPlayer* bot )
    {
        delete bot;
    }
}


