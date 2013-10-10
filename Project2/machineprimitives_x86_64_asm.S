.globl _minithread_switch, _minithread_root, _atomic_test_and_set, _swap, _minithread_trampoline,_interrupt_level

_minithread_switch:
    mov  %rdi,%rcx
    mov  %rsi,%rdx
    mov  %rdx,%rax
    push %r15
    push %r14
    push %r13
    push %r12
    push %r11
    push %r10
    push %r9
    push %r8
    push %rbp
    push %rsi
    push %rdi
    push %rbx
    mov %rsp,(%rcx)
    mov (%rax),%rsp
    movq _interrupt_level@GOTPCREL(%rip),%rbx #Enable interrupts after context switch
    movq $1,(%rbx) #Enable interrupts after context switch
    pop %rbx
    pop %rdi
    pop %rsi
    pop %rbp
    pop %r8
    pop %r9
    pop %r10
    pop %r11
    pop %r12
    pop %r13
    pop %r14
    pop %r15
    ret 

_minithread_root: 
    sub $0x78,%rsp
    pushq %rsi
    callq *%rbx    # call main proc

    popq %rsi      # get clean up location back
    sub $0x8,%rsp
    movq %rbp,%rdi
    callq *%rsi    # call the clean-up
    ret

_atomic_test_and_set:
    mov %rdi,%rdx # Get pointer to l

    # operate on rax here to also zero the upper half
    mov $0x1,%rcx                  # load 1 into the cmpxchg source
    mov $0x0,%rax                  # load 0 into the accumulator

                                # _if l == 0 then
    # we operate on e*x which should be the lower half of r*x
    lock cmpxchg %ecx,(%rdx)    # l = 1 (and eax = 0)
                                # _else
                                # (l = 1 and) eax = 1
    ret

_swap:
    mov %rsi,%rax
    mov %rdi,%rdx

    lock xchg %eax,(%rdx)

    ret

_compare_and_swap:
    # we get x = rdi
    #        oldval = rsi
    #        neval = rcx 

    mov %esi, %eax
    mov %rdi, %rsi
    mov %rdx, %rdi

    lock cmpxchg %edi, (%rsi)

    ret


#define RAX 0
#define RBX 1
#define RCX 2
#define RDX 3
#define RDI 4
#define RSI 5
#define RBP 6
#define RSP 7
#define R8 8
#define R9 9
#define R10 10
#define R11 11
#define R12 12
#define R13 13
#define R14 14
#define R15 15
#define RIP 16

_minithread_trampoline:
    popq %rax #fxrstor address
    cmpq $0,%rax
    je integer_regs #no fp state
    fxrstorq (%rax)
  integer_regs:
    popq %rax
    popq %rbx
    popq %rcx
    popq %rdx
    popq %rdi
    popq %rsi
    popq %rbp
    add  $0x8,%rsp #skip rsp
    popq %r8
    popq %r9
    popq %r10
    popq %r11
    popq %r12
    popq %r13
    popq %r14
    popq %r15
    add $0x8,%rsp #jump over rip
    popfq 
    mov 0x18(%rsp),%rsp #move to end of sigcontext struct
#MUST BE VERY CAREFUL: add $0x70,%rsp changes the carry flag!!!
    pushq %rbx
    movq _interrupt_level@GOTPCREL(%rip),%rbx #Enable interrupts after context switch
    movq $1,(%rbx) #Enable interrupts after context switch
    popq %rbx
    retq  #return address is here, directly below old SP

