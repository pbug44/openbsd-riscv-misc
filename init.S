/* 
 * Copyright (c) 2020 Peter J. Philipp
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*
 * to build
 * as -o init.o init.S ; ld -static -o init init.o
 *
 */


/* from Brian Bamsch */
.section ".note.openbsd.ident", "a"
    .p2align 2
    .long    8
    .long    4
    .long    1
    .ascii "OpenBSD\0"
    .long    0
    .previous

.text
.align 2
.globl main

main:
	addi 	sp,sp,-16
	sw 	ra,12(sp)

	/* open /dev/console */

	lui	a0, %hi(devconsole)		/* path hi */
	addi	a0,a0,%lo(devconsole)		/* path lo */
	
	li	a1, 2		/* 2 = O_RDWR */
	li	a2, 0		/* mode */

	/* syscall 5 - open */
	li	t0, 5		/* set syscall number */

	ecall 			/* do system call */

	li 	t0, -1		
	beq	a0, t0, 2f	/* if return code == -1 jump to 2f */

	mv	t0, a0		/* move returncode (fd) to t0)
	mv	t1, a0		/* file-descriptor of /dev/console */

	/* syscall 90 - dup2 */

	li	t3, 2		/* set STDERR_FILENO */
1:
	
	mv	a0, t1		/* arg1 - fd of /dev/console */
	mv 	a1, t3		/* arg2 - STDERR to STDIN_FILENO */

	li	t0, 90		/* dup2 */
	ecall
	
	addi	t3, t3, -1	/* subtract 1 */
	bgez	t3, 1b		/* branch if greater than zero */

	/* write to /dev/console */
	li	a0, 1		/* set to STDOUT_FILENO */

	lui	a1, %hi(string1)	/* helloworld hi */
	addi	a1,a1,%lo(string1)	/* helloworld lo */
	li	a2, 13			/* strlen helloworld */

	/* syscall 4 - write */
	li	t0, 4			/* syscall 4 */
	ecall

	mv 	t0, a0

2:
	/* enter endless loop */
	nop
	j 2b

.section .rodata
.balign 4

string1:
	.string "hello world!\n"

devconsole:
	.string "/dev/console"

