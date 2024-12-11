// OVERALL UPGRADE IDEAS https://discord.com/channels/435943710472011776/882956631514689597/1256706716515565638
// Move generation upgrade https://github.com/nkarve/surge/blob/master/src/position.h
// Issue: r2q1knr/5p2/p7/1ppQ1b2/5P1b/2NPp3/PPP3PP/R3KB1R w KQ - 2 16
// Move d5c5

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <deque>
#include <map>
#include <climits>
#include <limits>
#include <chrono>
#include <array>
#include <thread>
#include <atomic>
#include <optional>
#include <random>
#include <unordered_map>
#include <bitset>

#define ctzll(x) ((x) ? _tzcnt_u64(x) : 64)
#define clzll(x) ((x) ? __lzcnt64(x) : 64)
#define popcountll(x) __popcnt64(x)

//#define ctzll(x) ((x) ? __builtin_ctzll(x) : 64)
//#define clzll(x) ((x) ? __builtin_clzll(x) : 64)
//#define popcountll(x) __builtin_popcountll(x)

typedef uint64_t u64;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint8_t u8;

constexpr int NEG_INF = -std::numeric_limits<int>::max();
constexpr int POS_INF = std::numeric_limits<int>::max();

constexpr int EN_PASSANT = 0;
constexpr int CASTLING = 1;


enum Color : int {
    WHITE = 1, BLACK = 0
};

//Inverts the color (WHITE -> BLACK) and (BLACK -> WHITE)
constexpr Color operator~(Color c) {
    return Color(c ^ 1);
}

enum PieceType : int {
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_PIECE_TYPE
};

enum Square : int {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,
    NO_SQUARE
};

enum Direction : int {
    NORTH = 8, NORTH_EAST = 9, EAST = 1, SOUTH_EAST = -7,
    SOUTH = -8, SOUTH_WEST = -9, WEST = -1, NORTH_WEST = 7,
    NORTH_NORTH = 16, SOUTH_SOUTH = -16
};

enum File : int {
    AFILE, BFILE, CFILE, DFILE, EFILE, FFILE, GFILE, HFILE
};

enum Rank : int {
    RANK1, RANK2, RANK3, RANK4, RANK5, RANK6, RANK7, RANK8
};

inline Square& operator++(Square& s) { return s = Square(int(s) + 1); }
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
inline Square& operator+=(Square& s, Direction d) { return s = s + d; }
inline Square& operator-=(Square& s, Direction d) { return s = s - d; }

struct shifts {
    static inline int NORTH = 8;
    static inline int NORTH_EAST = 9;
    static inline int EAST = 1;
    static inline int SOUTH_EAST = -7;
    static inline int SOUTH = -8;
    static inline int SOUTH_WEST = -9;
    static inline int WEST = -1;
    static inline int NORTH_WEST = 7;

    static inline std::array<int, 8> dirs = { NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST };
    static inline std::array<int, 4> straightDirs = { NORTH, EAST, SOUTH, WEST };
    static inline std::array<int, 4> diagonalDirs = { NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };
};

struct indexes {
    static inline int NORTH = 0;
    static inline int NORTH_EAST = 1;
    static inline int EAST = 2;
    static inline int SOUTH_EAST = 3;
    static inline int SOUTH = 4;
    static inline int SOUTH_WEST = 5;
    static inline int WEST = 6;
    static inline int NORTH_WEST = 7;
    static inline int all = 8;

    static inline std::array<int, 8> dirs = { NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST };
    static inline std::array<int, 4> straightDirs = { NORTH, EAST, SOUTH, WEST };
    static inline std::array<int, 4> diagonalDirs = { NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };
};

static inline int parseSquare(const std::string& square) {
    return (square.at(1) - '1') * 8 + (square.at(0) - 'a'); // Calculate the index of any square
}

template <typename BitboardType>
static inline bool readBit(BitboardType bitboard, int index) {
    return (bitboard & (1ULL << index)) != 0;
}

template <typename BitboardType>
static inline void setBit(BitboardType& bitboard, int index, bool value) {
    if (value) bitboard |= (1ULL << index);
    else bitboard &= ~(1ULL << index);
}

class Board;


class Move {
public:
    uint16_t move;
    // Queen *14*, rook *13*, bishop/knight *12*, ending square *6-11*, starting square *0-5* 
    Move() {
        move = 0;
    }

    Move(std::string in) {
        if (in.size() == 4) *this = Move(parseSquare(in.substr(0, 2)), parseSquare(in.substr(2, 2)));
        else {
            int promo = 0;
            if (in.at(4) == 'q') promo = 4;
            else if (in.at(4) == 'r') promo = 3;
            else if (in.at(4) == 'b') promo = 2;
            *this = Move(parseSquare(in.substr(0, 2)), parseSquare(in.substr(2, 2)), promo);
        }
    }

    Move(u8 startSquare, u8 endSquare, u8 promotion = 0) {
        startSquare &= 0b111111; // Make sure input is only 6 bits
        endSquare &= 0b111111;
        move = startSquare;
        move |= endSquare << 6;

        if (promotion) {
            if (promotion == 4) setBit(move, 14, 1);
            else if (promotion == 3) setBit(move, 13, 1);
            else if (promotion == 2) setBit(move, 12, 1);
        }
    }

    std::string toString(Board& board);

    inline int startSquare() { return move & 0b111111; }
    inline int endSquare() { return (move >> 6) & 0b111111; }
    inline int promo() {
        if (readBit(move, 14)) return 4;
        else if (readBit(move, 13)) return 3;
        else if (readBit(move, 12)) return 2;
        return 1;
    }
};

