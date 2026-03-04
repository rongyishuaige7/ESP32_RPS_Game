#include "GameLogic.h"
#include "esp_random.h"

GameLogic::GameLogic() {}

Gesture GameLogic::generatePCGesture() {
    return (Gesture)((esp_random() % 3) + 1);
}

GameResult GameLogic::determineWinner(Gesture player, Gesture pc) {
    if (player == pc) return RESULT_DRAW;

    if ((player == GESTURE_STONE && pc == GESTURE_SCISSORS) ||
        (player == GESTURE_SCISSORS && pc == GESTURE_PAPER) ||
        (player == GESTURE_PAPER && pc == GESTURE_STONE)) {
        return RESULT_PLAYER_WIN;
    }

    return RESULT_PC_WIN;
}

const char* GameLogic::gestureToEmoji(Gesture g) {
    switch(g) {
        case GESTURE_STONE: return "R";     // Rock
        case GESTURE_SCISSORS: return "S"; // Scissors
        case GESTURE_PAPER: return "P";    // Paper
        default: return "?";
    }
}

const char* GameLogic::resultToString(GameResult r) {
    switch(r) {
        case RESULT_PLAYER_WIN: return "You Win!";
        case RESULT_PC_WIN: return "PC Wins!";
        case RESULT_DRAW: return "Draw!";
        default: return "?";
    }
}
