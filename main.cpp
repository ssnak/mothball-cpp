#include <cstdio>

#include "player.h"

int main() {
    Args a{};
    Player player;
    player.inputs = "w";
    player.sprint(12, a);
    player.sprint(12, a);
    printf("%f %f\n", player.velocity.z, player.position.z);
}
