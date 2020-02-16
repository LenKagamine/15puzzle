#include "../include/Board.h"

#include <iomanip>
#include <iostream>
#include <unordered_map>

const std::vector<Direction>& Board::generateMoveList(int x, int y,
                                                      Direction prevMove) {
    if (y == 0) {
        if (x == 0) {
            if (prevMove == Direction::L) return moves[2];  // 2
            if (prevMove == Direction::U) return moves[1];  // 1
            return moves[7];                                // 1, 2
        }
        if (x == WIDTH - 1) {
            if (prevMove == Direction::U) return moves[3];  // 3
            if (prevMove == Direction::R) return moves[2];  // 2
            return moves[9];                                // 2, 3
        }
        if (prevMove == Direction::L) return moves[9];  // 2, 3
        if (prevMove == Direction::U) return moves[8];  // 1, 3
        if (prevMove == Direction::R) return moves[7];  // 1, 2
        return moves[13];                               // 1, 2, 3
    }
    if (y == HEIGHT - 1) {
        if (x == 0) {
            if (prevMove == Direction::D) return moves[1];  // 1
            if (prevMove == Direction::L) return moves[0];  // 0
            return moves[4];                                // 0, 1
        }
        if (x == WIDTH - 1) {
            if (prevMove == Direction::D) return moves[3];  // 3;
            if (prevMove == Direction::R) return moves[0];  // 0
            return moves[6];                                // 0, 3
        }
        if (prevMove == Direction::D) return moves[8];  // 1, 3
        if (prevMove == Direction::L) return moves[6];  // 0, 3
        if (prevMove == Direction::R) return moves[4];  // 0, 1
        return moves[11];                               // 0, 1, 3
    }
    if (x == 0) {
        if (prevMove == Direction::D) return moves[7];  // 1, 2
        if (prevMove == Direction::L) return moves[5];  // 0, 2
        if (prevMove == Direction::U) return moves[4];  // 0, 1
        return moves[10];                               // 0, 1, 2
    }
    if (x == WIDTH - 1) {
        if (prevMove == Direction::D) return moves[9];  // 2, 3
        if (prevMove == Direction::U) return moves[6];  // 0, 3
        if (prevMove == Direction::R) return moves[5];  // 0, 2
        return moves[12];                               // 0, 2, 3
    }
    if (prevMove == Direction::U) return moves[11];  // 0, 1, 3
    if (prevMove == Direction::R) return moves[10];  // 0, 1, 2
    if (prevMove == Direction::D) return moves[13];  // 1, 2, 3
    if (prevMove == Direction::L) return moves[12];  // 0, 2, 3
    return moves[14];                                // 0, 1, 2, 3
}

Board::Board(const std::vector<std::vector<int>>& g, const DisjointDatabase& d)
    : blank(0), grid(0), database(d), WIDTH(g[0].size()), HEIGHT(g.size()) {
    uint64_t positions = 0;
    for (int y = HEIGHT - 1; y >= 0; y--) {
        for (int x = WIDTH - 1; x >= 0; x--) {
            positions |= (uint64_t)(y * WIDTH + x) << (4 * g[y][x]);
            grid = grid * 16 + g[y][x];

            if (g[y][x] == 0) blank = y * WIDTH + x;
        }
    }

    // x, y, prevMove, moves[]
    moveList = std::vector<std::vector<std::vector<Direction>>>(
        WIDTH * HEIGHT, std::vector<std::vector<Direction>>(5));

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            for (int m = 0; m < 4; m++) {
                moveList[y * WIDTH + x][m] =
                    generateMoveList(x, y, static_cast<Direction>(m));
            }
        }
    }

    // {0, 0}, {0, -1}, {1, 0}, {0, 1}, {-1, 0}};
    deltas = std::vector<int>{-4 * WIDTH, 4, 4 * WIDTH, -4};

    // Calculate deltas
    auto numPatterns = database.numPatterns();
    const auto& where = database.where;

    // The tiles in each pattern
    std::vector<std::vector<int>> patternTiles(numPatterns);
    for (int i = 1; i < WIDTH * HEIGHT; i++) {  // Ignore blank tile (0)
        patternTiles[where[i]].push_back(i);
    }

    tileDeltas.resize(WIDTH * HEIGHT, 1);
    patterns.resize(numPatterns, 0);

    for (int i = 0; i < numPatterns; i++) {
        // Calculate tileDeltas
        auto& tiles = patternTiles[i];
        for (int j = tiles.size() - 2; j >= 0; j--) {
            tileDeltas[tiles[j]] =
                tileDeltas[tiles[j + 1]] * (WIDTH * HEIGHT - 1 - j);
        }

        // Calculate pattern
        std::vector<uint> startPos(WIDTH * HEIGHT, 0);
        std::unordered_map<int, int> before;
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                if (where[g[y][x]] == i) {
                    // New tile found
                    uint beforeCount = 0;

                    // Count number of preceding pattern tiles that's smaller
                    for (auto& it : before) {
                        if (it.first < g[y][x]) {
                            beforeCount++;
                        }
                    }

                    before[g[y][x]] = beforeCount;
                    startPos[g[y][x]] = y * WIDTH + x;
                }
            }
        }
        uint j = WIDTH * HEIGHT;
        for (auto tile : tiles) {
            patterns[i] *= j--;
            patterns[i] += startPos[tile] - before[tile];
        }
    }
}

