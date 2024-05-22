#include <algorithm>
#include <functional>
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <bitset>
#include <set>
#include <stdio.h>
#include <string>
#if RAND_MAX/256 >= 0xFFFFFFFFFFFFFF
#define LOOP_COUNT 1
#elif RAND_MAX/256 >= 0xFFFFFF
#define LOOP_COUNT 2
#elif RAND_MAX/256 >= 0x3FFFF
#define LOOP_COUNT 3
#elif RAND_MAX/256 >= 0x1FF
#define LOOP_COUNT 4
#else
#define LOOP_COUNT 5
#endif
#define CHECK_BIT(var,pos) (((var)>>(pos)) & 1)
#define IS_BISHOP true
#define MAX_USED 10
#define ITERATIONS 1000000
#define START_INDEX 0
#define END_INDEX 63
using namespace std;

struct magicNumber {
    uint64_t number;
    int used, collisions, maxIndex;
    bool operator<(const magicNumber& other) const {
        if (used == other.used) {
            if (collisions == other.collisions) {
                if (maxIndex == other.maxIndex) {
                    return number < other.number;
                } return maxIndex < other.maxIndex;
            } return collisions > other.collisions;
        } return used < other.used;
    }
    magicNumber() {
        number = 0;
        used = 0;
        collisions = 0;
        maxIndex = 0;
    }
    magicNumber(const uint64_t &number, const int &used) {
        this->number = number;
        this->used = used;
        collisions = 0;
        maxIndex = 0;
    }
};

std::vector<int> indices;
std::vector<magicNumber> magicNumbers;
std::vector<uint64_t> blockerBitboards;

uint64_t genRand() {
    uint64_t r = 0;
    for (int i = LOOP_COUNT; i > 0; i--) {
        r = r * (RAND_MAX + (uint64_t)1) + rand();
    }
    return r;
}

uint64_t genRandWFewBits() {
    return genRand() & genRand() & genRand();
}

template<typename Type> inline Type powOfTwo(uint_fast8_t power) {
    if (power == 0) return 1;
    return (Type(1) << power);
}

void niceOutput(uint64_t bitboard, uint_fast8_t startSqr) {
    uint_fast8_t counter = 0;
    for (uint_fast8_t i = 0; i < 8; i++) {
        cout << "\t";
        for (uint_fast8_t j = 0; j < 8; j++) {
            if (CHECK_BIT(bitboard, counter)) cout << "X";
            else if (counter == startSqr) cout << "R";
            else cout << ".";
            counter++;
        } cout << "\n";
    } cout << "\n\n";
}

void greatOutput(const uint64_t bitboardArray[64], string pieceName) {
    for (int i = 0; i < 64; i++) {
        std::cout << pieceName << " Moves[" << i << "] = \n";
        niceOutput(bitboardArray[i], i);
    } cout << "\n\n\n";
}

