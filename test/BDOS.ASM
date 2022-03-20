        JMP INIT
        NOP
        NOP
        NOP             ; 0x0000 top of stack
        NOP
        MOV A,C
        CPI 2
        JZ PCHR
        CPI 9
        JZ PSTR
        RET
PCHR:   MOV A,E
        OUT 0
        RET
PSTR:   LDAX DE
        CPI '$'
        JNZ LOOP
        RET
LOOP:   OUT 0
        INX DE
        JMP PSTR
INIT:   IN 0
        CPI 0
        JZ 100H
        HLT
