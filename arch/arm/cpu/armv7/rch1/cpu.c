/*
 * arch/arm/cpu/armv7/rch1/cpu.c
 *
 * Copyright (C) 2011-2012 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>

void reset_cpu(ulong addr)
{
	board_reset();
}

int print_cpuinfo(void)
{
	unsigned int	md = readl(MODEMR);

	printf("CPU  : R-CarH1 (md:0x%x)\n", md);
	printf("       [CPU:%sMHz,SHwy:%sMHz,DDR:%sMHz,EXCLK:%sMHz]\n",
			md & MD1 ? "1000" : "750",
			md & MD1 ? "250" : "187.5",
			md & MD1 ? "500" : "375",
			md & MD2 ? md & MD1 ? "41.6" : "46.9" : "62.5");
	timer_init();
	return 0;
}
