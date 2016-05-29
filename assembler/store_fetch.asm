        ;; test for fetch and store
NAMEREG s2, pointer

        jump main

preload:
        load s0, be
        load s1, ef
        load pointer, 02
        return

main:   call preload
        store s0, 00
        store s1, 01
        store s0, pointer
        add pointer, 01
        store s0, pointer
        add pointer, 01
        store s0, pointer

        fetch s7, pointer
        sub pointer, 3
        fetch s6, pointer
        fetch s5, 00
