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
#include <algorithm>
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

#define ply_depth 6
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

double cornerAdjAnalyze(const OthelloBoard& board, Coin mxCol){
    int x[] = {0, 0, 1, 1, 1, 1,  BOARD_SIZE-2, BOARD_SIZE-2, BOARD_SIZE-2, BOARD_SIZE-2, BOARD_SIZE-1, BOARD_SIZE-1 };
    int y[] = {1, BOARD_SIZE-2, 0, 1, BOARD_SIZE-2, BOARD_SIZE-1, 0, 1, BOARD_SIZE-2, BOARD_SIZE-1, 1, BOARD_SIZE-2};

    double score = 0.0;
    for(int i=0; i < 12; i++){
        Coin token = board.get(x[i], y[i]);
        if(token == mxCol)
            score -= 1.0;
        else if(token == other(mxCol))
            score += 1.0;
    }

    return score/12;
}

double eval(const OthelloBoard board, Coin mxCol){
    // mxCol is the color of the MAX player
    double cp, mob, cc, stb; // coin parity, mobility, corners captured, stability
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

    double cornerAdjScore = cornerAdjAnalyze(board, mxCol);

    double K1 = 10.0, K2;
    double count_goodness;
    bool early_game = (maxCoins + minCoins < 40);
    if( early_game ){
        // give-away in the early game
        count_goodness = K1*(minCoins - maxCoins);
        cp = count_goodness;
    }else{
        // take-back later in the game
        // count_goodness := K2*(maxCoins - minCoins)
    }

//    board.print();
//    printf("cp=%f cc=%f mob=%f\n", cp, cc, mob);
    return (200*cp + 801.724*cc + 78.922*mob + 400*cornerAdjScore);
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
        double h;
        vector< pair<GameState*, Move> > children;

    GameState(OthelloBoard &pboard, Turn pturn): board(pboard)
    {
        turn = pturn;
        h = (double) INT_MIN;
    }
};

bool getChildren(GameState &node)
{
    list<Move> moves = node.board.getValidMoves(node.turn);
    list<Move>::iterator it = moves.begin();
    for(; it != moves.end(); it++) {
        Move mov = *it;
        OthelloBoard childBoard = node.board;
        childBoard.makeMove(node.turn, mov);

        GameState *child = new GameState(childBoard, other(node.turn));
        node.children.push_back(pair<GameState*, Move>(child, mov));
    }

    return node.children.size() ? true : false;
}

double alphaBeta(GameState &node, int depth, double alp, double bet, bool maximizingPlayer) {
    if(depth == 0 || !getChildren(node)) {
        node.h = eval(node.board, maximizingPlayer ? node.turn : other(node.turn));
        return node.h;
    }
    if(maximizingPlayer) {
        double v = (double) INT_MIN;
        for(int i=0; i<node.children.size(); i++) {
            v = max(v, alphaBeta(*node.children[i].first, depth-1, alp, bet, false));
            alp = max(alp, v);
            if(bet <= alp) {
                break;
            }
        }
        node.h = v;
        return node.h;
    } else {
        double v = (double) INT_MAX;
        for(int i=0; i<node.children.size(); i++) {
            v = min(v, alphaBeta(*node.children[i].first, depth-1, alp, bet, true));
            bet = min(bet, v);
            if(bet <= alp) {
                break;
            }
        }
        node.h = v;
        return node.h;
    }
}

Move MyBot::play( const OthelloBoard& board )
{
    OthelloBoard rootBoard = board;
    GameState *root = new GameState(rootBoard, turn);
    double rooth = alphaBeta(*root, ply_depth, (double) INT_MIN, (double) INT_MAX, true);

    Move bMov (0, 0);
    double bBet = (double) INT_MIN;
    for(int i=0; i<root->children.size(); i++) {
        double ch = root->children[i].first->h;
        if(ch > bBet) {
            bBet = ch;
            bMov = root->children[i].second;
        }
    }

    cout << "Best Beta = " << bBet << endl;

    return bMov;
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



