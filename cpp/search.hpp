#include "board.hpp"
#include "hash.hpp"
#include "heuristic.hpp"
#include "move.hpp"

Hash hash;

class Search {
public:
  Search(int minDepth) { MIN_DEPTH = minDepth; }

  float operator()(board_t s, int moveDir) {
    board_t newBoard = move(s, moveDir);
    if (newBoard == s)
      return 0;
    stateEvaled = 0;
    unsigned currentDepth = MIN_DEPTH;
    // int diff = CountDistinct(s);
    //  if (diff >= 12) currentDepth += (1 << (diff - 12));
    minProb = 1.0f / float(1 << (2 * currentDepth + 5));
    float result = ExpectimaxSpawnNode(newBoard, currentDepth, 1.0f);
    int minState = 1 << (3 * currentDepth + 5);
    int lastStates = 0;
    while ((stateEvaled < minState) && (stateEvaled > lastStates)) {
      ++currentDepth;
      minProb = 1.0f / float(1 << (2 * currentDepth + 5));
      minState *= 2;
      lastStates = stateEvaled;
      stateEvaled = 0;
      result = ExpectimaxSpawnNode(newBoard, currentDepth, 1.0f);
    }
    return result;
  }

private:
  Heuristic heuristic;
  Move move;
  int stateEvaled = 0;
  int MIN_DEPTH;
  float minProb;

  float ExpectimaxSpawnNode(board_t s, int depth, float prob) {
    if (depth <= 0 || prob < minProb)
      return heuristic(s);
    float expect = 0.0;
    int currentEvaled = stateEvaled;
    stateEvaled += hash.Lookup(s, depth, &expect);
    if (stateEvaled > currentEvaled)
      return expect;
    expect = 0.0f;
    float emptyTiles = CountEmpty(s);
    float prob2 = prob * .9 / emptyTiles;
    float prob4 = prob * .1 / emptyTiles;
    board_t tmp = s;
    for (board_t tile2 = 1; tile2; tile2 <<= 4) {
      if (!(tmp & 0xf)) {
        expect += ExpectimaxMoveNode(s | tile2, depth - 1, prob2) * .9;
        expect += ExpectimaxMoveNode(s | (tile2 << 1), depth - 1, prob4) * .1;
      }
      tmp >>= 4;
    }
    hash.Update(s, depth, expect / emptyTiles, stateEvaled - currentEvaled);
    return expect / emptyTiles;
  }

  float ExpectimaxMoveNode(board_t s, int depth, float prob) {
    ++stateEvaled;
    float max = Heuristic::LOSE_PENALTY;
    for (int i = 0; i < 4; ++i) {
      board_t newBoard = move(s, i);
      if (newBoard != s)
        max = std::max(max, ExpectimaxSpawnNode(newBoard, depth, prob));
    }
    return max;
  }
};
