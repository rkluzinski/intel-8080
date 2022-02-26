        JMP INIT
        NOP
        NOP
        MOV A,C     ; If C == 9 WRITESTR else return
        CPI 9
        JZ WSTR
        RET
WSTR:   XCHG
COND:   MOV A,M
        CPI '$'     ; If A != '$' loop else return
        JNZ LOOP
        XCHG
        RET
LOOP:   OUT 0
        INX HL
        JMP COND
INIT:   IN 0
        CPI 0       ; If A == 0 run COM else halt
        JZ 100H
        HLT
