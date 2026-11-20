	// Math routines for Calculator app
.syntax unified
.cpu cortex-m33
.thumb
.section .text


// uint32_t Increment(uint32_t num);
.global Increment
.type Increment, %function
Increment:
 add r0, r0, #1
 bx lr


// uint32_t Decrement(uint32_t num);
.global Decrement
.type Decrement, %function
Decrement:
 sub r0, r0, #1
 bx lr



// uint32_t Fibonacci(uint32_t n);
// F(0) = 0, F(1) = 1
// Recursive implementation

.global Fibonacci
.type Fibonacci, %function

Fibonacci:
    cmp r0, #1
    bls fib_return

    push {r4, r5, r6, lr}
    mov r4, r0

    sub r0, r4, #1
    bl Fibonacci
    mov r5, r0

    sub r0, r4, #2
    bl Fibonacci

    add r0, r0, r5
    pop {r4, r5, r6, pc}

fib_return:
    bx lr




 // uint32_t Arith(uint32_t op, uint32_t a, uint32_t b);
 // Four Arithmetic operations

.global Arith
.type Arith, %function
Arith:
	cmp r0, #0
    beq ArithAdd
    cmp r0, #1
    beq ArithSub
    cmp r0, #2
    beq ArithMul
    cmp r0, #3
    beq ArithDiv
    movs r0, #0              //out of range, return 0
    bx lr

ArithAdd:
    add r0, r1, r2          // r0 = a + b
    bx  lr

ArithSub:
    sub r0, r1, r2          // r0 = a - b
    bx  lr

ArithMul:
    mul r0, r1, r2          // r0 = a * b (low 32 bits)
    bx  lr

ArithDiv:
    cbz r2, ArithDivZero    // Guard against divide by zero
    udiv r0, r1, r2          // r0 = a / b (unsigned)
    bx lr
ArithDivZero:
    movs r0, #0
    bx lr




 // uint32_t Factorial(uint32_t n);
.global Factorial
.type Factorial, %function

Factorial:
	movs r1, #1 //base case = 1

FactorialLoop:
	cmp r0, #1 // checks if operand <= 1 (base case)
	bls FactorialDone

	mul r1, r1, r0   // performs factorial math
	subs r0, r0, #1  // decrements operand
	b FactorialLoop  //loop

FactorialDone:
	mov r0, r1   //place result in r0
	bx lr    //return result




// uint32_t GCD(uint32_t a, uint32_t b);
.global GCD
.type GCD, %function

GCD:
	cmp r0, r1
	beq end		// END when a = b
	blo less	// branch to less if a < b
	sub r0, r0, r1   // if a > b, a = a - b
	b GCD

less:
	sub r1, r1, r0   // b = b - a
	b GCD

end:
	bx lr		// return result




// uint32_t Average(uint32_t *arr, uint32_t n);
.global Average

.type Average, %function

Average:

    CMP r1, #0

    BEQ .L_Avg_Zero

    PUSH {r4, r5, lr}

    MOV r2, #0

    MOV r3, #0

.L_Avg_Loop:

    CMP r3, r1

    BGE .L_Avg_Done

    LDR r4, [r0, r3, lsl #2]

    ADD r2, r2, r4

    ADD r3, r3, #1

    B .L_Avg_Loop

.L_Avg_Done:

    MOV r4, #10

    MUL r2, r2, r4

    UDIV r0, r2, r1

    POP {r4, r5, pc}

.L_Avg_Zero:

    MOV r0, #0

    BX LR

//Sort Subroutine from C-Compiler in sort_original.s
.global Sort
.type Sort, %function
.thumb_func
Sort:
    push {r4, lr}
    mov ip, r1 //Array size
    mov lr, #0 //Outer Loop counter
    b .L2

.L3:
    adds r3, r3, #1 //Increase inner loop

.L5:
    add r2, ip, #-1 // for n-1
    cmp r2, r3 //Check inner loop limit
    bls .L8 //End inner loop when done

    ldr r1, [r0, r3, lsl #2] //Load first number
    adds r2, r3, #1 //Next index
    ldr r4, [r0, r2, lsl #2] //Load next number

    cmp r1, r4 //Compare numbers
    bls .L3 //Skip swap if in order
    str r4, [r0, r3, lsl #2] //Store smaller number first
    str r1, [r0, r2, lsl #2] //Store larger number second
    b .L3 //Repear inner loop

.L8:
    add lr, lr, #1 //Increae outer counter

.L2:
    cmp lr, ip //Check outer loop limit
    bcs .L9 //Finish if all passes done
    movs r3, #0 // Reset inner counter
    b .L5 //Start inner loop

.L9:
    pop {r4, pc} //Restore and return

.size Sort, .-Sort
