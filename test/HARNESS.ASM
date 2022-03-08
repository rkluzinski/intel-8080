        JMP INIT
        NOP
        NOP
        MOV A,C     ; If C == 9 WRITESTR else return
        CPI 2
        JZ PCHR
        CPI 9
        JZ PSTR
        RET
PCHR:   MOV A,E
        OUT 0
        RET
PSTR:   LDAX DE
        CPI '$'     ; If A != '$' loop else return
        JNZ LOOP
        RET
LOOP:   OUT 0
        INX DE
        JMP PSTR
INIT:   IN 0
        CPI 0       ; If A == 0 run COM else halt
        JZ 100H
        HLT
