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
 * This program is a calculator for trying to make human sense of the 
 * locore.S program in OpenBSD/riscv64 it is dated sept. 7 2023
 *
 */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/tree.h>

#include <machine/pmap.h>
#include <uvm/uvm_extern.h>
#include <uvm/uvm_page.h>

#include <stdio.h>
#include <stdlib.h>


#define PTE_D           (1 << 7) /* Dirty */
#define PTE_A           (1 << 6) /* Accessed */
#define PTE_G		(1 << 5) /* Global */
#define PTE_X		(1 << 3) /* execute */
#define PTE_W           (1 << 2) /* Write */
#define PTE_R           (1 << 1) /* Read */
#define PTE_V           (1 << 0) /* Valid */

#define OLD_PTE_KERN        (PTE_R | PTE_W | PTE_A | PTE_D | PTE_V)
#define PTE_KERN        (PTE_A | PTE_D | PTE_W)
#define KERNBASE	0xffffffc000000000UL
#define PTE_PPN0_S	10
#define PTE_PPN1_S      19
#define PTE_PPN2_S      28
#define PTE_PPN3_S      37

#define PTE_SIZE        8



int
main(void)
{
	uint64_t i = 0x40000000;
	uint64_t ib, ib0, ib1, p1, p2, pd;

	i = i / 4096;

#if 0
	printf("sizeof struct vm_page %lu (%lx)\n", sizeof(struct vm_page), sizeof(struct vm_page));
	printf("working PTE_KERN\n");
	printf("PTE_KERN = %d (%x) %s\n", PTE_KERN, PTE_KERN, (PTE_KERN % 8) ? "n" :  "y");
	printf("%lld (%llx) %s\n", i, i, (i % 8) ? "n" : "y");

	printf("%lld (%llx) %s\n", PTE_KERN | (i << 10), PTE_KERN | (i << 10),
		(PTE_KERN | (i << 10)) % 8 ? "n" : "y");
	printf("old PTE_KERN\n");
	printf("OLD_PTE_KERN = %d (%x) %s\n", OLD_PTE_KERN, OLD_PTE_KERN, (OLD_PTE_KERN % 8) ? "n" :  "y");
	printf("%lld (%llx) %s\n", i, i, (i % 8) ? "n" : "y");

	printf("%lld (%llx) %s\n", OLD_PTE_KERN | (i << 10), OLD_PTE_KERN | (i << 10),
		(OLD_PTE_KERN | (i << 10)) % 8 ? "n" : "y");
	printf("%lld (%llx)\n", (1ULL << 9), (1ULL << 9));
#endif


/*
   745: ffffffc00100b000     0 NOTYPE  LOCAL  DEFAULT    4 pagetable_l1
   746: ffffffc00100c000     0 NOTYPE  LOCAL  DEFAULT    4 pagetable_l2
   747: ffffffc00100d000     0 NOTYPE  LOCAL  DEFAULT    4 pagetable_l2_devmap
*/
	
	ib = 0xffffffc001101f20;
	printf("_end is at %llu (%llx)\n", ib, ib);


	ib = 0xffffffc00100b000;
	p1 = ib;
	printf("pagetable_l1 (this is s1) = %llu (%llx)\n", ib, ib);

	ib += 4096;
	p2 = ib;
	printf("pagetable_l2 = (this is s2) =  = %llu (%llx)\n", ib, ib);

	printf("add L1 entry =========================\n");
	ib >>= 12;
	p2 = ib;
	printf("pagetable_l2 >> PAGE_SHIFT(12) = %llu (%llx)\n", ib, ib);
	ib = KERNBASE;
	printf("KERNBASE = %lu (%lx)\n", KERNBASE, KERNBASE);
	ib >>= 30;
	printf("KERNBASE >> 30 = %llu (%llx)\n", (ib), (ib));
	ib &= 0x1ff;
	printf("(KERNBASE >> 30) & 0x1ff %llu (%llx)\n", (ib), (ib));
	ib = p2;
	ib <<= PTE_PPN0_S;
	printf("pagetable_l2 << PTE_PPN0_S: %llu (%llx)\n", (ib), (ib));
	ib |= PTE_V;
	ib0 = ib;
	printf("above (this is t6)| PTE_V: %llu (%llx)\n", (ib), (ib));
	ib = ((KERNBASE >> 30) & 0x1ff) * 8;
	printf("((KERNBASE >> 30) & 0x1ff) * 8(PTE_SIZE): %llu (%llx)\n", (ib), (ib));
	printf("((KERNBASE >> 30) & 0x1ff) * 8(PTE_SIZE): %llu (%llx)\\\n"
		"(2048) + s1(becomes t0) == %llu (%llx)\n", (ib), (ib),
		(ib + p1), (ib + p1));
	ib += p1;
	printf("store t6(%llu [%llx] into t0(%llu [%llx])\n", ib0, ib0,
		ib, ib);

	printf("============================\nL2 megapages (511 * 2 MB)\n");
	printf("   ^^^^^^^^^^^^^^^\n");

	printf("the comments and code are wrong, there is on 511 * 2 MB megapages (9 bits)\n");
	printf("$ dc\n");
	printf("2o\n");
	printf("512 p\n");
	printf("1000000000\n");
	printf("511 p\n");
	printf("111111111\n\n");

	pd = 0xffffffc00100d000;
	printf("let pd = pagetable_l2_devmap == %llu (%llx)\n", pd, pd);
	pd >>= 12;
	printf("right shift this pd by 12 (PAGE_SHIFT) %llu (%llx)\n", pd, pd);

	ib = 0x4000000;
	printf("let physmem BASE(0x40000000) >> 21 (divide 2MB, L2_SHIFT) == %llu (%llx)\n",
		(ib >> 21), (ib >> 21));

	ib >>= 21;
	ib0 = ib;
	ib += 512;

	printf("add (512) == %llu (%llx)\n", (ib), (ib));
		
	ib0 <<= PTE_PPN1_S;
	printf("base physmem pagecount (%llu [%llx]) << PTE_PPN1_S\n",
		ib0, ib0);
	
	ib0 |= (PTE_KERN | PTE_X);

	printf("OR with (PTE_KERN | PTE_X) %llu (%llx) and write to pagetable2"
		"at %llu (%llx)\n", ib0, ib0, p2, p2);
	
	ib1 = p2;
	ib1 += 8;
	printf("add PAGE_SIZE(8) to pagetable2 == %llu (%llx)\n", ib1, ib1);

	ib1 += 1;
	printf("add 1 it is now %llu (%llx)\n", ib1, ib1);	

	printf("li      a5, (VM_MAX_KERNEL_ADDRESS - L2_SIZE)\n");
#define       PJP_VM_MAX_KERNEL_ADDRESS   (0xffffffc800000000UL)
	printf(" VM_MAX_KERNEL_ADDRESS - 2MB == %llu (%llx)\n",
		(PJP_VM_MAX_KERNEL_ADDRESS - (1ULL << 21)),
		(PJP_VM_MAX_KERNEL_ADDRESS - (1ULL << 21)));

	ib = (PJP_VM_MAX_KERNEL_ADDRESS - (1ULL << 21));
	ib >>= 30;

	printf("right shift this by 30 (L1_SHIFT) %llu (%llx)\n",
		ib, ib);

	ib &= 0x1ff;
	printf("and it by 0x1ff == %llu (%llx)\n",
		ib, ib);

	ib |= 1;

	printf ("and  OR it by 1 (PTE_V) == %llu (%llx)\n", ib, ib);
	
	pd <<= 10;
	printf("leftshift pagetable_devmap (pd) by PTE_PPN0_S == %llu (%llx)\n",
		pd, pd);
	
	
	
	printf("more to come.. \n");
	
	
	
}
