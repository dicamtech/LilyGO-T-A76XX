/**
 * @file      main.ino
 * @author    Ibrahim Hroob (ibrahim.hroub7@gmail.com)
 * @copyright Copyright (c) 2024  Dicam Technology Ltd
 * @date      2024-12-25
 *
 */

#include "src/app.h"

BrooderAlarmGateway bag;

void setup(){
    bag.Init();
}

void loop(){
    bag.Run();
}
