#include "../MenuFlow.h"
#include <stdio.h>
using namespace tower;
static int failures=0;
static void expect(bool pass,const char* title) {
    printf("%s %s\n",pass?"PASS":"FAIL",title);
    if(!pass) ++failures;
}
int main() {
    MenuFlow flow;
    expect(flow.screen()==MenuFlow::MAIN_MENU,"starts at main menu");
    expect(flow.selection()==MenuFlow::PLAY,"Play selected by default");
    flow.down();
    expect(flow.selection()==MenuFlow::EXIT,"Down highlights Exit");
    flow.up();
    expect(flow.selection()==MenuFlow::PLAY,"Up highlights Play");
    flow.activate();
    expect(flow.screen()==MenuFlow::PLAYING,"Play enters game");
    flow.down(); flow.activate();
    expect(flow.screen()==MenuFlow::PLAYING,"Menu navigation ignored during gameplay");
    flow.backToMenu();
    expect(flow.screen()==MenuFlow::MAIN_MENU &&
           flow.selection()==MenuFlow::PLAY,"return to menu selects Play");
    flow.down(); flow.activate();
    expect(flow.screen()==MenuFlow::EXIT_REQUESTED,"Exit requests application shutdown");
    flow.backToMenu(); flow.activate();
    expect(flow.screen()==MenuFlow::EXIT_REQUESTED,"Exit is terminal");
    {
        MenuFlow game;
        game.activate();
        game.lose();
        expect(game.screen()==MenuFlow::GAME_OVER,"lose transitions into game-over screen");
        game.lose();
        expect(game.screen()==MenuFlow::GAME_OVER,"repeated loss is idempotent");
        game.down(); game.activate();
        expect(game.screen()==MenuFlow::GAME_OVER,"menu input ignored on game-over screen");
        game.retry();
        expect(game.screen()==MenuFlow::PLAYING,"retry resumes gameplay state");
        game.lose();
        game.backToMenu();
        expect(game.screen()==MenuFlow::MAIN_MENU &&
               game.selection()==MenuFlow::PLAY,"Triangle returns to menu from game over");
        game.retry();
        expect(game.screen()==MenuFlow::MAIN_MENU,"retry ignored on menu");
    }
    printf("Menu flow tests: %s\n",failures?"FAILED":"PASSED");
    return failures?1:0;
}
