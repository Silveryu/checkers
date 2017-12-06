#include "game.hpp"
#include <iostream>

using namespace std;

Game::Game()
{
    for (int x = 0; x < 8; ++x)
        for (int y = 0; y < 8; ++y)
            board[x][y] = EMPTY;

    for (int y = 0; y < 3; ++y)
        for (int x = (y + 1) % 2; x < 8; x += 2)
            board[x][y] = BLACK;

    for (int y = 5; y < 8; ++y)
        for (int x = (y + 1) % 2; x < 8; x += 2)
            board[x][y] = WHITE;
}

Cell Game::cell(int x, int y) const
{
    return board[x][y];
}

void Game::print()
{
    for (int x = 0; x < 8; ++x) {
        cout << endl;
        for (int y = 0; y < 8; ++y) {
            cout << board[x][y];
        }
    }
}
