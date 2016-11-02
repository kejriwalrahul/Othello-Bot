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
        bool operator()(tuple<OthelloBoard,Turn,int, int,int> n1,tuple<OthelloBoard,Turn,int,int,int> n2) {
            return get<3>(n1) - get<3>(n2);
        }
};

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

int min(int a, int b){
    if(a<b) return a;
    else    return b;    
}

int getOppTurn(Turn turn){
    if(turn == RED) return BLACK;
    else            return RED;
}

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

    priority_queue<tuple<OthelloBoard,Turn,int,int,int>, vector<tuple<OthelloBoard,Turn,int,int,int>>, CompareDist> open;
    open.push(make_tuple(board, turn, 0, 0, INT_MAX));

    while(true){
        
        tuple<OthelloBoard,Turn,int,int,int> current = open.pop();
        
        if(get<0>(current) == board && get<3>(current) == 1) {
            // retrieve best move
            break;
        } else {
            // Process the current state
            OthelloBoard &currBoard = get<0>(current);
            Turn &currTurn = get<1>(current);
               
            if(get<3>(current) == 0){
                // Downward Pass
                if(get<2>(current) >= ply_depth){
                    open.push(make_tuple(currBoard, currTurn, get<2>(current), 1, min(get<4>(current), eval(currBoard, currTurn)) ));
                }
                else if(get<1>(current) != turn){
                    list<Move> currMoves = currBoard.getValidMoves(currTurn);
                    currBoard.makeMove(*(currMoves.begin()));
                    open.push(make_tuple(currBoard, getOppTurn(currTurn), get<2>(current)+1, 0, get<4>(current)));
                }
                else{
                    list<Move> currMoves = currBoard.getValidMoves(currTurn);
                    list<Move>::iterator it = moves.begin();
                    for(; it != currMoves.end(); it++){
                        OthelloBoard newBoard = currBoard;
                        newBoard.makeMove(*it);
                        open.push(make_tuple(newBoard, getOppTurn(currTurn), get<2>(current)+1, 0, get<4>(current)));       
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


