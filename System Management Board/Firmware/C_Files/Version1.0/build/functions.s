.thumb_func

.align 4
.word RETURN_ADDR
.equ MAX_PIN, -29

@R0 has input from which pins to activate
state_enforce:  LDR R6, =RETURN_ADDR
                STR R7, [R6]
                MOV R3, #0
                ADD R3, R3, R0
                MOV R1, #1
                MOV R2, #1
loop1:          BL gpio_put_wrapper
                ADD R1, R1, #1
                ADD R2, R2, R2
                MOV R0, #0
                ADD R0, R0, R3
                AND R0, R2, R0
                Beq skip
                MOV R0, #1
skip:           LDR R4, =MAX_PIN
                LDR R4, [R4]
                ADD R4, R1, R4
                Beq loop1
                LDR R7, =RETURN_ADDR
                LDR R7, [R7]
                B R7
