/* 
 * Copyright (c) 2023 Peter J. Philipp
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
 * this program calculates the L2 pagetable algorithm and points out what it
 * does with address inputs (no arguments) and page inputs (> 1 argument).
 * as discussed on https://marc.info/?l=openbsd-bugs&m=169497376413299&w=2
 */

/*
	The particular ASM code:

     94         lla     s1, pagetable_l2
     95         srli    t4, s9, L2_SHIFT        
     96         li      t2, 512                
     97         add     t3, t4, t2
     98         li      t0, (PTE_KERN | PTE_X)
     99 1:
    100         slli    t2, t4, PTE_PPN1_S    
    101         or      t5, t0, t2
    102         sd      t5, (s1)             
    103         addi    s1, s1, PTE_SIZE
    104
    105         addi    t4, t4, 1
    106         bltu    t4, t3, 1b
    107

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define P_KERN	0x1	/* not real, for demonstration purposes only */
#define P_X	0x2	/* not real, for demonstration purposes only */

char *
binary(ulong t5)
{
	static char ret[1280];
	int i = 0;

	ret[0] = '\0';

	for (i = 53; i >= 0; i--) {
		switch (i) {
		case (53 - 26):
			strlcat(ret,"[32m", sizeof(ret));
			break;
		case (53 - 26 - 9):
			strlcat(ret,"[34m", sizeof(ret));
			break;
		case (53 - 26 - 9 - 9):
			strlcat(ret,"[35m", sizeof(ret));
			break;
		default:
			//strlcat(ret,"[0m", sizeof(ret));
			break;
		}

		if (t5 & (1UL << i)) {
			strlcat(ret, "1", sizeof(ret));
		} else {
			strlcat(ret, "0", sizeof(ret));
		}
	}
		
	return (&ret[0]);
}
	
/* 
 * from /usr/src/sys/arch/riscv64/include/pte.h -
 * PTE magic numbers sed'ed s/PTE_/P_/g 
 */
	
#define	P_SIZE			8		/* 8 bytes PTE size */
#define P_PPN1_S      		19		/* explanation below */

/*
 * P_PPN1_S is a shift of bits from the LSb of the PTE leftwards to the 
 * respective level (given 0, 1, 2, 3), this is becuase the PTE looks like
 * this:
 *
 *			Sv39 Page Table Entry
 * 63                                                              0
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
 * |N| rsvd|         PPN[2]          | PPN[1] |  PPN[0] | ptebits |
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *         |            26           |  9     |   9     |   10
 *         |                         |        |         |
 *         |                         |        |         |
 *         |                         |        |         +--->    P_PPN0_S 10
 *         |                         |        |         
 *         |                         |        +----------->      P_PPN1_S 19
 *         |                         |        
 *         |                         +-------------------->      P_PPN2_S 28
 *         |                         
 *         +---------------------------------------------->      P_PPN3_S 37*
 *								
 *   * In Sv39 this is really 26? which is 26 bits pages, or 67,108,864 pages
 *	or 64 Mega Pages (not to be confused with the Megapage ie, PPN[1]
 *	or 274877906944 bytes (big number)
 *                                   
 */

#define L2_SHIFT		21		/* 2 MiB == 21 bits == 2 ^ 21 */
#define PAGE_SHIFT		12		/* 4096 bytes */


int
main(int argc, char *argv[])
{
	u_long pagetable_l2 = 0x100c000UL;/* VA from readelf -a bsd  |\
							grep pagetable_l2 */

	u_long physmem = 0x40200000UL >> ((argc > 1) ? PAGE_SHIFT : 0); /* PA in s9 */	
	u_long megapages = physmem >> L2_SHIFT;	/* physmem base / 2 MiB */
	u_long slot = 512;			/* slot # */
	u_long slot_limit = megapages + slot;
	u_long pte_bits = (P_KERN | P_X);	/* reg t0, spans 10b from LSb */
	u_long pte;				/* register t5 */

	do {
    		/* 100         slli    t2, t4, PTE_PPN1_S     */

		 slot = megapages << P_PPN1_S;


    		/* 101         or      t5, t0, t2 */

		pte = pte_bits | slot;

		
    		/* 102         sd      t5, (s1)             */


		printf("sd %08lX(%s[0m) to %lX\n", pte, binary(pte), \
			pagetable_l2);


    		/* 103         addi    s1, s1, PTE_SIZE */


		pagetable_l2 += P_SIZE;


    		/* 105         addi    t4, t4, 1 */

		megapages += 1;


    		/* 106         bltu    t4, t3, 1b */

	} while (megapages < slot_limit);
	
	
	return 0;
}
