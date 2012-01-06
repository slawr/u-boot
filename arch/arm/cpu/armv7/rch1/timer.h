/*
 * arch/arm/cpu/armv7/rch1/timer.h
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

#define	TIMER_LOAD_VAL	0xffffffff
#define	READ_TIMER	(readl(TBASE + TCNT0))

#define timestamp (gd->tbl)
#define lastdec (gd->lastinc)