std::deque<std::string> split(const std::string& s, char delim) {
    std::deque<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

int findIndexOf(const std::deque<std::string>& deque, std::string entry) {
    auto it = std::find(deque.begin(), deque.end(), entry);
    if (it != deque.end()) {
        return std::distance(deque.begin(), it); // Calculate the index
    }
    return -1; // Not found
}

void printBitboard(u64 bitboard) {
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << "+---+---+---+---+---+---+---+---+" << std::endl;
        for (int file = 0; file < 8; file++) {
            int i = rank * 8 + file;  // Map rank and file to bitboard index
            char currentPiece = readBit(bitboard, i) ? '1' : ' ';

            std::cout << "| " << currentPiece << " ";
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "+---+---+---+---+---+---+---+---+" << std::endl;
}

std::string formatNum(u64 v) {
    auto s = std::to_string(v);

    int n = s.length() - 3;
    while (n > 0) {
        s.insert(n, ",");
        n -= 3;
    }
    return s;
}

class Precomputed {
public:
    static std::array<u64, 64> knightMoves;
    static std::array<u64, 64> kingMoves;
    static std::array<std::array<u64, 64>, 12> zobrist;
    static std::array<std::array<u64, 9>, 64> rays;
    static std::array<u64, 64> rookAttack;
    static std::array<u64, 64> bishopAttack;
    static u64 isOnA;
    static u64 isOnB;
    static u64 isOnC;
    static u64 isOnD;
    static u64 isOnE;
    static u64 isOnF;
    static u64 isOnG;
    static u64 isOnH;
    static u64 isOn1;
    static u64 isOn2;
    static u64 isOn3;
    static u64 isOn4;
    static u64 isOn5;
    static u64 isOn6;
    static u64 isOn7;
    static u64 isOn8;
    static inline const std::array<int, 64> white_pawn_table = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 10, 20, 20, 10, 0, 0,
        0, 0, 10, 20, 20, 10, 0, 0,
        0, 0, 5, 10, 10, 5, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    static inline const std::array<int, 64> white_knight_table = {
        -50, -40, -40, -40, -40, -40, -40, -50,
        -40, 20, 0, 0, 0, 0, -20, -40,
        -40, 0, 10, 20, 20, 10, 0, -40,
        -40, 0, 0, 25, 25, 0, 0, -40,
        -40, 0, 0, 25, 25, 0, 0, -40,
        -40, 0, 10, 20, 20, 10, 0, -40,
        -40, -20, 0, 0, 0, 0, -20, -40,
        -50, -40, -40, -40, -40, -40, -40, -50
    };

    static inline const std::array<int, 64> white_bishop_table = {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -20,-10,-10,-10,-10,-10,-10,-20
    };

    static inline const std::array<int, 64> white_rook_table = {
         0,  0,  0,  0,  0,  0,  0,  0,
         5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
         0,  0,  0,  5,  5,  0,  0,  0
    };

    static inline const std::array<int, 64> white_queen_table = {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -10,  5,  5,  5,  5,  5,  0,-10,
          0,  0,  5,  5,  5,  5,  0, -5,
         -5,  0,  5,  5,  5,  5,  0, -5,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    };

    static inline const std::array<int, 64> white_king_table = {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
         20, 20,  0,  0,  0,  0, 20, 20,
         20, 30, 10,  0,  0, 10, 30, 20
    };

    static void compute() {
        // *** FILE AND COL ARRAYS ***
        isOnA = 0;
        isOnB = 0;
        isOnC = 0;
        isOnD = 0;
        isOnE = 0;
        isOnF = 0;
        isOnG = 0;
        isOnH = 0;
        isOn1 = 0;
        isOn2 = 0;
        isOn3 = 0;
        isOn4 = 0;
        isOn5 = 0;
        isOn6 = 0;
        isOn7 = 0;
        isOn8 = 0;

        for (int i = 0; i < 64; i++) {
            int file = i % 8; // File index (0 = A, 1 = B, ..., 7 = H)
            if (file == 0) isOnA |= 1ULL << i;
            if (file == 1) isOnB |= 1ULL << i;
            if (file == 2) isOnC |= 1ULL << i;
            if (file == 3) isOnD |= 1ULL << i;
            if (file == 4) isOnE |= 1ULL << i;
            if (file == 5) isOnF |= 1ULL << i;
            if (file == 6) isOnG |= 1ULL << i;
            if (file == 7) isOnH |= 1ULL << i;

            // Fill ranks (1-8)
            int rank = i / 8; // Rank index (0 = 1, 1 = 2, ..., 7 = 8)
            if (rank == 0) isOn1 |= 1ULL << i;
            if (rank == 1) isOn2 |= 1ULL << i;
            if (rank == 2) isOn3 |= 1ULL << i;
            if (rank == 3) isOn4 |= 1ULL << i;
            if (rank == 4) isOn5 |= 1ULL << i;
            if (rank == 5) isOn6 |= 1ULL << i;
            if (rank == 6) isOn7 |= 1ULL << i;
            if (rank == 7) isOn8 |= 1ULL << i;
        }

        // *** MOVE GENERATION ***

        for (int i = 0; i < 64; i++) {
            u64 all = 0;
            for (int dir : indexes::dirs) {
                u64 ray = 0;
                int currentIndex = i;

                while (true) {
                    // Break if wrapping across ranks (for EAST/WEST)
                    if ((dir == indexes::EAST || dir == indexes::NORTH_EAST || dir == indexes::SOUTH_EAST) && (isOnH & (1ULL << currentIndex))) break;
                    if ((dir == indexes::WEST || dir == indexes::NORTH_WEST || dir == indexes::SOUTH_WEST) && (isOnA & (1ULL << currentIndex))) break;

                    currentIndex += shifts::dirs[dir];

                    // Break if out of bounds
                    if (currentIndex < 0 || currentIndex >= 64) break;

                    setBit(ray, currentIndex, 1);

                }
                rays[i][dir] = ray;
                all |= ray;
            }
            rays[i][8] = all;
        }

        // *** ROOK MOVES ***

        for (int i = 0; i < 64; i++) {
            u64 all = 0;
            for (int dir : indexes::straightDirs) {
                u64 ray = 0;
                int currentIndex = i;

                while (true) {// Break if wrapping across ranks (for EAST/WEST)
                    if ((dir == indexes::EAST) && ((isOnG | isOnH) & (1ULL << currentIndex))) break;
                    if ((dir == indexes::WEST) && ((isOnA | isOnB) & (1ULL << currentIndex))) break;

                    currentIndex += shifts::dirs[dir];

                    // Break if out of bounds
                    if (currentIndex < 0 || currentIndex >= 64) break;

                    if ((shifts::dirs[dir] > 1 || shifts::dirs[dir] < -1) && (currentIndex < 8 || currentIndex >= 56)) break; // Shift is not horizontal

                    setBit(ray, currentIndex, 1);
                }
                all |= ray;
            }
            rookAttack[i] = all;
        }


        // *** BISHOP MOVES ***

        for (int i = 0; i < 64; i++) {
            u64 all = 0;
            for (int dir : indexes::diagonalDirs) {
                u64 ray = 0;
                int currentIndex = i;

                while (true) {// Break if wrapping across ranks (for EAST/WEST)
                    if ((dir == indexes::NORTH_EAST || dir == indexes::SOUTH_EAST) && ((isOnG | isOnH) & (1ULL << currentIndex))) break;
                    if ((dir == indexes::NORTH_WEST || dir == indexes::SOUTH_WEST) && ((isOnA | isOnB) & (1ULL << currentIndex))) break;

                    currentIndex += shifts::dirs[dir];

                    // Break if out of bounds
                    if (currentIndex < 0 || currentIndex >= 64) break;

                    if ((shifts::dirs[dir] > 1 || shifts::dirs[dir] < -1) && (currentIndex < 8 || currentIndex >= 56)) break; // Shift is not horizontal

                    setBit(ray, currentIndex, 1);
                }
                all |= ray;
            }
            bishopAttack[i] = all;
        }

        // *** KNIGHT MOVES ***
        bool onNEdge;
        bool onNEdge2;
        bool onEEdge;
        bool onEEdge2;
        bool onSEdge;
        bool onSEdge2;
        bool onWEdge;
        bool onWEdge2;

        for (int i = 0; i < 64; i++) {
            onNEdge = i / 8 == 7;
            onNEdge2 = i / 8 == 7 || i / 8 == 6;
            onEEdge = i % 8 == 7;
            onEEdge2 = i % 8 == 7 || i % 8 == 6;
            onSEdge = i / 8 == 0;
            onSEdge2 = i / 8 == 0 || i / 8 == 1;
            onWEdge = i % 8 == 0;
            onWEdge2 = i % 8 == 0 || i % 8 == 1;

            u64 positions = 0;

            if (!onNEdge2 && !onEEdge) setBit(positions, i + 17, 1); // Up 2, right 1
            if (!onNEdge && !onEEdge2) setBit(positions, i + 10, 1); // Up 1, right 2
            if (!onSEdge && !onEEdge2) setBit(positions, i - 6, 1);  // Down 1, right 2
            if (!onSEdge2 && !onEEdge) setBit(positions, i - 15, 1); // Down 2, right 1

            if (!onSEdge2 && !onWEdge) setBit(positions, i - 17, 1); // Down 2, left 1
            if (!onSEdge && !onWEdge2) setBit(positions, i - 10, 1); // Down 1, left 2
            if (!onNEdge && !onWEdge2) setBit(positions, i + 6, 1);  // Up 1, left 2
            if (!onNEdge2 && !onWEdge) setBit(positions, i + 15, 1); // Up 2, left 1

            knightMoves[i] = positions;
        }
        // *** KING MOVES ***
        for (int i = 0; i < 64; i++) {
            onNEdge = (1ULL << i) & isOn8;
            onEEdge = (1ULL << i) & isOnH;
            onSEdge = (1ULL << i) & isOn1;
            onWEdge = (1ULL << i) & isOnA;

            u64 positions = 0;

            // Horizontal
            if (!onNEdge) setBit(positions, i + shifts::NORTH, 1);
            if (!onEEdge) setBit(positions, i + shifts::EAST, 1);
            if (!onSEdge) setBit(positions, i + shifts::SOUTH, 1);
            if (!onWEdge) setBit(positions, i + shifts::WEST, 1);

            // Diagonal
            if (!onNEdge && !onEEdge) setBit(positions, i + shifts::NORTH_EAST, 1);
            if (!onNEdge && !onWEdge) setBit(positions, i + shifts::NORTH_WEST, 1);
            if (!onSEdge && !onEEdge) setBit(positions, i + shifts::SOUTH_EAST, 1);
            if (!onSEdge && !onWEdge) setBit(positions, i + shifts::SOUTH_WEST, 1);

            kingMoves[i] = positions;
        }

        // *** MAKE RANDOM ZOBRIST TABLE ****
        std::random_device rd;

        std::mt19937_64 engine(rd());

        std::uniform_int_distribution<u64> dist(0, std::numeric_limits<u64>::max());

        for (auto& pieceTable : zobrist) {
            for (int i = 0; i < 64; i++) {
                pieceTable[i] = dist(engine);
            }
        }
    }
};

std::array<u64, 64> Precomputed::knightMoves;
std::array<u64, 64> Precomputed::kingMoves;
std::array<std::array<u64, 64>, 12> Precomputed::zobrist;
std::array<std::array<u64, 9>, 64> Precomputed::rays;
std::array<u64, 64> Precomputed::rookAttack;
std::array<u64, 64> Precomputed::bishopAttack;
u64 Precomputed::isOnA;
u64 Precomputed::isOnB;
u64 Precomputed::isOnC;
u64 Precomputed::isOnD;
u64 Precomputed::isOnE;
u64 Precomputed::isOnF;
u64 Precomputed::isOnG;
u64 Precomputed::isOnH;
u64 Precomputed::isOn1;
u64 Precomputed::isOn2;
u64 Precomputed::isOn3;
u64 Precomputed::isOn4;
u64 Precomputed::isOn5;
u64 Precomputed::isOn6;
u64 Precomputed::isOn7;
u64 Precomputed::isOn8;


constexpr Rank rankOf(Square s) { return Rank(s >> 3); }
constexpr File fileOf(Square s) { return File(s & 0b111); }

// *** MANY A PROGRAMMER HAS "BORROWED" CODE. I AM NO EXCEPTION ***
// Original code from https://github.com/nkarve/surge/blob/master/src/tables.cpp

constexpr int diagonalOf(Square s) { return 7 + rankOf(s) - fileOf(s); }
constexpr int antiDiagonalOf(Square s) { return rankOf(s) + fileOf(s); }

//Precomputed file masks
const u64 MASK_FILE[8] = {
    0x101010101010101, 0x202020202020202, 0x404040404040404, 0x808080808080808,
    0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080,
};

//Precomputed rank masks
const u64 MASK_RANK[8] = {
    0xff, 0xff00, 0xff0000, 0xff000000,
    0xff00000000, 0xff0000000000, 0xff000000000000, 0xff00000000000000
};

//Precomputed diagonal masks
const u64 MASK_DIAGONAL[15] = {
    0x80, 0x8040, 0x804020,
    0x80402010, 0x8040201008, 0x804020100804,
    0x80402010080402, 0x8040201008040201, 0x4020100804020100,
    0x2010080402010000, 0x1008040201000000, 0x804020100000000,
    0x402010000000000, 0x201000000000000, 0x100000000000000,
};

//Precomputed anti-diagonal masks
const u64 MASK_ANTI_DIAGONAL[15] = {
    0x1, 0x102, 0x10204,
    0x1020408, 0x102040810, 0x10204081020,
    0x1020408102040, 0x102040810204080, 0x204081020408000,
    0x408102040800000, 0x810204080000000, 0x1020408000000000,
    0x2040800000000000, 0x4080000000000000, 0x8000000000000000,
};

//Precomputed square masks
const u64 SQUARE_BB[65] = {
    0x1, 0x2, 0x4, 0x8,
    0x10, 0x20, 0x40, 0x80,
    0x100, 0x200, 0x400, 0x800,
    0x1000, 0x2000, 0x4000, 0x8000,
    0x10000, 0x20000, 0x40000, 0x80000,
    0x100000, 0x200000, 0x400000, 0x800000,
    0x1000000, 0x2000000, 0x4000000, 0x8000000,
    0x10000000, 0x20000000, 0x40000000, 0x80000000,
    0x100000000, 0x200000000, 0x400000000, 0x800000000,
    0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
    0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000,
    0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000,
    0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000,
    0x10000000000000, 0x20000000000000, 0x40000000000000, 0x80000000000000,
    0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000,
    0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000,
    0x0
};

//Reverses a bitboard                        
u64 reverse(u64 b) {
    b = (b & 0x5555555555555555) << 1 | (b >> 1) & 0x5555555555555555;
    b = (b & 0x3333333333333333) << 2 | (b >> 2) & 0x3333333333333333;
    b = (b & 0x0f0f0f0f0f0f0f0f) << 4 | (b >> 4) & 0x0f0f0f0f0f0f0f0f;
    b = (b & 0x00ff00ff00ff00ff) << 8 | (b >> 8) & 0x00ff00ff00ff00ff;

    return (b << 48) | ((b & 0xffff0000) << 16) |
        ((b >> 16) & 0xffff0000) | (b >> 48);
}

//Calculates sliding attacks from a given square, on a given axis, taking into
//account the blocking pieces. This uses the Hyperbola Quintessence Algorithm.
u64 sliding_attacks(Square square, u64 occ, u64 mask) {
    return (((mask & occ) - SQUARE_BB[square] * 2) ^
        reverse(reverse(mask & occ) - reverse(SQUARE_BB[square]) * 2)) & mask;
}

//Returns rook attacks from a given square, using the Hyperbola Quintessence Algorithm. Only used to initialize
//the magic lookup table
u64 get_rook_attacks_for_init(Square square, u64 occ) {
    return sliding_attacks(square, occ, MASK_FILE[fileOf(square)]) |
        sliding_attacks(square, occ, MASK_RANK[rankOf(square)]);
}

u64 ROOK_ATTACK_MASKS[64];
int ROOK_ATTACK_SHIFTS[64];
u64 ROOK_ATTACKS[64][4096];

const u64 ROOK_MAGICS[64] = {
    0x0080001020400080, 0x0040001000200040, 0x0080081000200080, 0x0080040800100080,
    0x0080020400080080, 0x0080010200040080, 0x0080008001000200, 0x0080002040800100,
    0x0000800020400080, 0x0000400020005000, 0x0000801000200080, 0x0000800800100080,
    0x0000800400080080, 0x0000800200040080, 0x0000800100020080, 0x0000800040800100,
    0x0000208000400080, 0x0000404000201000, 0x0000808010002000, 0x0000808008001000,
    0x0000808004000800, 0x0000808002000400, 0x0000010100020004, 0x0000020000408104,
    0x0000208080004000, 0x0000200040005000, 0x0000100080200080, 0x0000080080100080,
    0x0000040080080080, 0x0000020080040080, 0x0000010080800200, 0x0000800080004100,
    0x0000204000800080, 0x0000200040401000, 0x0000100080802000, 0x0000080080801000,
    0x0000040080800800, 0x0000020080800400, 0x0000020001010004, 0x0000800040800100,
    0x0000204000808000, 0x0000200040008080, 0x0000100020008080, 0x0000080010008080,
    0x0000040008008080, 0x0000020004008080, 0x0000010002008080, 0x0000004081020004,
    0x0000204000800080, 0x0000200040008080, 0x0000100020008080, 0x0000080010008080,
    0x0000040008008080, 0x0000020004008080, 0x0000800100020080, 0x0000800041000080,
    0x00FFFCDDFCED714A, 0x007FFCDDFCED714A, 0x003FFFCDFFD88096, 0x0000040810002101,
    0x0001000204080011, 0x0001000204000801, 0x0001000082000401, 0x0001FFFAABFAD1A2
};

//Initializes the magic lookup table for rooks
void initializeRookAttacks() {
    u64 edges, subset, index;

    for (Square sq = a1; sq <= h8; ++sq) {
        edges = ((MASK_RANK[AFILE] | MASK_RANK[HFILE]) & ~MASK_RANK[rankOf(sq)]) |
            ((MASK_FILE[AFILE] | MASK_FILE[HFILE]) & ~MASK_FILE[fileOf(sq)]);
        ROOK_ATTACK_MASKS[sq] = (MASK_RANK[rankOf(sq)]
            ^ MASK_FILE[fileOf(sq)]) & ~edges;
        ROOK_ATTACK_SHIFTS[sq] = 64 - popcountll(ROOK_ATTACK_MASKS[sq]);

        subset = 0;
        do {
            index = subset;
            index = index * ROOK_MAGICS[sq];
            index = index >> ROOK_ATTACK_SHIFTS[sq];
            ROOK_ATTACKS[sq][index] = get_rook_attacks_for_init(sq, subset);
            subset = (subset - ROOK_ATTACK_MASKS[sq]) & ROOK_ATTACK_MASKS[sq];
        } while (subset);
    }
}

//Returns the attacks bitboard for a rook at a given square, using the magic lookup table
constexpr u64 getRookAttacks(Square square, u64 occ) {
    return ROOK_ATTACKS[square][((occ & ROOK_ATTACK_MASKS[square]) * ROOK_MAGICS[square])
        >> ROOK_ATTACK_SHIFTS[square]];
}

//Returns the 'x-ray attacks' for a rook at a given square. X-ray attacks cover squares that are not immediately
//accessible by the rook, but become available when the immediate blockers are removed from the board 
u64 getXrayRookAttacks(Square square, u64 occ, u64 blockers) {
    u64 attacks = getRookAttacks(square, occ);
    blockers &= attacks;
    return attacks ^ getRookAttacks(square, occ ^ blockers);
}

//Returns bishop attacks from a given square, using the Hyperbola Quintessence Algorithm. Only used to initialize
//the magic lookup table
u64 getBishopAttacksForInit(Square square, u64 occ) {
    return sliding_attacks(square, occ, MASK_DIAGONAL[diagonalOf(square)]) |
        sliding_attacks(square, occ, MASK_ANTI_DIAGONAL[antiDiagonalOf(square)]);
}

u64 BISHOP_ATTACK_MASKS[64];
int BISHOP_ATTACK_SHIFTS[64];
u64 BISHOP_ATTACKS[64][512];

const u64 BISHOP_MAGICS[64] = {
    0x0002020202020200, 0x0002020202020000, 0x0004010202000000, 0x0004040080000000,
    0x0001104000000000, 0x0000821040000000, 0x0000410410400000, 0x0000104104104000,
    0x0000040404040400, 0x0000020202020200, 0x0000040102020000, 0x0000040400800000,
    0x0000011040000000, 0x0000008210400000, 0x0000004104104000, 0x0000002082082000,
    0x0004000808080800, 0x0002000404040400, 0x0001000202020200, 0x0000800802004000,
    0x0000800400A00000, 0x0000200100884000, 0x0000400082082000, 0x0000200041041000,
    0x0002080010101000, 0x0001040008080800, 0x0000208004010400, 0x0000404004010200,
    0x0000840000802000, 0x0000404002011000, 0x0000808001041000, 0x0000404000820800,
    0x0001041000202000, 0x0000820800101000, 0x0000104400080800, 0x0000020080080080,
    0x0000404040040100, 0x0000808100020100, 0x0001010100020800, 0x0000808080010400,
    0x0000820820004000, 0x0000410410002000, 0x0000082088001000, 0x0000002011000800,
    0x0000080100400400, 0x0001010101000200, 0x0002020202000400, 0x0001010101000200,
    0x0000410410400000, 0x0000208208200000, 0x0000002084100000, 0x0000000020880000,
    0x0000001002020000, 0x0000040408020000, 0x0004040404040000, 0x0002020202020000,
    0x0000104104104000, 0x0000002082082000, 0x0000000020841000, 0x0000000000208800,
    0x0000000010020200, 0x0000000404080200, 0x0000040404040400, 0x0002020202020200
};

//Initializes the magic lookup table for bishops
void initializeBishopAttacks() {
    u64 edges, subset, index;

    for (Square sq = a1; sq <= h8; ++sq) {
        edges = ((MASK_RANK[AFILE] | MASK_RANK[HFILE]) & ~MASK_RANK[rankOf(sq)]) |
            ((MASK_FILE[AFILE] | MASK_FILE[HFILE]) & ~MASK_FILE[fileOf(sq)]);
        BISHOP_ATTACK_MASKS[sq] = (MASK_DIAGONAL[diagonalOf(sq)]
            ^ MASK_ANTI_DIAGONAL[antiDiagonalOf(sq)]) & ~edges;
        BISHOP_ATTACK_SHIFTS[sq] = 64 - popcountll(BISHOP_ATTACK_MASKS[sq]);

        subset = 0;
        do {
            index = subset;
            index = index * BISHOP_MAGICS[sq];
            index = index >> BISHOP_ATTACK_SHIFTS[sq];
            BISHOP_ATTACKS[sq][index] = getBishopAttacksForInit(sq, subset);
            subset = (subset - BISHOP_ATTACK_MASKS[sq]) & BISHOP_ATTACK_MASKS[sq];
        } while (subset);
    }
}

//Returns the attacks bitboard for a bishop at a given square, using the magic lookup table
constexpr u64 getBishopAttacks(Square square, u64 occ) {
    return BISHOP_ATTACKS[square][((occ & BISHOP_ATTACK_MASKS[square]) * BISHOP_MAGICS[square])
        >> BISHOP_ATTACK_SHIFTS[square]];
}

//Returns the 'x-ray attacks' for a bishop at a given square. X-ray attacks cover squares that are not immediately
//accessible by the rook, but become available when the immediate blockers are removed from the board 
u64 getXrayBishopAttacks(Square square, u64 occ, u64 blockers) {
    u64 attacks = getBishopAttacks(square, occ);
    blockers &= attacks;
    return attacks ^ getBishopAttacks(square, occ ^ blockers);
}

u64 SQUARES_BETWEEN_BB[64][64];

//Initializes the lookup table for the bitboard of squares in between two given squares (0 if the 
//two squares are not aligned)
void initializeSquaresBetween() {
    u64 sqs;
    for (Square sq1 = a1; sq1 <= h8; ++sq1)
        for (Square sq2 = a1; sq2 <= h8; ++sq2) {
            sqs = SQUARE_BB[sq1] | SQUARE_BB[sq2];
            if (fileOf(sq1) == fileOf(sq2) || rankOf(sq1) == rankOf(sq2))
                SQUARES_BETWEEN_BB[sq1][sq2] =
                get_rook_attacks_for_init(sq1, sqs) & get_rook_attacks_for_init(sq2, sqs);
            else if (diagonalOf(sq1) == diagonalOf(sq2) || antiDiagonalOf(sq1) == antiDiagonalOf(sq2))
                SQUARES_BETWEEN_BB[sq1][sq2] =
                getBishopAttacksForInit(sq1, sqs) & getBishopAttacksForInit(sq2, sqs);
        }
}


u64 LINE[64][64];

//Initializes the lookup table for the bitboard of all squares along the line of two given squares (0 if the 
//two squares are not aligned)
void initializeLine() {
    for (Square sq1 = a1; sq1 <= h8; ++sq1)
        for (Square sq2 = a1; sq2 <= h8; ++sq2) {
            if (fileOf(sq1) == fileOf(sq2) || rankOf(sq1) == rankOf(sq2))
                LINE[sq1][sq2] =
                get_rook_attacks_for_init(sq1, 0) & get_rook_attacks_for_init(sq2, 0)
                | SQUARE_BB[sq1] | SQUARE_BB[sq2];
            else if (diagonalOf(sq1) == diagonalOf(sq2) || antiDiagonalOf(sq1) == antiDiagonalOf(sq2))
                LINE[sq1][sq2] =
                getBishopAttacksForInit(sq1, 0) & getBishopAttacksForInit(sq2, 0)
                | SQUARE_BB[sq1] | SQUARE_BB[sq2];
        }
}

//Initializes lookup tables for rook moves, bishop moves, in-between squares, aligned squares and pseudolegal moves
void initializeAllDatabases() {
    initializeRookAttacks();
    initializeBishopAttacks();
    initializeSquaresBetween();
    initializeLine();
}

// Back to my code from here and below

struct MoveEvaluation {
    Move move;
    int eval;
};

struct MoveList {
    std::array<Move, 218> moves;
    int count;

    MoveList() {
        count = 0;
    }

    inline void add(Move m) {
        moves[count++] = m;
    }
};

class Board {
public:
    std::array<u64, 6> white; // Goes pawns, knights, bishops, rooks, queens, king
    std::array<u64, 6> black; // Goes pawns, knights, bishops, rooks, queens, king

    u64 blackPieces;
    u64 whitePieces;
    u64 emptySquares;

    uint64_t enPassant;
    u8 castlingRights;

    Color side = WHITE;

    int halfMoveClock;

    void reset() {
        // Reset position
        white[0] = 0xFF00ULL;
        white[1] = 0x42ULL;
        white[2] = 0x24ULL;
        white[3] = 0x81ULL;
        white[4] = 0x8ULL;
        white[5] = 0x10ULL;

        black[0] = 0xFF000000000000ULL;
        black[1] = 0x4200000000000000ULL;
        black[2] = 0x2400000000000000ULL;
        black[3] = 0x8100000000000000ULL;
        black[4] = 0x800000000000000ULL;
        black[5] = 0x1000000000000000ULL;

        castlingRights = 0xF;

        enPassant = 0; // No en passant target square
        side = WHITE;

        halfMoveClock = 0;

        recompute();
    }

    inline void clearIndex(int index) {
        const u64 mask = ~(1ULL << index);
        white[0] &= mask;
        white[1] &= mask;
        white[2] &= mask;
        white[3] &= mask;
        white[4] &= mask;
        white[5] &= mask;

        black[0] &= mask;
        black[1] &= mask;
        black[2] &= mask;
        black[3] &= mask;
        black[4] &= mask;
        black[5] &= mask;
    }

    inline void recompute() {
        whitePieces = white[0] | white[1] | white[2] | white[3] | white[4] | white[5];
        blackPieces = black[0] | black[1] | black[2] | black[3] | black[4] | black[5];

        emptySquares = ~(whitePieces | blackPieces);
    }

    inline void generatePawnMoves(MoveList& moves) {
        // Note: captures like "pawnCaptureLeft" means take TOWARD THE A FILE, regardless of color, AKA capture to the left if you are playing as white
        u64 pawns = (side) * (white[0]) + (!side) * (black[0]);

        u64 pawnPushes = (side) * (white[0] << 8) + (!side) * (black[0] >> 8); // Branchless if to get pawn bitboard
        pawnPushes &= emptySquares;
        int currentIndex;

        u64 pawnCaptureRight = pawns & ~(Precomputed::isOnH);
        pawnCaptureRight = (side) * (pawnCaptureRight << 9) + (!side) * (pawnCaptureRight >> 7); // Branchless if to get pawn bitboard
        pawnCaptureRight &= (side) * (blackPieces | enPassant) + (!side) * (whitePieces | enPassant);

        u64 pawnCaptureLeft = pawns & ~(Precomputed::isOnA);
        pawnCaptureLeft = (side) * (pawnCaptureLeft << 7) + (!side) * (pawnCaptureLeft >> 9); // Branchless if to get pawn bitboard
        pawnCaptureLeft &= (side) * (blackPieces | enPassant) + (!side) * (whitePieces | enPassant);

        u64 pawnDoublePush = (side) * (white[0] << 16) + (!side) * (black[0] >> 16); // Branchless if to get pawn bitboard
        pawnDoublePush &= emptySquares & ((side) * (emptySquares << 8) + (!side) * (emptySquares >> 8));
        pawnDoublePush &= (side) * (Precomputed::isOn2 << 16) + (!side) * (Precomputed::isOn7 >> 16);
        if (side) {
            pawnDoublePush &= (Precomputed::isOn2 << 16);
        }
        else {
            pawnDoublePush &= (Precomputed::isOn7 >> 16);
        }
        while (pawnDoublePush) {
            currentIndex = ctzll(pawnDoublePush);
            moves.add(Move((side) * (currentIndex - 16) + (!side) * (currentIndex + 16), currentIndex)); // Another branchless loop

            pawnDoublePush &= pawnDoublePush - 1;
        }

        while (pawnPushes) {
            currentIndex = ctzll(pawnPushes);
            if ((1ULL << currentIndex) & (Precomputed::isOn1 | Precomputed::isOn8)) {
                moves.add(Move((side) * (currentIndex - 8) + (!side) * (currentIndex + 8), currentIndex, 2)); // Another branchless loop
                moves.add(Move((side) * (currentIndex - 8) + (!side) * (currentIndex + 8), currentIndex, 3)); // Another branchless loop
                moves.add(Move((side) * (currentIndex - 8) + (!side) * (currentIndex + 8), currentIndex, 4)); // Another branchless loop
            }
            moves.add(Move((side) * (currentIndex - 8) + (!side) * (currentIndex + 8), currentIndex)); // Another branchless loop

            pawnPushes &= pawnPushes - 1;
        }

        while (pawnCaptureRight) {
            currentIndex = ctzll(pawnCaptureRight);
            if ((1ULL << currentIndex) & (Precomputed::isOn1 | Precomputed::isOn8)) {
                moves.add(Move((side) * (currentIndex - 9) + (!side) * (currentIndex + 7), currentIndex, 2)); // Another branchless loop
                moves.add(Move((side) * (currentIndex - 9) + (!side) * (currentIndex + 7), currentIndex, 3)); // Another branchless loop
                moves.add(Move((side) * (currentIndex - 9) + (!side) * (currentIndex + 7), currentIndex, 4)); // Another branchless loop
            }
            moves.add(Move((side) * (currentIndex - 9) + (!side) * (currentIndex + 7), currentIndex)); // Another branchless loop

            pawnCaptureRight &= pawnCaptureRight - 1;
        }

        while (pawnCaptureLeft) {
            currentIndex = ctzll(pawnCaptureLeft);
            if ((1ULL << currentIndex) & (Precomputed::isOn1 | Precomputed::isOn8)) {
                moves.add(Move((side) * (currentIndex - 7) + (!side) * (currentIndex + 9), currentIndex, 2)); // Another branchless loop
                moves.add(Move((side) * (currentIndex - 7) + (!side) * (currentIndex + 9), currentIndex, 3)); // Another branchless loop
                moves.add(Move((side) * (currentIndex - 7) + (!side) * (currentIndex + 9), currentIndex, 4)); // Another branchless loop
            }
            moves.add(Move((side) * (currentIndex - 7) + (!side) * (currentIndex + 9), currentIndex)); // Another branchless loop

            pawnCaptureLeft &= pawnCaptureLeft - 1;
        }
    }

    inline void generateKnightMoves(MoveList& moves) {
        int currentIndex;

        u64 knightBitboard = side ? white[1] : black[1];
        u64 ownBitboard = side ? whitePieces : blackPieces;

        while (knightBitboard > 0) {
            currentIndex = ctzll(knightBitboard);

            u64 knightMoves = Precomputed::knightMoves[currentIndex];
            knightMoves &= ~ownBitboard;

            while (knightMoves > 0) {
                moves.add(Move(currentIndex, ctzll(knightMoves)));
                knightMoves &= knightMoves - 1;
            }
            knightBitboard &= knightBitboard - 1; // Clear least significant bit
        }
    }

    void generateBishopMoves(MoveList& moves) {
        int currentIndex;
        int& currentMoveIndex = moves.count;

        u64 bishopBitboard = side ? white[2] : black[2];
        u64 ourBitboard = side ? whitePieces : blackPieces;

        u64 occupancy = whitePieces | blackPieces;

        u64 moveMask;

        while (bishopBitboard > 0) {
            currentIndex = ctzll(bishopBitboard);

            moveMask = getBishopAttacks(Square(currentIndex), occupancy);

            moveMask &= ~ourBitboard;

            // Make move with each legal move in mask
            while (moveMask > 0) {
                int maskIndex = ctzll(moveMask);
                moves.moves[currentMoveIndex++] = Move(currentIndex, maskIndex);
                moveMask &= moveMask - 1;
            }

            bishopBitboard &= bishopBitboard - 1; // Clear least significant bit
        }
    }

    void generateRookMoves(MoveList& moves) {
        int currentIndex;
        int& currentMoveIndex = moves.count;

        u64 rookBitboard = side ? white[3] : black[3];
        u64 ourBitboard = side ? whitePieces : blackPieces;

        u64 occupancy = whitePieces | blackPieces;

        u64 moveMask;

        while (rookBitboard > 0) {
            currentIndex = ctzll(rookBitboard);

            moveMask = getRookAttacks(Square(currentIndex), occupancy);

            moveMask &= ~ourBitboard;

            // Make move with each legal move in mask
            while (moveMask > 0) {
                int maskIndex = ctzll(moveMask);
                moves.moves[currentMoveIndex++] = Move(currentIndex, maskIndex);
                moveMask &= moveMask - 1;
            }

            rookBitboard &= rookBitboard - 1; // Clear least significant bit
        }
    }

    void generateQueenMoves(MoveList& moves) {
        int currentIndex;
        int& currentMoveIndex = moves.count;

        u64 queenBitboard = side ? white[4] : black[4];
        u64 ourBitboard = side ? whitePieces : blackPieces;

        u64 occupancy = whitePieces | blackPieces;

        u64 moveMask;

        while (queenBitboard > 0) {
            currentIndex = ctzll(queenBitboard);

            moveMask = getBishopAttacks(Square(currentIndex), occupancy);
            moveMask |= getRookAttacks(Square(currentIndex), occupancy);

            moveMask &= ~ourBitboard;

            // Make move with each legal move in mask
            while (moveMask > 0) {
                int maskIndex = ctzll(moveMask);
                moves.moves[currentMoveIndex++] = Move(currentIndex, maskIndex);
                moveMask &= moveMask - 1;
            }

            queenBitboard &= queenBitboard - 1; // Clear least significant bit
        }
    }

    void generateKingMoves(MoveList& moves) {
        int& currentMoveIndex = moves.count;

        u64 kingBitboard = side ? white[5] : black[5];
        if (!kingBitboard) return;
        u64 ownBitboard = side ? whitePieces : blackPieces;
        int currentIndex = ctzll(kingBitboard);

        u64 kingMoves = Precomputed::kingMoves[currentIndex];
        kingMoves &= ~ownBitboard;

        // Regular king moves
        while (kingMoves > 0) {
            moves.add(Move(currentIndex, ctzll(kingMoves)));
            kingMoves &= kingMoves - 1;
        }

        // Castling moves
        if (side && currentIndex == e1) {
            moves.moves[currentMoveIndex++] = Move(e1, g1);
            moves.moves[currentMoveIndex++] = Move(e1, c1);
        }
        else if (!side && currentIndex == e8) {
            moves.moves[currentMoveIndex++] = Move(e8, g8);
            moves.moves[currentMoveIndex++] = Move(e8, c8);
        }
    }

    MoveList generateMoves() {
        recompute();
        MoveList moves;
        generatePawnMoves(moves);
        generateKnightMoves(moves);
        generateBishopMoves(moves);
        generateRookMoves(moves);
        generateQueenMoves(moves);
        generateKingMoves(moves);

        return moves;
    }

    inline bool isInCheck(bool checkWhite) {
        u64 kingBit = checkWhite ? white[5] : black[5];
        if (!kingBit) return false;
        return isUnderAttack(checkWhite, ctzll(kingBit));
    }

    bool isUnderAttack(bool checkWhite, int square) {
        auto& opponentPieces = checkWhite ? black : white;

        // *** SLIDING PIECE ATTACKS ***
        u64 allPieces = whitePieces | blackPieces;
        u64& occupancy = allPieces;

        // Straight Directions (Rooks and Queens)
        if ((opponentPieces[3] | opponentPieces[4]) & getRookAttacks(Square(square), occupancy)) return true;

        // Diagonal Directions (Bishops and Queens)
        if ((opponentPieces[2] | opponentPieces[4]) & getBishopAttacks(Square(square), occupancy)) return true;


        // *** KNIGHT ATTACKS ***
        if (opponentPieces[1] & Precomputed::knightMoves[square]) return true;

        // *** KING ATTACKS ***
        if (opponentPieces[5] & Precomputed::kingMoves[square]) return true;


        // *** PAWN ATTACKS ***
        if (checkWhite) {
            if ((opponentPieces[0] & (1ULL << (square + 7))) && (square % 8 != 0)) return true;
            if ((opponentPieces[0] & (1ULL << (square + 9))) && (square % 8 != 7)) return true;
        }
        else {
            if ((opponentPieces[0] & (1ULL << (square - 7))) && (square % 8 != 7)) return true;
            if ((opponentPieces[0] & (1ULL << (square - 9))) && (square % 8 != 0)) return true;
        }

        return false;
    }

    bool isLegalMove(Move m) {
        int start = m.startSquare();
        int end = m.endSquare();

        // Delete null moves
        if (start == end) return false;

        int kingIndex = ctzll(side ? white[5] : black[5]);

        // Castling checks:
        // White king from e1(4) to g1(6)=K-side or c1(2)=Q-side
        // Black king from e8(60) to g8(62)=k-side or c8(58)=q-side
        if (typeOf(m) == CASTLING) {
            if (side) {
                bool kingside = (end == g1);
                if (kingside && !readBit(castlingRights, 3)) return false; // 'K'
                if (!kingside && !readBit(castlingRights, 2)) return false; // 'Q'
                if (isInCheck(side)) return false;
                u64 occupied = whitePieces | blackPieces;
                if (kingside) {
                    if (occupied & ((1ULL << f1) & (1ULL << g1))) return false;
                    if ((occupied & ((1ULL << f1) | (1ULL << g1))) != 0) return false;
                    if (isUnderAttack(true, f1) || isUnderAttack(true, g1)) return false;
                }
                else {
                    if ((occupied & ((1ULL << b1) | (1ULL << c1) | (1ULL << d1))) != 0) return false;
                    if (isUnderAttack(true, d1) || isUnderAttack(true, c1)) return false;
                }
            }
            else if (!side) {
                bool kingside = (end == g8);
                if (kingside && !readBit(castlingRights, 1)) return false; // 'k'
                if (!kingside && !readBit(castlingRights, 0)) return false; // 'q'
                // Use false here because we are checking if black is under attack
                if (isInCheck(false)) return false;
                u64 occupied = whitePieces | blackPieces;
                if (kingside) {
                    if ((occupied & ((1ULL << f8) | (1ULL << g8))) != 0) return false;
                    if (isUnderAttack(false, f8) || isUnderAttack(false, g8)) return false;
                }
                else {
                    if ((occupied & ((1ULL << b8) | (1ULL << c8) | (1ULL << d8))) != 0) return false;
                    if (isUnderAttack(false, d8) || isUnderAttack(false, c8)) return false;
                }
            }
        }

        // Test final position
        Board testBoard = *this;
        testBoard.move(m, false);

        return !testBoard.isInCheck(side);
        // Moving piece that in pinned
        //if (pinned && !(LINE[kingIndex][pinner] & (1ULL << m.endSquare()))) return false;

        return true;
    }

    MoveList generateLegalMoves() {
        auto moves = generateMoves();
        int i = 0;

        while (i < moves.count) {
            if (!isLegalMove(moves.moves[i])) {
                // Replace the current move with the last move and decrease count
                moves.moves[i] = moves.moves[moves.count - 1];
                moves.count--;
            }
            else {
                i++;
            }
        }
        return moves;
    }


    void display() {
        if (side)
            std::cout << "White's turn" << std::endl;
        else
            std::cout << "Black's turn" << std::endl;

        for (int rank = 7; rank >= 0; rank--) {
            std::cout << "+---+---+---+---+---+---+---+---+" << std::endl;
            for (int file = 0; file < 8; file++) {
                int i = rank * 8 + file;  // Map rank and file to bitboard index
                char currentPiece = ' ';

                if (readBit(white[0], i)) currentPiece = 'P';
                else if (readBit(white[1], i)) currentPiece = 'N';
                else if (readBit(white[2], i)) currentPiece = 'B';
                else if (readBit(white[3], i)) currentPiece = 'R';
                else if (readBit(white[4], i)) currentPiece = 'Q';
                else if (readBit(white[5], i)) currentPiece = 'K';

                else if (readBit(black[0], i)) currentPiece = 'p';
                else if (readBit(black[1], i)) currentPiece = 'n';
                else if (readBit(black[2], i)) currentPiece = 'b';
                else if (readBit(black[3], i)) currentPiece = 'r';
                else if (readBit(black[4], i)) currentPiece = 'q';
                else if (readBit(black[5], i)) currentPiece = 'k';

                std::cout << "| " << currentPiece << " ";
            }
            std::cout << "|" << std::endl;
        }
        std::cout << "+---+---+---+---+---+---+---+---+" << std::endl;
    }

    int typeOf(Move& m) {
        int from = m.startSquare();
        int to = m.endSquare();
        u64 fromMask = 1ULL << from;
        u64 endMask = 1ULL << to;
        if (side && (white[5] & fromMask) && from == e1 && (to == c1 || to == g1)) return CASTLING;
        if (!side && (black[5] & fromMask) && from == e8 && (to == c8 || to == g8)) return CASTLING;
        if (side && (white[0] & fromMask) && (enPassant & endMask)) return EN_PASSANT;
        if (!side && (black[0] & fromMask) && (enPassant & endMask)) return EN_PASSANT;
        return -1; // Null value for all moves
    }

    inline void move(std::string& moveIn) {
        move(Move(moveIn));
    }

    void move(Move moveIn, bool updateMoveHistory = true) {
        auto& ourSide = side ? white : black;

        for (int i = 0; i < 6; i++) {
            if (readBit(ourSide[i], moveIn.startSquare())) {
                auto from = moveIn.startSquare();
                auto to = moveIn.endSquare();

                clearIndex(from);
                clearIndex(to);

                if (i == 0 && (((1ULL << to) & (Precomputed::isOn1 | Precomputed::isOn8)) > 0)) { // Move is promo
                    int promo = moveIn.promo();
                    if (promo == 4) setBit(ourSide[4], moveIn.endSquare(), 1);
                    else if (promo == 3) setBit(ourSide[3], moveIn.endSquare(), 1);
                    else if (promo == 2) setBit(ourSide[2], moveIn.endSquare(), 1);
                    else if (promo == 1) setBit(ourSide[1], moveIn.endSquare(), 1);
                }
                else setBit(ourSide[i], to, 1);

                // EN PASSANT
                if (i == 0 && to == ctzll(enPassant)) {
                    if (side) setBit(black[0], to + shifts::SOUTH, 0);
                    else setBit(white[0], to + shifts::NORTH, 0);
                }

                // Halfmove clock, promo and set en passant
                if (i == 0 || readBit((side) ? blackPieces : whitePieces, to)) halfMoveClock = -1; // Reset halfmove clock on capture or pawn move
                if (i == 0 && std::abs(from - to) == 16) enPassant = 1ULL << ((side) * (from + 8) + (!side) * (from - 8)); // Set en passant bitboard
                else enPassant = 0;


                // Castling
                if (i == 5) {
                    if (from == e1 && to == g1 && readBit(castlingRights, 3)) {
                        setBit(ourSide[3], h1, 0); // Remove rook

                        setBit(ourSide[3], f1, 1); // Set rook
                    }
                    else if (from == e1 && to == c1 && readBit(castlingRights, 2)) {
                        setBit(ourSide[3], a1, 0); // Remove rook

                        setBit(ourSide[3], d1, 1); // Set rook
                    }
                    else if (from == e8 && to == g8 && readBit(castlingRights, 1)) {
                        setBit(ourSide[3], h8, 0); // Remove rook

                        setBit(ourSide[3], f8, 1); // Set rook
                    }
                    else if (from == e8 && to == c8 && readBit(castlingRights, 0)) {
                        setBit(ourSide[3], a8, 0); // Remove rook

                        setBit(ourSide[3], d8, 1); // Set rook
                    }
                }
                break;
            }
        }

        // Remove castling rights (very unoptimized)
        if (castlingRights) {
            int from = moveIn.startSquare();
            int to = moveIn.endSquare();
            if (from == 0 || to == 0) setBit(castlingRights, 2, 0);
            if (from == 7 || to == 7) setBit(castlingRights, 3, 0);
            if (from == 4) { // King moved
                setBit(castlingRights, 2, 0);
                setBit(castlingRights, 3, 0);
            }
            if (from == 56 || to == 56) setBit(castlingRights, 0, 0);
            if (from == 63 || to == 63) setBit(castlingRights, 1, 0);
            if (from == 60) { // King moved
                setBit(castlingRights, 0, 0);
                setBit(castlingRights, 1, 0);
            }
        }

        side = ~side;

        halfMoveClock++;

        recompute();

        //if (updateMoveHistory) moveHistory.push_back(calculateZobrist()); CALCULATE ZOBRIST IS BAD. NO MORE TRIFOLD DETECTION FOR THIS BOT.
    }

    void loadFromFEN(const std::deque<std::string>& inputFEN) {
        // Reset
        reset();

        for (int i = 0; i < 64; i++) clearIndex(i);

        // Sanitize input
        if (inputFEN.size() < 6) {
            std::cerr << "Invalid FEN string" << std::endl;
            return;
        }

        std::deque<std::string> parsedPosition = split(inputFEN.at(0), '/');
        char currentCharacter;
        int currentIndex = 56; // Start at rank 8, file 'a' (index 56)

        for (const std::string& rankString : parsedPosition) {
            for (char c : rankString) {
                currentCharacter = c;

                if (isdigit(currentCharacter)) { // Empty squares
                    int emptySquares = currentCharacter - '0';
                    currentIndex += emptySquares; // Skip the given number of empty squares
                }
                else { // Piece placement
                    switch (currentCharacter) {
                    case 'P': setBit(white[0], currentIndex, 1); break;
                    case 'N': setBit(white[1], currentIndex, 1); break;
                    case 'B': setBit(white[2], currentIndex, 1); break;
                    case 'R': setBit(white[3], currentIndex, 1); break;
                    case 'Q': setBit(white[4], currentIndex, 1); break;
                    case 'K': setBit(white[5], currentIndex, 1); break;
                    case 'p': setBit(black[0], currentIndex, 1); break;
                    case 'n': setBit(black[1], currentIndex, 1); break;
                    case 'b': setBit(black[2], currentIndex, 1); break;
                    case 'r': setBit(black[3], currentIndex, 1); break;
                    case 'q': setBit(black[4], currentIndex, 1); break;
                    case 'k': setBit(black[5], currentIndex, 1); break;
                    default: break;
                    }
                    currentIndex++;
                }
            }
            currentIndex -= 16; // Move to next rank in FEN
        }

        if (inputFEN.at(1) == "w") { // Check the next player's move
            side = WHITE;
        }
        else {
            side = BLACK;
        }

        castlingRights = 0;

        if (inputFEN.at(2).find('K') != std::string::npos) castlingRights |= 1 << 3;
        if (inputFEN.at(2).find('Q') != std::string::npos) castlingRights |= 1 << 2;
        if (inputFEN.at(2).find('k') != std::string::npos) castlingRights |= 1 << 1;
        if (inputFEN.at(2).find('q') != std::string::npos) castlingRights |= 1;

        if (inputFEN.at(3) != "-") enPassant = 1ULL << parseSquare(inputFEN.at(3));
        else enPassant = 0;

        halfMoveClock = std::stoi(inputFEN.at(4));

        recompute();
    }

    inline int black_to_white(int index) {
        int rank = index / 8;
        int file = index % 8;
        int mirrored_rank = 7 - rank;
        return mirrored_rank * 8 + file;
    }

    int evaluate() { // Returns evaluation in centipawns
        //if (isDraw()) return 0;
        //if (isCheckmate(isWhite)) return (isWhite) ? -100000 : 100000;
        int eval = 0;
        int whitePieces = 0;
        int blackPieces = 0;

        // Uses some magic python buffoonery https://github.com/ianfab/chess-variant-stats/blob/main/piece_values.py
        // Based on this https://discord.com/channels/435943710472011776/1300744461281787974/1312722964915027980

        // Material evaluation
        whitePieces += popcountll(white[0]) * 100;
        whitePieces += popcountll(white[1]) * 316;
        whitePieces += popcountll(white[2]) * 328;
        whitePieces += popcountll(white[3]) * 493;
        whitePieces += popcountll(white[4]) * 982;

        blackPieces += popcountll(black[0]) * 100;
        blackPieces += popcountll(black[1]) * 316;
        blackPieces += popcountll(black[2]) * 328;
        blackPieces += popcountll(black[3]) * 493;
        blackPieces += popcountll(black[4]) * 982;

        eval = whitePieces - blackPieces;

        // Piece value adjustment
        if (std::abs(eval) < 950) { // Ignore if the eval is already very very high
            for (int i = 0; i < 6; i++) {
                u64 currentBitboard = white[i];
                while (currentBitboard > 0) {
                    int currentIndex = ctzll(currentBitboard);

                    if (i == 0) eval += Precomputed::white_pawn_table[currentIndex];
                    if (i == 1) eval += Precomputed::white_knight_table[currentIndex];
                    if (i == 2) eval += Precomputed::white_bishop_table[currentIndex];
                    if (i == 3) eval += Precomputed::white_rook_table[currentIndex];
                    if (i == 4) eval += Precomputed::white_queen_table[currentIndex];
                    if (i == 5) eval += Precomputed::white_king_table[currentIndex];

                    currentBitboard &= currentBitboard - 1;
                }
            }

            for (int i = 0; i < 6; i++) {
                u64 currentBitboard = black[i];
                while (currentBitboard > 0) {
                    int currentIndex = ctzll(currentBitboard);

                    if (i == 0) eval -= Precomputed::white_pawn_table[black_to_white(currentIndex)];
                    if (i == 1) eval -= Precomputed::white_knight_table[black_to_white(currentIndex)];
                    if (i == 2) eval -= Precomputed::white_bishop_table[black_to_white(currentIndex)];
                    if (i == 3) eval -= Precomputed::white_rook_table[black_to_white(currentIndex)];
                    if (i == 4) eval -= Precomputed::white_queen_table[black_to_white(currentIndex)];
                    if (i == 5) eval -= Precomputed::white_king_table[black_to_white(currentIndex)];

                    currentBitboard &= currentBitboard - 1;
                }
            }
        }

        // Adjust evaluation for the side to move
        return (side) ? eval : -eval;;
    }
};

std::string Move::toString(Board& board) {
    int startSquare = this->startSquare();
    int endSquare = this->endSquare();
    std::string out = std::string(1, (char)(startSquare % 8 + 'a')) + std::to_string(startSquare / 8 + 1) + std::string(1, (char)(endSquare % 8 + 'a')) + std::to_string(endSquare / 8 + 1);

    if (readBit(move, 14)) out += "q";
    else if (readBit(move, 13)) out += "r";
    else if (readBit(move, 12)) out += "b";
    else if (board.side && ((1ULL << endSquare) & Precomputed::isOn8) && (board.white[0] & 1ULL << startSquare)) out += "n"; // Is there a pawn on the start square
    else if (!board.side && ((1ULL << endSquare) & Precomputed::isOn1) && (board.black[0] & 1ULL << startSquare)) out += "n";
    return out;
}

u64 _perft(Board& board, int depth) {
    if (depth == 1) {
        return board.generateLegalMoves().count;
    }

    else if (depth == 0) {
        return 1ULL;
    }

    MoveList moves = board.generateLegalMoves();
    u64 localNodes = 0;
    for (int i = 0; i < moves.count; i++) {
        Board testBoard = board;
        testBoard.move(moves.moves[i]);
        localNodes += _perft(testBoard, depth - 1);
    }
    return localNodes;
}

u64 perft(Board& board, int depth) {
    if (depth < 1) return -1;

    MoveList moves = board.generateLegalMoves();
    u64 localNodes = 0;
    u64 movesThisIter = 0;
    for (int i = 0; i < moves.count; i++) {
        Board testBoard = board;
        testBoard.move(moves.moves[i]);
        movesThisIter = _perft(testBoard, depth - 1);
        localNodes += movesThisIter;
        std::cout << moves.moves[i].toString(board) << ": " << movesThisIter << std::endl;
    }
    return localNodes;
}

void perftSuite(const std::string& filePath) {
    Board board;

    std::fstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    std::string line;
    int totalTests = 0;
    int passedTests = 0;

    while (std::getline(file, line)) {
        auto start = std::chrono::high_resolution_clock::now();

        u64 nodes = 0;

        if (line.empty()) continue; // Skip empty lines

        // Split the line by ';'
        std::deque<std::string> parts = split(line, ';');

        if (parts.empty()) continue;

        // The first part is the FEN string
        std::string fen = parts.front();
        parts.pop_front();

        // Split FEN into space-separated parts
        std::deque<std::string> fenParts = split(fen, ' ');

        // Load FEN into the board
        board.loadFromFEN(fenParts);

        // Iterate over perft entries
        bool allPassed = true;
        std::cout << "Testing position: " << fen << std::endl;

        for (const auto& perftEntry : parts) {
            // Trim leading spaces
            std::string entry = perftEntry;
            size_t firstNonSpace = entry.find_first_not_of(" ");
            if (firstNonSpace != std::string::npos) {
                entry = entry.substr(firstNonSpace);
            }

            // Expecting format Dx N (e.g., D1 4)
            if (entry.empty()) continue;

            std::deque<std::string> entryParts = split(entry, ' ');
            if (entryParts.size() != 2) {
                std::cerr << "Invalid perft entry format: \"" << entry << "\"" << std::endl;
                continue;
            }

            std::string depthStr = entryParts[0];
            std::string expectedStr = entryParts[1];

            if (depthStr.size() < 2 || depthStr[0] != 'D') {
                std::cerr << "Invalid depth format: \"" << depthStr << "\"" << std::endl;
                continue;
            }

            // Extract depth and expected node count
            int depth = std::stoi(depthStr.substr(1));
            u64 expectedNodes = std::stoull(expectedStr);

            // Execute perft for the given depth
            u64 actualNodes = _perft(board, depth);

            nodes += actualNodes;

            // Compare and report
            bool pass = (actualNodes == expectedNodes);
            std::cout << "Depth " << depth << ": Expected " << expectedNodes
                << ", Got " << actualNodes << " -> "
                << (pass ? "PASS" : "FAIL") << std::endl;

            totalTests++;
            if (pass) passedTests++;
            else allPassed = false;
        }

        int nps = (int)(((double)nodes) / (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) * 1000000000);
        double timeTaken = (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count());
        if (timeTaken / 1000000 < 1000) std::cout << "Time taken: " << timeTaken / 1000000 << " milliseconds" << std::endl;
        else std::cout << "Time taken: " << timeTaken / 1000000000 << " seconds" << std::endl;
        std::cout << "Generated moves with " << formatNum(nodes) << " nodes and NPS of " << formatNum(nps) << std::endl;

        if (allPassed) {
            std::cout << "All perft tests passed for this position." << std::endl;
        }
        else {
            std::cout << "Some perft tests failed for this position." << std::endl;
        }

        std::cout << "----------------------------------------" << std::endl << std::endl;
    }

    std::cout << "Perft Suite Completed: " << passedTests << " / "
        << totalTests << " tests passed." << std::endl;
}

int main() {
    Precomputed::compute();
    initializeAllDatabases();
    std::string command;
    std::deque<std::string> parsedcommand;
    Board currentPos;
    currentPos.reset();
    currentPos.generateLegalMoves();
    std::atomic<bool> breakFlag(false);
    std::optional<std::thread> searchThreadOpt;
    std::cout << "Bot ready and awaiting commands" << std::endl;

    while (true) {
        std::getline(std::cin, command);
        parsedcommand = split(command, ' ');
        if (!parsedcommand.empty() && parsedcommand.at(0) == "position") { // Handle "position" command
            currentPos.reset();
            if (parsedcommand.size() > 3 && parsedcommand.at(2) == "moves") { // "position startpos moves ..."
                for (size_t i = 3; i < parsedcommand.size(); i++) {
                    currentPos.move(parsedcommand.at(i));
                }
            }
            else if (parsedcommand.at(1) == "fen") { // "position fen ..."
                parsedcommand.pop_front(); // Pop 'position'
                parsedcommand.pop_front(); // Pop 'fen'
                currentPos.loadFromFEN(parsedcommand);
            }
            if (parsedcommand.size() > 6 && parsedcommand.at(6) == "moves") {
                for (size_t i = 7; i < parsedcommand.size(); i++) {
                    currentPos.move(parsedcommand.at(i));
                }
            }
        }
        else if (command == "d") {
            currentPos.display();
        }
        else if (parsedcommand.size() > 1 && parsedcommand.at(0) == "move") {
            currentPos.move(parsedcommand.at(1));
        }
        else if (command == "quit") {
            breakFlag.store(true);
            // Ensure the search thread is joined before exiting
            if (searchThreadOpt.has_value()) {
                if (searchThreadOpt->joinable()) {
                    searchThreadOpt->join();
                }
            }
            return 0;
        }
        else if (command == "debug.gamestate") {
            std::string bestMoveAlgebra;
            std::cout << "Is in check (white): " << currentPos.isInCheck(WHITE) << std::endl;
            std::cout << "Is in check (black): " << currentPos.isInCheck(BLACK) << std::endl;
            std::cout << "En passant index (64 if none): " << ctzll(currentPos.enPassant) << std::endl;
            std::cout << "Castling rights: " << std::bitset<4>(currentPos.castlingRights) << std::endl;
            std::cout << "Legal moves (current side to move):" << std::endl;
            MoveList moves = currentPos.generateLegalMoves();
            for (int i = 0; i < moves.count; i++) {
                std::cout << moves.moves[i].toString(currentPos) << std::endl;
            }
        }
        else if (parsedcommand.at(0) == "perft") {
            auto start = std::chrono::high_resolution_clock::now();
            u64 nodes = perft(currentPos, stoi(parsedcommand.at(1)));
            int nps = (int)(((double)nodes) / (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) * 1000000000);
            double timeTaken = (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count());
            if (timeTaken / 1000000 < 1000) std::cout << "Time taken: " << timeTaken / 1000000 << " milliseconds" << std::endl;
            else std::cout << "Time taken: " << timeTaken / 1000000000 << " seconds" << std::endl;
            std::cout << "Generated moves with " << formatNum(nodes) << " nodes and NPS of " << formatNum(nps) << std::endl;
        }
        else if (parsedcommand.at(0) == "perftsuite") {
            perftSuite(parsedcommand.at(1));
        }
        else if (command == "debug.moves") {
            std::cout << "All moves (current side to move):" << std::endl;
            auto moves = currentPos.generateMoves();
            for (int i = 0; i < moves.count; i++) {
                std::cout << moves.moves[i].toString(currentPos) << std::endl;
            }
        }
    }
    return 0;
}
