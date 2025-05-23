/*
 * Copyright 2024 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include "system/logging.h"
//#include "drivers/imu/lis3dh/lis3dh.h"
#include "drivers/imu/mmc5603/mmc5603.h"


void imu_init(void) {
  // Init accelerometer
  //lis3dh_init();
  // Init magnetometer
  mmc5603_init();
}

void imu_power_up(void) {
  //lis3dh_power_up();
}

void imu_power_down(void) {
  //lis3dh_power_down();
}
