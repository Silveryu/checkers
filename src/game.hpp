#include <utility>
#include <vector>

enum Side { BLACK_SIDE = 1, WHITE_SIDE = 2 };
enum Cell { EMPTY, BLACK = BLACK_SIDE, WHITE = WHITE_SIDE };

typedef std::pair<int, int> Position;
typedef std::pair<Position, Position> Step;
typedef std::vector<Step> Move;
typedef Cell Board[8][8];

class Game
{
    public:
        Game();
        Cell cell(int x, int y) const;
        void print();

    private:
        Board board;
};
