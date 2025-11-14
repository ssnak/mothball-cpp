#include <iostream>

#include "player.h"
#include "stratfinder.h"

int main() {
    StratFinder stratfinder;
    stratfinder.findStrat();
    std::cout << stratfinder.duration << std::endl << stratfinder.rotation << std::endl << stratfinder.player;
    StratFinder stratfinder2;
    stratfinder2.left = false;
    stratfinder2.findStrat();
    std::cout << stratfinder2.duration << std::endl << stratfinder2.rotation << std::endl << stratfinder2.player;
}
