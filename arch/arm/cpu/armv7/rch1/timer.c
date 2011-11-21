/*
 * arch/arm/cpu/armv7/rch1/timer.c
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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include "timer.h"

DECLARE_GLOBAL_DATA_PTR;

void reset_timer_masked(void)
{
	lastdec = READ_TIMER;
	timestamp = 0;
}

ulong get_timer_timestamp(void)
{
	ulong	now = READ_TIMER;

	if (lastdec > now)
		timestamp += (lastdec - now);
	else
		timestamp += (TIMER_LOAD_VAL - now) + lastdec;
	lastdec = now;
	return timestamp;
}

ulong get_timer_masked(void)
{
	return get_timer_timestamp() / (gd->timer_rate_hz / CONFIG_SYS_HZ);
}

int timer_init(void)
{
	if (readl(MODEMR) & MD1)
		gd->timer_rate_hz = 62500000 / 4;
	else
		gd->timer_rate_hz = 46875000 / 4;
	writeb(readb(TBASE + TSTR0) & ~STR0,
			TBASE + TSTR0);
	writew(0x0, TBASE + TCR0);
	writel(TIMER_LOAD_VAL, TBASE + TCOR0);
	writel(TIMER_LOAD_VAL, TBASE + TCNT0);
	writeb(readb(TBASE + TSTR0) | STR0,
			TBASE + TSTR0);
	reset_timer_masked();
	return 0;
}

void reset_timer(void)
{
	reset_timer_masked();
	return;
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void __udelay(unsigned long usec)
{
	ulong	startcount, delaycount;
	ulong	usecwait;

	while (usec) {
		startcount = get_timer_timestamp();
		usecwait = (usec >= 100) ? 100 : usec;
		delaycount = usecwait * gd->timer_rate_hz / 1000000;
		while ((get_timer_timestamp() - startcount) < delaycount)
			;
		usec -= usecwait;
	}
}
