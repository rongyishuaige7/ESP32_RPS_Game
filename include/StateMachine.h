#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "DisplayManager.h"
#include "ButtonManager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

enum GameState {
    STATE_IDLE,
    STATE_COUNTDOWN,
    STATE_PLAYING,
    STATE_RESULT
};

class StateMachine {
public:
    StateMachine(DisplayManager* display, ButtonManager* buttons);
    void update();
    void reset();

private:
    GameState currentState;
    DisplayManager* display;
    ButtonManager* buttons;
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
