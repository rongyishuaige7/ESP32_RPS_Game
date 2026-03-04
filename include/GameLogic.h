#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdint.h>

enum Gesture {
    GESTURE_NONE = 0,
    GESTURE_STONE = 1,      // 石头 ✊
    GESTURE_SCISSORS = 2,  // 剪刀 ✌️
    GESTURE_PAPER = 3      // 布 ✋
};

enum GameResult {
    RESULT_PLAYER_WIN,
    RESULT_PC_WIN,
    RESULT_DRAW
};

class GameLogic {
public:
    GameLogic();
    Gesture generatePCGesture();
    GameResult determineWinner(Gesture player, Gesture pc);
    const char* gestureToEmoji(Gesture g);
    const char* resultToString(GameResult r);
};

#endif
