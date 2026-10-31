\\DATA 100

main:   MOV EAX, 5
        MOV EBX, 10
        SYS 0x0F            ; breakpoint 1: EAX=5, EBX=10
        ADD EAX, EBX
        MOV l[DS+0], EAX
        SYS 0x0F            ; breakpoint 2: EAX=15, DS+0 = 15
        SUB EAX, 20
        MOV l[DS+4], EAX
        SYS 0x0F            ; breakpoint 3: EAX=-5, DS+4 = -5
        STOP
