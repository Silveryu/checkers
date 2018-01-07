#include "game.hpp"
#include <iostream>

using namespace std;

Game::Game()
{
    for (int x = 0; x < 8; ++x)
        for (int y = 0; y < 8; ++y)
            board[x][y] = EMPTY;
}

Cell Game::get(int x, int y) const
{
    return board[x][y];
}

void Game::set_red(int x, int y)
{
    board[x][y] = BLACK;
}

void Game::set_yellow(int x, int y)
{
    board[x][y] = WHITE;
}

void Game::print()
{
    for (int y = 0; y < 8; ++y) {
        cout << endl;
        for (int x = 0; x < 8; ++x) {
            cout << board[x][y];
        }
    }
    cout << endl << endl;
}
