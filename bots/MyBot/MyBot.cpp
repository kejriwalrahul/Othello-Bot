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
#include <queue>
#include <vector>
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
#define BOARD_SIZE 8

// Returns the number of coins of color "color" at corners
int corners(const OthelloBoard &board, Coin color) {
    int ans = 0;
    int x[4] = {0, 0, BOARD_SIZE-1, BOARD_SIZE-1};
    int y[4] = {0, BOARD_SIZE-1, BOARD_SIZE-1, 0};

    for(int i=0; i<4; i++) {
        if(board.get(x[i], y[i]) == color) {
            ans += 1;
        }
    }
    return ans;
}

bool border(int x, int y) {
    if(y==0 || y==BOARD_SIZE-1) {
        if(x>1 && x<BOARD_SIZE-1-1) {
            return true;
        }
    }
    if(x==0 || x==BOARD_SIZE-1) {
        if(y>1 && y<BOARD_SIZE-1-1) {
            return true;
        }
    }
    return false;
}

int eval(const OthelloBoard board, Coin mxCol){
    // turn is the color of the MAX player
    float cp, mob, cc, stb; // coin parity, mobility, corners captured, stability
    cp=mob=cc=stb=0;
    int maxCoins, minCoins;
    int maxMoves, minMoves;
    int maxCorners, minCorners;
    Coin mnCol  = other(mxCol);

    maxMoves = board.getValidMoves(mxCol).size();
    minMoves = board.getValidMoves(mnCol).size();

    maxCorners = corners(board, mxCol);
    minCorners = corners(board, mnCol);

    if(mxCol == RED) {
        maxCoins = board.getRedCount();
        minCoins = board.getBlackCount();

    } else {
        maxCoins = board.getBlackCount();
        minCoins = board.getRedCount();
    }

    cp = 100.0*(maxCoins-minCoins)/(maxCoins+minCoins+0.0);

    if(maxMoves + minMoves != 0) {
        mob = 100.0*(maxMoves-minMoves)/(maxMoves+minMoves+0.0);
    } else {
        mob = 0;
    }

    if(maxCorners+minCorners != 0) {
        cc = 100.0*(maxCorners-minCorners)/(maxCorners+minCorners+0.0);
    } else {
        cc = 0;
    }

    return (int) (10*cp + 801.724*cc + 78.922*mob);
}

int min(int a, int b){
    if(a<b) return a;
    else    return b;
}

enum status{
    LIVE,
    SOLVED
};

class GameState{
    public:
        OthelloBoard &board;
        Turn turn;
        int depth;
        int status;
        int h;
        GameState* parent;
        int parentsChildIndex;
        vector<GameState*> children;

    GameState(OthelloBoard &pboard, Turn pturn, int pdepth, int pstatus, int ph, GameState *pparent, int index): board(pboard)
    {
        turn   = pturn;
        depth  = pdepth;
        status = pstatus;
        h      = ph;
        parent = pparent;
        parentsChildIndex = index;
    }
};

class CompareDist{
    public:
        bool operator()(GameState* n1, GameState* n2) {
            return n1->h - n2->h;
        }
};

typedef GameState TreeNode;

template<typename T>
class custom_priority_queue : public priority_queue<T, vector<T>>
{
  public:
    bool remove(const T& value) {
        auto it = find(this->c.begin(), this->c.end(), value);
        if (it != this->c.end()) {
            this->c.erase(it);
            make_heap(this->c.begin(), this->c.end(), this->comp);
            return true;
        }
        else {
        return false;
        }
    }
};

void recursiveRemoveFromPQ(GameState* node, custom_priority_queue& open){
    for(int i= 0; i<node->children.size(); i++)
        recursiveRemoveFromPQ(node->children[i]);
    open.remove(node);
}

