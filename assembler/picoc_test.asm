        ;;
        ;; @author: FM.
        ;; all instructions of picoblaze of the 16 bit version
        ;;,,,,,,,,,

        ;; arithmetic

        ;; test RL

        jump main               ;0

fnord:  load s6, ff             ;1
        load s7, 11             ;2
        return                  ;3

main:
        call fnord              ;4
        test s6, s7             ;5
