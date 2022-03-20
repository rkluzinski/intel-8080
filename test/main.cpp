#include <fstream>
#include <iostream>
#include <string>

#include "emulator.h"

#include "BDOS.h"

#ifdef TST8080
#include "TST8080.h"
unsigned char *test_bin = coms_TST8080_COM;
unsigned int test_len = coms_TST8080_COM_len;
#endif

#ifdef CPUTEST
#include "CPUTEST.h"
unsigned char *test_bin = coms_CPUTEST_COM;
unsigned int test_len = coms_CPUTEST_COM_len;
#endif

#ifdef _8080PRE
#include "8080PRE.h"
unsigned char *test_bin = coms_8080PRE_COM;
unsigned int test_len = coms_8080PRE_COM_len;
#endif

#ifdef _8080EXM
#include "8080EXM.h"
unsigned char *test_bin = coms_8080EXM_COM;
unsigned int test_len = coms_8080EXM_COM_len;
#endif

uint8_t in_callback(uint8_t port) {
    static uint8_t halt = 0;
    if (port == 0) {
        return halt++;
    }
    return 0;
}

void out_callback(uint8_t port, uint8_t A) {
    if (port == 0) {
        std::cout << A;
    }
}

int main() {
    Intel8080 i8080;
    i8080.in_callback = in_callback;
    i8080.out_callback = out_callback;
    // Load mock CPM BDOS at address 0
    for (uint16_t i = 0; i < bin_BDOS_len; i++) {
        i8080.memory[i] = bin_BDOS[i];
    }
    // Load test com at address 0x100
    for (uint16_t i = 0; i < test_len; i++) {
        i8080.memory[i + 0x100] = test_bin[i];
    }
    try {
        i8080.execute();
        std::cout << std::endl;
    } catch (std::runtime_error &e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}