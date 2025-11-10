#include <cstdio>
#include <optional>

#include "player.h"

int main() {
    Player player;
    player.inputs = "w";
    player.sprint(12, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
    printf("%f %f\n", player.velocity.z, player.position.z);
}
