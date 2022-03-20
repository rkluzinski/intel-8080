# Intel 8080

An emulator for the Intel 8080 microprocessor that passes all available CPU tests and is used to emulate the space invaders arcade cabinet. Also includes a partly finished assembler for Intel 8080 assembly code.

## Building

Requirements
* C++20
* SDL2
* Space Invader's ROMs

Download the four Space Invaders ROM files and copy them into the roms folder. Next, create the `bin` folder and build the project using `make`. This will build the test binaries and the `invaders` binary.

## Testing

The test binaries will be created in the `bin` folder. Run the binaries to run the tests.

## Usage

Run the `invaders` binary to play Space Invaders. Controls are 'c' to insert coins, enter to start, arrow keys to move and space to fire. Controls for 2P are not bound to any keys.

# References
* https://altairclone.com/downloads/manuals/8080%20Programmers%20Manual.pdf
* https://pastraiser.com/cpu/i8080/i8080_opcodes.html
* https://altairclone.com/downloads/cpu_tests/
* https://www.computerarcheology.com/Arcade/SpaceInvaders/Hardware.html
* http://dunfield.classiccmp.org/r/8080.txt
* http://www.emulator101.com