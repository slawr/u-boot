/*
 * board/renesas/marzen/lowlevel_init.S
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

#include <config.h>
#include <version.h>

	.globl lowlevel_init
lowlevel_init:
	mrc	p15, 0, r0, c0, c0, 5	@ Check of CPU ID #
	ands	r0, r0, #0xf
	beq	do_lowlevel_init	@ CPU ID #0
	b	do_cpu_waiting		@ CPU ID #1-#3

	.pool

/*
 * CPU ID #1-#3 come here
 */
	.align  4
do_cpu_waiting:
	wfi

	b	do_cpu_waiting

/*
 * Only CPU ID #0 comes here
 */
	.align  4
do_lowlevel_init:

	ldr	r3, =0xfe798000
	sub	sp, r3, #4
	str	ip, [sp]
	mov	ip, lr

	/* initialize wdt */
	bl	wdt_init

	/* select pin */
	bl	pin_init

	/* initialize exbus */
	bl	exbus_init

	/* initialize ddr */
	bl	memc_init

	ldr	ip, [sp]
	mov	lr, ip
	mov	pc, lr
	nop
