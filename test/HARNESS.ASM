        JMP INIT
        NOP
        NOP
        MOV A,C     ; If C == 9 WRITESTR else return
        CPI 9
        JZ WSTR
        RET
WSTR:   LDAX DE
        CPI '$'     ; If A != '$' loop else return
        JNZ LOOP
        RET
LOOP:   OUT 0
        INX DE
        JMP WSTR
INIT:   IN 0
        CPI 0       ; If A == 0 run COM else halt
        JZ 100H
        HLT
