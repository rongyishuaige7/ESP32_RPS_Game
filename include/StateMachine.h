#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "DisplayManager.h"
#include "ButtonManager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class AudioManager;
class LEDManager;
class HandRecognition;
class GameLogic;

enum GameState {
    STATE_IDLE,
    STATE_COUNTDOWN,
    STATE_PLAYING,
    STATE_RESULT
};

class StateMachine {
public:
    StateMachine(DisplayManager* display, ButtonManager* buttons,
                 AudioManager* audio, LEDManager* led,
                 HandRecognition* handRecog, GameLogic* gameLogic);
    void update();
    void reset();
    /** 当前游戏状态（如推流时仅在 IDLE/RESULT 下发帧）。 */
    GameState getState() const { return currentState; }

private:
    GameState currentState;
    DisplayManager* display;
    ButtonManager* buttons;
    AudioManager* audio;
    LEDManager* led;
    HandRecognition* handRecog;
    GameLogic* gameLogic;
    int countdownValue;
    int playerScore;
    int pcScore;
    unsigned long stateStartTime;

    void handleIdle();
    void handleCountdown();
    void handlePlaying();
    void handleResult();
    bool isTimeElapsed(unsigned long ms);
};

#endif
