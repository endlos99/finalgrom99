/*
 * FinalGROM 99
 *
 * A cartridge for the TI 99 that allows for running ROM and GROM images
 * stored on an SD card
 *
 * Copyright (c) 2017 Ralph Benzinger <r@0x01.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
  *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */


#include <stdint.h>

#include "loader.h"


extern uint8_t selection[];


int main(void)
{
  uint8_t regen = 1;
  uint8_t sel = SEL_PROGRAM;

  // setup cart
  setup();

  while (1) {
    // create menu loader
    if (regen && genMenu())
      break;  // load single image immediately
    // wait for image selection
    sel = selectImage(0);
    if (sel == SEL_PROGRAM)
      break;  // load program
    else if (sel == SEL_HELP)
      regen = loadHelp();  // show help file
    else
      regen = changeDir();  // cd into selected folder
  }

  // load selected image, and wait for potential reload or dump
  while (1) {
    if (sel == SEL_DUMP)
      dumpImage();
    else
      loadImage();
    sel = selectImage(1);
  }
}
