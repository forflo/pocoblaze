        ;;
        ;; @author: FM.
        ;; all instructions of picoblaze of the 16 bit version
        ;;,,,,,,,,,

        ;; arithmetic
        add s7, ff              ; s7 <- s7 + ff
        add s7, s7              ; s7 <- s7 + s7

        addcy s7, ff            ; s7 <- s7 + ff + CARRY
        addcy s7, s7            ; s7 <- s7 + s7 + CARRY

        sub s7, ff              ; s7 <- s7 - ff
        sub s7, s7              ; s7 <- s7 - s7

        subcy s7, ff            ; s7 <- s7 - ff - CARRY
        subcy s7, s7            ; s7 <- s7 - s7 - CARRY

        ;; logical
        and s7, ff              ; s7 <- s7 AND ff
        and s7, s7              ; s7 <- s7 AND s7

        or s7, ff               ; s7 <- s7 OR ff
        or s7, s7               ; s7 <- s7 OR s7

        xor s7, ff              ; s7 <- s7 XOR ff
        xor s7, s7              ; s7 <- s7 XOR s7

        ;; picoblaze assembler can't do this
        ;; compare s7, ff          ; if s7 = ff then ZERO <- 1
                                ; if s7 < ff then CARRY <- 1

        ;; compare s7, s7          ; if s7 = s7 then ZERO <- 1
                                ; if s7 < s7 then CARRY <- 1

        test s7, ff             ; if (s7 AND ff) = 0 then ZERO <- 1,
                                ;   CARRY <- odd parity of (s7 AND ff)

        test s7, s7             ; if (s7 AND s7) = 0 then ZERO <- 1
                                ;   CARRY <- odd parity of (s7 AND s7)

        ;; rotation
        ;; done
        RL s7                   ; s7 <- {s7[6:0],s7[7]}, CARRY <- s7[7]
        RR s7                   ; s7 <- {s7[0],s7[7:1]}, CARRY <- s7[0]
        SL0 s7                  ; s7 <- {s7[6:0],0}, CARRY <- s7[7]
        SL1 s7                  ; s7 <- {s7[6:0],1}, CARRY <- s7[7], ZERO <- 0

        SLA s7                  ; s7 <- {s7[6:0],CARRY}, CARRY <- s7[7]
        SLX s7                  ; s7 <- {s7[6:0],s7[0]}, CARRY <- s7[7]

        SR0 s7                  ; s7 <- {0,s7[7:1]}, CARRY <- s7[0]
        SR1 s7                  ; s7 <- {1,s7[7:1]}, CARRY <- s7[0], ZERO <- 0

        
        SRA s7                  ; s7 <- {CARRY, s7[7:1]}, CARRY <- s7[0]
        SRX s7                  ; s7 <- {s7[7], s7[7:1]}, CARRY <- s7[0]
        

        ;; branch commands
        call fnord              ; unconditionally call fnord

        call C, fnord           ; if CARRY = 1 then {TOS <- PC, PC <- fnord}
        call NC, fnord          ; if CARRY = 0 then {TOS <- PC, PC <- fnord}
        call Z, fnord           ; if ZERO = 1 then {TOS <- PC, PC <- fnord}
        call NZ, fnord          ; if ZERO = 0 then {TOS <- PC, PC <- fnord}

        jump fnord              ; PC <- fnord
        jump C fnord            ; if CARRY = 1 then PC <- fnord
        jump NC fnord           ; if CARRY = 0 then PC <- fnord
        jump Z fnord            ; if ZERO = 1 then PC <- fnord
        jump NZ fnord           ; if ZERO = 0 then PC <- fnord

        return                  ; PC <- TOS + 1
        return C                ; if CARRY = 1 then PC <- TOS + 1 else NOP
        return NC               ; if CARRY = 0 then PC <- TOS + 1 else NOP
        return Z                ; if ZERO = 1 then PC <- TOS + 1 else NOP
        return NZ               ; if ZERO = 0 then PC <- TOS + 1 else NOP

        ;; storage functions
        load s7, ff             ; s7 <- ff
        load s7, s7             ; s7 <- s7

        ;; fetch and store not implemented by assembler
        ;; fetch s6, s7            ; s6 <- RAM[(s7)]
        ;; fetch s6, ff            ; s6 <- RAM[ff]

        ;; store s7, s7            ; RAM[(s7)] <- s7
        ;; store s7, ff            ; RAM[ff] <- s7

        ;; returni ENABLE and DISABLE omitted
        ;; input omitted
        ;; output omitted

fnord:  call fnord
