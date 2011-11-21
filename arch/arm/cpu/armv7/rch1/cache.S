/*
 * arch/arm/cpu/armv7/rch1/cache.S
 *
 * Copyright(c) 2011 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

	.align	5
	.global	invalidate_dcache
/* invalidate dcache */
invalidate_dcache:
	stmfd   r13!, {r0 - r5, r7, r9 - r12, r14}
	dmb
	mrc	p15, 1, r0, c0, c0, 1	/* read clidr */
	ands	r3, r0, #0x7000000	/* extract loc */
	mov	r3, r3, lsr #23		/* cache level value */
	beq	invd_done		/* if loc is 0, then no need */
	mov	r10, #0			/* start at cache level 0 */
invd_loop_level:
	and	r2, r10, r10, lsr #1	/* work out 3x cache level */
	mov	r1, r0, lsr r2		/* cache type for this level */
	and	r1, r1, #7		/* get those 3 bits alone */
	cmp	r1, #2
	blt	invd_skip		/* no cache or just icache */
	mcr	p15, 2, r10, c0, c0, 0	/* write csselr */
	isb
	mrc	p15, 1, r1, c0, c0, 0	/* read ccsidr */
	and	r2, r1, #7		/* line length field */
	add	r2, r2, #4		/* SetSFT */
	ldr	r4, =0x3ff
	ands	r4, r4, r1, lsr #3	/* maximum number of way size */
	clz	r5, r4			/* WaySFT */
	ldr	r7, =0x7fff		/* maximum number of index size */
	ands	r7, r7, r1, lsr #13	/* set = max_set; */
invd_loop_set:				/* do { */
	mov	r9, r4			/*   way = max_way; */
invd_loop_way:				/*   do { */
	orr	r11, r10, r9, lsl r5	/*     val = (way << WaySFT) */
	orr	r11, r11, r7, lsl r2	/*         | (set << SetSFT); */
	mcr	p15, 0, r11, c7, c6, 2	/*     invalidate(val); */
	subs	r9, r9, #1		/*     way--; */
	bge	invd_loop_way		/*   while (way >= 0); */
	subs	r7, r7, #1		/*   set--; */
	bge	invd_loop_set		/* while (set >= 0); */
invd_skip:
	add	r10, r10, #2		/* increment cache number */
	cmp	r3, r10
	bgt	invd_loop_level
invd_done:
	mov	r10, #0			/* cache level 0 */
	mcr	p15, 2, r10, c0, c0, 0	/* write csselr */
	dsb
	isb
	ldmfd	r13!, {r0 - r5, r7, r9 - r12, pc}
