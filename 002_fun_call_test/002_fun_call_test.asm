.LC1:
        .string "%s\r\n"
funb():
        stp     x29, x30, [sp, -32]!
        mov     x29, sp
        adrp    x0, .LC0
        add     x1, x0, :lo12:.LC0
        add     x0, sp, 16
        ldr     x2, [x1]
        str     x2, [x0]
        ldr     x1, [x1, 5]
        str     x1, [x0, 5]
        add     x0, sp, 16
        mov     x1, x0
        adrp    x0, .LC1
        add     x0, x0, :lo12:.LC1
        bl      printf
        nop
        ldp     x29, x30, [sp], 32
        ret
.LC0:
        .string "hello,world!"
funa():
        stp     x29, x30, [sp, -16]!
        mov     x29, sp
        bl      funb()
        mov     w0, 0
        ldp     x29, x30, [sp], 16
        ret
fun():
        stp     x29, x30, [sp, -32]!
        mov     x29, sp
        mov     w0, 10
        str     w0, [sp, 28]
        mov     w0, 12
        str     w0, [sp, 24]
        mov     w0, 255
        str     w0, [sp, 20]
        bl      funa()
        mov     w0, 0
        ldp     x29, x30, [sp], 32
        ret
main:
        stp     x29, x30, [sp, -16]!
        mov     x29, sp
        bl      fun()
        mov     w0, 0
        ldp     x29, x30, [sp], 16
        ret