bool sameMSB(const uint64_t &first, const uint64_t &second, int square) {
    if (IS_BISHOP) {
        int_fast8_t row = (square >> 3) << 3;
        int_fast8_t column = (square & 7);

        for (int_fast8_t i = square + 7; (((i & 7) <= 7) && ((i >> 3) <= 7)); i += 7) {
            bool one = CHECK_BIT(first, i), two = CHECK_BIT(second, i);
            if (one == 0 && two == 0) continue;
            if (one == two) break;
            return 0;
        }
        for (int_fast8_t i = square - 7; (((i & 7) >= 0) && ((i >> 3) >= 0)); i -= 7) {
            bool one = CHECK_BIT(first, i), two = CHECK_BIT(second, i);
            if (one == 0 && two == 0) continue;
            if (one == two) break;
            return 0;
        }
        for (int_fast8_t i = square - 9; (((i & 7) <= 7) && ((i >> 3) >= 0)); i -= 9) {
            bool one = CHECK_BIT(first, i), two = CHECK_BIT(second, i);
            if (one == 0 && two == 0) continue;
            if (one == two) break;
            return 0;
        }
        for (int_fast8_t i = square + 9; (((i & 7) >= 0) && ((i >> 3) <= 7)); i += 9) {
            bool one = CHECK_BIT(first, i), two = CHECK_BIT(second, i);
            if (one == 0 && two == 0) continue;
            if (one == two) break;
            return 0;
        }
    } else {
        int_fast8_t row = (square >> 3) << 3;
        int_fast8_t column = (square & 7);

        for (int_fast8_t i = column - 1; i > 0; i--) {
            int_fast8_t index = row + i;
            bool one = CHECK_BIT(first, index), two = CHECK_BIT(second, index);
            if (one == 0 && two == 0) continue;
            if (one == two) break;
            return 0;
        }
        for (int_fast8_t i = column + 1; i < 7; i++) {
            int_fast8_t index = row + i;
            bool one = CHECK_BIT(first, index), two = CHECK_BIT(second, index);
            if (one == 0 && two == 0) continue;
            if (one == two) break;
            return 0;
        }
        for (int_fast8_t i = ((square >> 3) - 1); i > 0; i--) {
            int_fast8_t index = (i << 3) + column;
            bool one = CHECK_BIT(first, index), two = CHECK_BIT(second, index);
            if (one == 0 && two == 0) continue;
            if (one == two) break;
            return 0;
        }
        for (int_fast8_t i = ((square >> 3) + 1); i < 7; i++) {
            int_fast8_t index = (i << 3) + column;
            bool one = CHECK_BIT(first, index), two = CHECK_BIT(second, index);
            if (one == 0 && two == 0) continue;
            if (one == two) break;
            return 0;
        }
    } return 1;
}

uint64_t checkMagic(uint64_t random, int used, int square) {
    int shiftAmount = 64 - used;
    magicNumber temp(random, used);
    for (int i = 1; i < blockerBitboards.size() - 1; i++) {
        uint64_t product1 = (random * blockerBitboards[i]) >> shiftAmount;
        if (product1 > temp.maxIndex) temp.maxIndex = product1;
        for (int j = i + 1; j < blockerBitboards.size(); j++) {
            uint64_t product2 = (random * blockerBitboards[j]) >> shiftAmount;
            if (product1 == product2 && blockerBitboards[i] != blockerBitboards[j]) {
                if (sameMSB(blockerBitboards[i], blockerBitboards[j], square)) {
                    temp.collisions += 1;
                    continue;
                } else return 0;
            }
        }
    } magicNumbers.push_back(temp);
    return random;
}

void makeBlockerBitboards(uint64_t allMoves) {
    indices.clear();
    for (uint_fast8_t i = 0; i < 64; i++) if (CHECK_BIT(allMoves, i)) indices.push_back(i);
    int blockerBitboardsAmount = 1 << indices.size();

    blockerBitboards.resize(blockerBitboardsAmount);
    std::fill(blockerBitboards.begin(), blockerBitboards.end(), 0);
    
    for (int n = 0; n < blockerBitboardsAmount; n++) {
        uint64_t temp;
        for (int bitIndex = 0; bitIndex < indices.size(); bitIndex++) {
            int bit = (n >> bitIndex) & 1;
            blockerBitboards[n] |= (uint64_t)bit << indices[bitIndex];
        }
    }
}

