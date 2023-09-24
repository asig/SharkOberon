; 32-bit by 32-bit multiplication routine for use with C

    AREA |mul64$$code|, CODE, READONLY

|x$codeseg|

    EXPORT mul64

; On entry a1 and a2 contain the 32-bit integers to be multiplied (a, b)
; On exit a1 and a2 contain the result (a1 bits 0-31, a2 bits 32-63) 
mul64
    MOV    ip, a1, LSR #16        ; ip = a_hi
    MOV    a4, a2, LSR #16        ; a4 = b_hi
    BIC    a1, a1, ip, LSL #16    ; a1 = a_lo
    BIC    a2, a2, a4, LSL #16    ; a2 = b_lo
    MUL    a3, a1, a2             ; a3 = a_lo * b_lo        (m_lo)
    MUL    a2, ip, a2             ; a2 = a_hi * b_lo        (m_mid1)
    MUL    a1, a4, a1             ; a1 = a_lo * b_hi        (m_mid2)
    MUL    a4, ip, a4             ; a4 = a_hi * b_hi        (m_hi)
    ADDS   ip, a2, a1             ; ip = m_mid1 + m_mid2    (m_mid)
    ADDCS  a4, a4, #&10000        ; a4 = m_hi + carry       (m_hi')
    ADDS   a1, a3, ip, LSL #16    ; a1 = m_lo + (m_mid<<16)
    ADC    a2, a4, ip, LSR #16    ; a2 = m_hi' + (m_mid>>16) + carry
    MOV    pc, lr

    END
