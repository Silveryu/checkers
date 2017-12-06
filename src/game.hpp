#include <utility>
#include <vector>

typedef std::pair<int, int> Position;
typedef std::pair<Position, Position> Step;
typedef std::vector<Step> Move;

class Game
{
    public:
        enum Side { BLACK_SIDE = 1, WHITE_SIDE = 2 };
        enum Cell { EMPTY, BLACK = BLACK_SIDE, WHITE = WHITE_SIDE };
        Game();
        Cell cell(int x, int y) const;
        void print();

    private:
        typedef Cell Board[8][8];
        Board board;
        int score() const;
};