int main() {
    srand(static_cast<unsigned int>(time(0)));

    uint64_t universalSet = 0b1111111111111111111111111111111111111111111111111111111111111111;
    uint64_t rookMasks[64];
    uint64_t bishopMasks[64];
    uint64_t precompRookMoves[64];
    uint64_t precompBishopMoves[64];
    uint64_t precompPowOfTwo[64];

    for (uint_fast8_t i = 0; i < 64; i++) {
        precompRookMoves[i] = 0;
        precompBishopMoves[i] = 0;
        rookMasks[i] = 0;
        bishopMasks[i] = 0;
        precompPowOfTwo[i] = powOfTwo<uint64_t>(i);
    }

    for (uint_fast8_t i = 0; i < 64; i++) {
        for (int_fast8_t j = i - 7; ((((j >> 3) < 7)  && ((j & 7) <= 7)) && (((j >> 3) >= 0) && ((j & 7) > 0)));  j -= 7) precompBishopMoves[i] |= precompPowOfTwo[j];
        for (int_fast8_t j = i + 7; ((((j >> 3) <= 7) && ((j & 7) < 7))  && (((j >> 3) > 0)  && ((j & 7) >= 0))); j += 7) precompBishopMoves[i] |= precompPowOfTwo[j];
        for (int_fast8_t j = i - 9; ((((j >> 3) < 7)  && ((j & 7) < 7))  && (((j >> 3) >= 0) && ((j & 7) >= 0))); j -= 9) precompBishopMoves[i] |= precompPowOfTwo[j];
        for (int_fast8_t j = i + 9; ((((j >> 3) <= 7) && ((j & 7) <= 7)) && (((j >> 3) > 0)  && ((j & 7) > 0)));  j += 9) precompBishopMoves[i] |= precompPowOfTwo[j];

        uint_fast8_t row = i >> 3;   //x / 8 == x >> 3
        uint_fast8_t column = i & 7; //x % 8 == x & 7

        bishopMasks[i] = universalSet ^ (0xFF818181818181FF & precompBishopMoves[i]);

        if (row != 0) {
            rookMasks[i] |= powOfTwo<uint64_t>(column);
            for (int_fast8_t j = row - 1; j >= 0; j--)    precompRookMoves[i] |= precompPowOfTwo[(j << 3) + column];
        }
        if (row != 7) {
            rookMasks[i] |= powOfTwo<uint64_t>(column + 56);
            for (int_fast8_t j = row + 1; j <= 7; j++)    precompRookMoves[i] |= precompPowOfTwo[(j << 3) + column];
        }
        if (column != 0) {
            rookMasks[i] |= powOfTwo<uint64_t>(row << 3);
            for (int_fast8_t j = column - 1; j >= 0; j--) precompRookMoves[i] |= precompPowOfTwo[j + (row << 3)];
        }
        if (column != 7) {
            rookMasks[i] |= powOfTwo<uint64_t>((row << 3) + 7);
            for (int_fast8_t j = column + 1; j <= 7; j++) precompRookMoves[i] |= precompPowOfTwo[j + (row << 3)];
        }
        rookMasks[i] ^= universalSet;
    }

    for (int square = START_INDEX; square <= END_INDEX; square++) {
       uint64_t maskedRookMoves = precompRookMoves[square] & rookMasks[square];
       uint64_t maskedBishopMoves = precompBishopMoves[square] & bishopMasks[square];

       //////           uncomment to see masked legal rook moves        //////
       //niceOutput(maskedMoves,1);
       makeBlockerBitboards(maskedBishopMoves);
       //////           uncomment to see blocker bitboards              //////
       /*for (uint64_t i = 0; i < blockerBitboards.size(); i++) {
            cout << bitset<64>(blockerBitboards[i]) << "\n";
            niceOutput(blockerBitboards[i], square);
       }*/

        cout << "Size: " << blockerBitboards.size() << endl;
        std::ofstream file("bishop/logsBishop" + to_string(square) + ".txt");
    
        for (int i = 0; i < ITERATIONS; i++) {
            uint64_t random = genRandWFewBits();
            for (uint_fast8_t j = 1; j <= MAX_USED; j++) {
                uint64_t result = checkMagic(random, j, square);
                if (result != 0) cout << "\n\n\nfound a magic number: " << result << "\tshift: " << 64 - (int)j << "\n\n";
            }
        } std::sort(magicNumbers.begin(), magicNumbers.end());
        for (int i = 0; i < magicNumbers.size(); i++) file << "\nmagic number " << i << ":\t" << (magicNumbers[i].number) << "\tused: " << (int)(magicNumbers[i].used) << "\n" << "collisions: " << magicNumbers[i].collisions << "\t" << "maxIndex: " << magicNumbers[i].maxIndex;
        magicNumbers.clear();
    }
}