int Board::getHeuristic() const { return database.getHeuristic(patterns); }

const std::vector<Direction>& Board::getMoves() const {
    // Should be run only once at start of search
    if (blank < WIDTH) {           // top
        if (blank % WIDTH == 0) {  // left
            return moves[7];
        }
        if (blank % WIDTH == WIDTH - 1) {  // right
            return moves[9];
        }
        return moves[13];
    }
    if (blank >= (WIDTH - 1) * HEIGHT) {  // bottom
        if (blank % WIDTH == 0) {         // left
            return moves[4];
        }
        if (blank % WIDTH == WIDTH - 1) {  // right
            return moves[6];
        }
        return moves[11];
    }
    if (blank % WIDTH == 0) {  // left
        return moves[10];
    }
    if (blank % WIDTH == WIDTH - 1) {  // right
        return moves[12];
    }

    return moves[14];
}

const std::vector<Direction>& Board::getMoves(Direction prevMove) const {
    // Branchless lookup for moves
    return moveList[blank][static_cast<int>(prevMove)];
}

inline int Board::getTile(int posn) {
    return (grid & (0xfULL << posn)) >> posn;
}

inline void Board::setTile(int posn, int tile) {
    grid = (grid & ~(0xfULL << posn)) | ((uint64_t)tile << posn);
}

int Board::applyMove(Direction dir) {
    // Position of blank
    const auto& delta = deltas[static_cast<int>(dir)];
    const int blank = 4 * this->blank;
    // Position of sliding tile
    const int slidingPos = blank + delta;
    // Value of sliding tile
    const int tile = getTile(slidingPos);

    // Set value of slid tile
    setTile(blank, tile);

    // Update pattern
    int index = database.where[tile];  // Which pattern the sliding tile is in

    switch (dir) {
        case Direction::U: {
            int numGreater = 0;
            int numBlanks = 1;
            int skipDelta = 0;
            for (int i = this->blank - WIDTH + 1; i < this->blank; i++) {
                int skip = getTile(i * 4);
                if (database.where[skip] != index) {
                    numBlanks++;
                } else if (skip > tile) {
                    numGreater++;
                    skipDelta += tileDeltas[skip];
                }
            }
            patterns[index] +=
                skipDelta + (numGreater + numBlanks) * tileDeltas[tile];
            break;
        }
        case Direction::R:
            patterns[index] -= tileDeltas[tile];
            break;
        case Direction::D: {
            int numGreater = 0;
            int numBlanks = 1;
            int skipDelta = 0;
            for (int i = this->blank + 1; i < this->blank + WIDTH; i++) {
                int skip = getTile(i * 4);
                if (database.where[skip] != index) {
                    numBlanks++;
                } else if (skip > tile) {
                    numGreater++;
                    skipDelta += tileDeltas[skip];
                }
            }
            patterns[index] -=
                skipDelta + (numGreater + numBlanks) * tileDeltas[tile];
            break;
        }
        default:
            patterns[index] += tileDeltas[tile];
            break;
    }

    // Update blank tile
    this->blank += delta / 4;

    return 1;
}

void Board::undoMove(Direction dir) {
    // Position of blank
    const auto& delta = deltas[static_cast<int>(dir)];
    const int blank = 4 * this->blank;
    // Position of sliding tile
    const int slidingPos = blank - delta;
    // Value of sliding tile
    const int tile = getTile(slidingPos);

    // Set value of slid tile
    setTile(blank, tile);

    // Update partial position
    int index = database.where[tile];  // Which pattern the sliding tile is in

    switch (dir) {
        case Direction::U: {
            int numGreater = 0;
            int numBlanks = 1;
            int skipDelta = 0;
            for (int i = this->blank + 1; i < this->blank + WIDTH; i++) {
                int skip = getTile(i * 4);
                if (database.where[skip] != index) {
                    numBlanks++;
                } else if (skip > tile) {
                    numGreater++;
                    skipDelta += tileDeltas[skip];
                }
            }
            patterns[index] -=
                skipDelta + (numGreater + numBlanks) * tileDeltas[tile];

            break;
        }
        case Direction::R:
            patterns[index] += tileDeltas[tile];
            break;
        case Direction::D: {
            int numGreater = 0;
            int numBlanks = 1;
            int skipDelta = 0;
            for (int i = this->blank - WIDTH + 1; i < this->blank; i++) {
                int skip = getTile(i * 4);
                if (database.where[skip] != index) {
                    numBlanks++;
                } else if (skip > tile) {
                    numGreater++;
                    skipDelta += tileDeltas[skip];
                }
            }
            patterns[index] +=
                skipDelta + (numGreater + numBlanks) * tileDeltas[tile];
            break;
        }
        default:
            patterns[index] -= tileDeltas[tile];
            break;
    }

    // Update blank tile
    this->blank -= delta / 4;
}

Board::~Board() = default;

std::ostream& operator<<(std::ostream& out, const Board& board) {
    for (int y = 0; y < board.HEIGHT; y++) {
        for (int x = 0; x < board.WIDTH; x++) {
            const int i = y * board.WIDTH + x;
            if (i == board.blank) {
                out << std::setw(3) << 0;
            } else {
                out << std::setw(3)
                    << ((board.grid & (0xfULL << (4 * i))) >> (4 * i));
            }
        }
        out << std::endl;
    }
    return out;
}
