#ifndef ENUMS_H
#define ENUMS_H

#include <Misc/Enum.h>

enum class LEDs{ HeadlightsLeft, HeadlightsRight, TaillightsLeft, TaillightsRight, COUNT};

DECLARE_ENUM(Button, Pair);

enum class RGB_LEDs{ Left1, Left2, Left3, Right1, Right2, Right3, COUNT};

DECLARE_ENUM(MotorsEnum, Motor);

DECLARE_ENUM(ServoEnum, Steer);

#endif //ENUMS_H