Move MyBot::play( const OthelloBoard& board )
{
    OthelloBoard originalBoard = board;

    custom_priority_queue<GameState*, vector<GameState*>, CompareDist> open;
    open.push(new GameState(originalBoard, turn, 0, 0, INT_MAX, NULL, 0));

    while(true){

        GameState *current = open.top();
        open.pop();

        if(current->depth == 0 && current->status == SOLVED) {
            // retrieve best move: returned in (4.)
            // if this is reached, root should have been a terminal node
            // Return a random move, just for safety.
            list<Move> moves = current->board.getValidMoves(turn);
            if(!moves.size() == 0) {
                return *moves.begin();
            }
            // return best move
            break;
        } else {
            // Process the current state
            OthelloBoard &currBoard = current->board;
            Turn &currTurn = current->turn;

            if(current->status == 0){
                list<Move> moves = current->board.getValidMoves(currTurn);
                // Downward Pass
                if(current->depth >= ply_depth || moves.size() == 0){
                    GameState *next = current;
                    next->status = SOLVED;
//                    next->h = min(current->h, eval(currBoard, currTurn));
                    next->h = min(current->h, eval(currBoard, turn)); // evaluate current board position wrt MAX

                    open.push(next);
                    // current->children.push_back(next); Do we need this?
                }
                else if(current->turn != turn){
                    OthelloBoard newBoard  = currBoard;
                    list<Move>   currMoves = newBoard.getValidMoves(currTurn);
                    newBoard.makeMove(current->turn, *(currMoves.begin()));

                    GameState *next = new GameState(newBoard, other(currTurn), current->depth+1, LIVE, current->h, current, 0);
                    open.push(next);
                    current->children.push_back(next);

                    // Fill up the children vector of the current min node, so that it can be accessed later
                    // (when a child is solved)
                    list<Move>::iterator it = currMoves.begin();
                    it++;
                    for(int cnt = 1; it != currMoves.end(); it++, cnt++) {
                        OthelloBoard newBoard = currBoard;
                        newBoard.makeMove(current->turn, *it);

                        GameState *next = new GameState(newBoard, other(currTurn), current->depth+1, LIVE, current->h, current, cnt);
                        current->children.push_back(next);
                    }
                }
                else{
                    list<Move> currMoves = currBoard.getValidMoves(currTurn);
                    list<Move>::iterator it = currMoves.begin();
                    for(int cnt = 0; it != currMoves.end(); it++, cnt++){
                        OthelloBoard newBoard = currBoard;
                        newBoard.makeMove(current->turn, *it);

                        GameState *next = new GameState(newBoard, other(currTurn), current->depth+1, LIVE, current->h, current, cnt);
                        open.push(next);
                        current->children.push_back(next);
                    }
                }
            }
            else{
                // Upward Pass
                if(currTurn != turn){
                   GameState *next = current->parent;
                   next->status = SOLVED;
                   next->h      = current->h;


                   if(current->depth == 1) { // Is a child node of root
                        OthelloBoard &parentBoard = current->parent->board;
                        list<Move> moves = parentBoard.getValidMoves(turn);
                        list<Move>::iterator it = moves.begin();
                        for(int i=0; it != moves.end(); it++, i++) {
                            if(i == current->parentsChildIndex) {
                                return *it;
                            }
                        }
                   }

                   open.push(next);
                   recursiveRemoveFromPQ(next, open);
                }
                else if(current->parent->children[current->parent->children.size()-1] == current){
                   GameState *next = current->parent;
                   next->status = SOLVED;
                   next->h      = current->h;

                   open.push(next);
                }
                else{
                   GameState *next = current->parent->children[current->parentsChildIndex + 1];
                   next->status = LIVE;
                   next->h      = current->h;

                   open.push(next);
                }
            }
        }

        /*
            PseudoCode:
               if s == L
                   if J is a terminal node
                       (1.) add (J,S,min(h,value(J))) to OPEN
                   else if J is a MIN node
                       (2.) add (J.1,L,h) to OPEN
                   else
                       (3.) for j=1..number_of_children(J) add (J.j,L,h) to OPEN
               else
                   if J is a MIN node
                       (4.) add (parent(J),S,h) to OPEN
                            remove from OPEN all the states that are associated with the children of parent(J)
                   else if is_last_child(J)   // if J is the last child of parent(J)
                       (5.) add (parent(J),S,h) to OPEN
                   else
                       (6.) add (parent(J).(k+1),L,h) to OPEN   // add state associated with the next child of parent(J) to OPEN
        */
    }
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


