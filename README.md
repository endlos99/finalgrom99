FinalGROM 99
============

The *TI 99/4A FinalGROM Cartridge*, or FinalGROM 99 for short, is a
cartridge for the TI 99/4A home computer that allow you to run ROM and GROM
cartridge images from an SD card.  It succeeds the [FlashROM 99][3] released
in 2016.

![The FinalGROM 99 Cartridge](/doc/finalgrom99.jpg)

The FinalGROM 99 supports ROM images, GROM images, and mixed images of up to
1 MB in size that use the write-to-ROM bank switching scheme.  The cartridge
does not require the Peripheral Expansion Box and runs on both PAL and NTSC
consoles, including modified consoles with an F18A.  It will also run on
v2.2 consoles and enables those to run ROM-only programs.

The cartridge offers some advanced modes that provide RAM and GRAM modes.
It also allows a running program to reload another program from SD card.
All firmware of the FinalGROM 99 can be updated by SD card.

The FinalGROM 99 is released as Open Source Hardware under the
[CERN OHL license][5] and the [GNU GPL license][6].  Both hardware design
files and firmware sources are available on [GitHub][2].

The project homepage with detailed instructions is located at [GitHub][1].


Using the FinalGROM 99
----------------------

Using the FinalGROM 99 is simple and doesn't require any special hardware or
software.

To begin, copy some cartridge dumps or other images files onto an SD or SDHC
card.  You can create directories to organize your programs.  Switch off the
TI 99 and plug in the FinalGROM 99, then insert the SD card into the
FinalGROM 99.  Switch on the TI 99 and wait until the activity indicator on
the FinalGROM 99 is no longer lit.

Press any key to bring up the TI menu screen.  You should see the `FINALGROM
99` entry.  Select it to start the image browser, where you can page through
the list of available image entries with `,`, `.`, `SPACE`, and number keys.
When you descend into a folder, the FinalGROM will reload its contents from
the SD card.  To go back up, select the special folder `[ .. ]`.

![Image selection](/doc/selection.png)

Select the image you want to run.  The screen will show a loader animation
while the image loads.  Once the image has been loaded, it will start
automatically.  The SD card is now no longer required and may be removed.

If only one image is found on the SD card, it is loaded immediately without
the need to select it first.  This method ensures maximum compatibility.
Note that ROM images loaded as single-image SD card will not run on v2.2
consoles!

If you want to run a different program from the SD card, reset the TI 99 by
pressing `FCTN-=` and then reset the FinalGROM 99 by pushing the reset
button.  If you do not reset the FinalGROM 99, the TI menu will show only
the previously selected image.  Alternatively, you can power cycle the
console, which will reset both TI 99 and FinalGROM 99.

For detailed usage information please refer to the [FinalGROM 99 homepage][1].


Building the FinalGROM 99
-------------------------

The [GitHub repository][2] will soon contain all hardware design files and
software sources required to build the FinalGROM 99.


About the Project
-----------------

The TI 99/4A FinalGROM Cartridge is Open Source Hardware released under the
[CERN OHL license][5], in the hope that TI 99 enthusiasts may find it
useful.  Software components are released under the [GNU GPL license][6].

The microcontroller code uses a modified version of the [Petit FatFs][10]
library and an adapted version of the Xilinx [Application Note 58][11] code.

Contributions to both hardware and software are very welcome!  Please email
feedback, support questions, inquiries for parts, and bug reports to the
developer at <r@0x01.de>.


[1]: https://endlos99.github.io/finalgrom99
[2]: https://github.com/endlos99/finalgrom99
[3]: https://endlos99.github.io/flashrom99
[4]: https://endlos99.github.io/xdt99
[5]: http://www.ohwr.org/projects/cernohl/wiki
[6]: http://www.gnu.org/licenses/gpl.html
[7]: http://kicad-pcb.org
[8]: http://www.nongnu.org/avrdude/
[9]: https://www.xilinx.com/products/design-tools/ise-design-suite/ise-webpack.html
[10]: http://elm-chan.org/fsw/ff/00index_p.html
[11]: http://www.xilinx.com/support/documentation/application_notes/xapp058.pdf
[12]: SMD video tut.
