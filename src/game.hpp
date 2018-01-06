#include <utility>
#include <vector>
#define BOARD_SIZE 8

enum Side { BLACK_SIDE = 1, WHITE_SIDE = 2 };
enum Cell { EMPTY, BLACK = BLACK_SIDE, WHITE = WHITE_SIDE };

typedef std::pair<int, int> Position;
typedef std::pair<Position, Position> Step;
typedef std::vector<Step> Move;
typedef Cell Board[BOARD_SIZE][BOARD_SIZE];

class Game
{
    public:
        Game();
        Cell get(int x, int y) const;
        void set_red(int x, int y);
        void set_yellow(int x, int y);
        void print();

    private:
        Board board;
};
