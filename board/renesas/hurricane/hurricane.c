/*
 * board/renesas/hurricane/hurricane.c
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
#include <netdev.h>
#include <asm/arch/cpu.h>
#include "board.h"

DECLARE_GLOBAL_DATA_PTR;

struct pin_db	pin_guard[] = {
	{ MOD_SEL, 0x01c00000, 0x00000000 },
	{ MOD_SEL2, 0x0c000000, 0x04000000 },
	#if (CONFIG_MMC_ENABLED == 0)
	/* LBSC */
	{ GPSR0, 0x007ffffc, 0x007ffffc },
		/* RD/WR#, EX_CS5-0#, A25-A17, A0, CS1#, CS0#, BS# */
	{ IPSR0, 0x7e7ffff8, 0x00000000 },
	{ IPSR1, 0x0007ffff, 0x00000000 },
	#elif #if (CONFIG_MMC_ENABLED == 1)
	{ GPSR0, 0x007ffffc, 0x007ffffc },
		/* RD/WR#, MMC0_D7-D0, MMC_CLK, MMC_CMD A23-A17, CS1#, CS0# */
	{ IPSR0, 0x7e7ffff8, 0x00120090 },
	{ IPSR1, 0x0007ffff, 0x00011126 },
	#endif
	/* SCIF4 */
	{ GPSR3, 0x0c000000, 0x00000000 },	/* TX4, RX4 */
	{ IPSR1, 0x01e00000, 0x01400000 },
	{ GPSR3, 0x00000000, 0x0c000000 },

	/* SCIF1 */
	{ GPSR4, 0x00010000, 0x00000000 },	/* TX1 */
	{ IPSR1, 0x1e000000, 0x02000000 },
	{ GPSR3, 0x00000000, 0x00010000 },

	{ GPSR4, 0x00020000, 0x00000000 },	/* RX1 */
	{ IPSR2, 0x0000000f, 0x00000001 },
	{ GPSR4, 0x00000000, 0x00020000 },

	/* SCIF CLK */
	{ GPSR4, 0x00000020, 0x00000000 },	/* SCIF_CLK_B */
	{ IPSR6, 0x00038000, 0x00028000 },
	{ GPSR4, 0x00000000, 0x00000020 },
};

struct pin_db	pin_tbl[] = {
};

void pin_init(void)
{
	struct pin_db	*db;

	for (db = pin_guard; db < &pin_guard[sizeof(pin_guard) /
			sizeof(struct pin_db)]; db++) {
		SetGuardREG(db);
	}
	for (db = pin_tbl; db < &pin_tbl[sizeof(pin_tbl) /
			sizeof(struct pin_db)]; db++) {
		SetREG(db);
	}
}

void exbus_init(void)
{
	EXB_W(CS0BSTCTL, 0);		/* none burst */
	EXB_W(CS0BTPH, 0x000000f7);	/* a0h:0, a0w:15, a0b:7 */
	EXB_W(CS0CTRL, 0x00008121);	/* little,128MB,16bit,BROM */
	EXB_W(CS1CTRL, 0x00000020);	/* 16bit, SRAM */
	EXB_W(ECS0CTRL, 0x00000920);	/* 9MB, 16bit, sram */

	/* pulse control */
	/* (write) setup, hold, pulse, (read) setup, hold, pulse */
	#if (CONFIG_MMC_ENABLED == 0)
	EXB_W(ECSWCR0, 0x00280028);	/* 0, 2, 8, 0, 2, 8 */
	#elif #if (CONFIG_MMC_ENABLED == 1)
	EXB_W(CSWCR1, 0x00280028);	/* 0, 2, 8, 0, 2, 8 */
	#endif

	EXB_W(CSPWCR0, 0x00000000);	/* v:0, rv:0, winv:0, */
					/* exwt2:0, wxwt1:0 exwt0:0 */

	#if (CONFIG_MMC_ENABLED == 0)
	EXB_W(EXSPWCR0, 0x00000000);	/* v:0, rv:0, winv:0, */
					/* exwt2:0, wxwt1:0 exwt0:0 */
	#elif #if (CONFIG_MMC_ENABLED == 1)
	EXB_W(CSPWCR1, 0x00000000);	/* v:0, rv:0, winv:0, */
					/* exwt2:0, wxwt1:0 exwt0:0 */
	#endif
}

static void exbus_late_init(void)
{
	EXB_W(CSWCR0, 0x02140326);	/* 2, 1, 4, 3, 2, 6 */
}

static void uart_init(unsigned int scif_base)
{
	writew(CONFIG_SYS_CLK_FREQ / CONFIG_BAUDRATE / 16,
			scif_base + SCIF_DL);
	writew(CKS_EXTERNAL, scif_base + SCIF_CKS);
	wait_usec((1000000 + (CONFIG_BAUDRATE - 1))
			/ CONFIG_BAUDRATE);	/* one bit interval */
}

int board_early_init_f(void)
{
	uart_init(SCIF0_BASE);
	uart_init(SCIF1_BASE);
	uart_init(SCIF2_BASE);
	uart_init(SCIF3_BASE);
	uart_init(SCIF4_BASE);
	uart_init(SCIF5_BASE);

	return 0;
}

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_HURRICANE;
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	icache_enable();
	invalidate_dcache();

	return 0;
}

int board_late_init(void)
{
	/* This function is called after relocating. Now running in SDRAM. */
	exbus_late_init();

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int	rc = 0;

#if defined(CONFIG_SMC911X)
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
	printf("SMSC Base Address - 0x%x ", CONFIG_SMC911X_BASE);
#endif
	return rc;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = CONFIG_SYS_SDRAM_SIZE;
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;
	return 0;
}

int checkboard(void)
{
	printf("BOARD: Hurricane\n");

	return 0;
}

void
wait_usec(int usec)
{
	unsigned long	tick;

	tick = usec * TCLOCK / 4 - 1;
	writeb(readb(TBASE + TSTR0) & ~(1 << 0),
			TBASE + TSTR0);
	writew(0, TBASE + TCR0);
	writel(tick, TBASE + TCOR0);
	writel(tick, TBASE + TCNT0);
	writeb(readb(TBASE + TSTR0) | (1 << 0),
			TBASE + TSTR0);
	while ((readw(TBASE + TCR0) & (1 << 8)) == 0)
		;
	writeb(readb(TBASE + TSTR0) & ~(1 << 0),
			TBASE + TSTR0);
}

void do_memc_init(u32 base)
{
	u32	val;

	/* (1) */
	wait_usec(200);
	/* (2) */
	/* M0BKPRST#(M1BKPRST#)->high */
	/* (3) */
	MEMC_W(base, DBCMD, 0x20000000);		/* opc:RstL, arg:0 */
	/* (4) */
	MEMC_W(base, DBCMD, 0x1000C350);		/* opc:PDEn, arg:100us */
	/* (5) */
	/* M0SDBUP#(M1SDBUP#)->high */

	/* dbsc3 setting-1 */
	/* (6) */
	MEMC_W(base, DBKIND, 0x00000007);		/* ddr3-sdram */
	/* (7) */
	MEMC_W(base, DBCONF0, 0x0F030A02);		/* RowAd:15, Bank:8, CoulumAd:10 */
	MEMC_W(base, DBTR0, 0x00000007);		/* CL:7 */
	MEMC_W(base, DBTR1, 0x00000006);		/* CWL:6 */
	MEMC_W(base, DBTR2, 0x00000000);		/* AL:0 */
	MEMC_W(base, DBTR3, 0x00000007);		/* TRCD:7 */
	MEMC_W(base, DBTR4, 0x00080007);		/* TRPA:8, TRP:7 */
	MEMC_W(base, DBTR5, 0x00000019);		/* TRC:25 */
	MEMC_W(base, DBTR6, 0x00000012);		/* TRAS:18 */
	MEMC_W(base, DBTR7, 0x00000005);		/* TRRD:5 */
	MEMC_W(base, DBTR8, 0x00000019);		/* TFAW:25 */
	MEMC_W(base, DBTR9, 0x00000006);		/* TRDPR:6 */
	MEMC_W(base, DBTR10, 0x00000008);		/* TWR:8 */
	MEMC_W(base, DBTR11, 0x00000007);		/* TRDWR:7 */
	MEMC_W(base, DBTR12, 0x0000000e);		/* TWRRD:14 */
	MEMC_W(base, DBTR13, 0x00000050);		/* TRFC:80 */
	MEMC_W(base, DBTR14, 0x000C0004);		/* DLL:12, TCKEH:4 */
	MEMC_W(base, DBTR15, 0x00000004);	       /* TCKEL:4 */
	MEMC_W(base, DBTR16, 0x10171001);		/* DQL:23 */
	MEMC_W(base, DBTR17, 0x000C000C);		/* TMOD:12, TRDMR:12 */
	MEMC_W(base, DBTR18, 0x00000200);		/* ODT */
	MEMC_W(base, DBTR19, 0x00000040);		/* TZQCS */
	MEMC_W(base, DBRNK0, 0x00000000);		/* initial value */
	/* (8) */
	MEMC_W(base, DBADJ0, 0x00000001);		/* CAMODE:1 */
	/* (9) */
	MEMC_W(base, DBADJ2, 0x00000008);		/* ACAP0:8 */
	/* (10) */
	MEMC_W(base, DBADJ2, 0x00002008);		/* ACAPC0:20 */

	/* phy setting */
	/* (11) */
	val = MEMC_R(base, DBPDCNT3);
	MEMC_W(base, DBPDCNT3, val | 0x00000100);	/* PTRRST:1 */
	/* (12) */
	val = MEMC_R(base, DBPDCNT3);
	MEMC_W(base, DBPDCNT3, val & ~0x00000100);	/* PTRRST:0 */
	/* (13) */
	val = MEMC_R(base, DBPDCNT3);
	MEMC_W(base, DBPDCNT3, val | 0x00000800);	/* CALMODE:1 */
	/* (14) */
	val = MEMC_R(base, DBPDCNT3);
	MEMC_W(base, DBPDCNT3, val | 0x00000C00);	/* CALEN:1 */
	/* (15) */
	val = MEMC_R(base, DBPDCNT3);
	MEMC_W(base, DBPDCNT3, val | 0x01000C00);       /* DLLRESET:1 */
	/* (16) */
	wait_usec(50);
	/* (17) */
	val = MEMC_R(base, DBPDCNT3);
	MEMC_W(base, DBPDCNT3, val | 0x01002C00);	/* IO_ENABLE:1 */
	/* (18) */
	val = MEMC_R(base, DBPDCNT3);
	MEMC_W(base, DBPDCNT3, val | 0x81002C00);	/* PLL2_RESET:1 */
	/* (19) */
	wait_usec(100);
	/* (20) */
	val = MEMC_R(base, DBPDCNT3);
	MEMC_W(base, DBPDCNT3, val | 0xB1002C00);	/* STBY[1]:1,
							STBY[0]:1 */
	/* (21) */
	val = MEMC_R(base, DBPDCNT0);
	MEMC_W(base, DBPDCNT0, val | 0x80000000);	/* BW32:1 */
	/* (22) */
	val = MEMC_R(base, DBPDCNT0);
	MEMC_W(base, DBPDCNT0, val | 0x00110000);	/* ODT:disable */
	MEMC_W(base, DBPDCNT1, 0x00000000);		/* Drive:40ohm */
	MEMC_W(base, DBPDCNT2, 0x00000000);		/* initial value */

#ifdef CONFIG_DDR_DQS_EXTENSION
	/* Extend a period of recieving Data Strobe from DRAM */
	/* (23) */
	MEMC_W(base, DBPDLCK, 0x0000a55a);
	/* (24) */
	MEMC_W(base, DBPDRGA, 0x00000000);
	/* (25) */
	MEMC_W(base, DBPDRGD, 0xa5390000);
	/* (26) */
	MEMC_W(base, DBPDRGA, 0x00000022);
	/* (27) */
	MEMC_W(base, DBPDRGD, 0x00481846);
	/* (28) */
	MEMC_W(base, DBPDRGA, 0x00000023);
	/* (29) */
	MEMC_W(base, DBPDRGD, 0x00481546);
	/* (30) */
	MEMC_W(base, DBPDRGA, 0x00000025);
	/* (31) */
	MEMC_W(base, DBPDRGD, 0x00017200);
	/* (32) */
	MEMC_W(base, DBPDLCK, 0x00000000);
#endif /* CONFIG_DDR_DQS_EXTENSION */

	/* ddr3-sdram setting */
	/* (33) */
	MEMC_W(base, DBCMD, 0x0000c350);		/* Wait, 100us */
	/* (34) */
	MEMC_W(base, DBCMD, 0x2100c350);		/* RstH, 100us */
	/* (35) */
	MEMC_W(base, DBCMD, 0x0000c350);		/* Wait, 100us */
	MEMC_W(base, DBCMD, 0x0000c350);		/* Wait, 100us */
	MEMC_W(base, DBCMD, 0x0000c350);		/* Wait, 100us */
	MEMC_W(base, DBCMD, 0x0000c350);		/* Wait, 100us */
	/* (36) */
	MEMC_W(base, DBCMD, 0x11000055);		/* PDXt, 170ns */
	/* (37) */
	MEMC_W(base, DBCMD, 0x2a000008);		/* MR2, CWL:6 */
	/* (38) */
	MEMC_W(base, DBCMD, 0x2b000000);		/* MR3 */
	/* (39) */
	MEMC_W(base, DBCMD, 0x29000000);		/* MR1, AL:0, DLL:En, ODT=Dis,
							ODS:40ohm */
	/* (40) */
	MEMC_W(base, DBCMD, 0x28000930);		/* MR0, Na, DLL_RES, BL:8,
							BT:Seque, CL:7, WR:8 */
	/* (41) */
	MEMC_W(base, DBCMD, 0x03000200);		/* ZQCL, 512 */

	/* dbsc3 setting-2 */
	/* (42) */
	MEMC_W(base, DBBS0CNT0, 0x00000000);		/* DBBS0CNT0 */
	MEMC_W(base, DBBS0CNT1, 0x00000000);		/* DBBS0CNT1 */
	MEMC_W(base, DBCALCNF, 0x01005398);		/* DBCALCNF */
	MEMC_W(base, DBCALTR, 0x05dc05dc);		/* DBCALTR */
	MEMC_W(base, DBPDNCNF, 0x00000200);		/* DBPDNCNF */
	/* (43) */
	MEMC_W(base, DBRFCNF0, 0x000000c8);		/* 200cycle */
	if (readl(MODEMR) & MD1)
		MEMC_W(base, DBRFCNF1, 0x00000dde);     /* 500MHz */
	else
		MEMC_W(base, DBRFCNF1, 0x00000af0);	/* 375MHz */

	MEMC_W(base, DBRFCNF2, 0x00000000);		/* REFINT:1/1 */
	/* (44) */
	MEMC_W(base, DBRFEN, 0x00000001);		/* DBRFEN. ARFEN */

	/* (45) */
	MEMC_W(base, DBACEN, 0x00000001);		/* DBACEN. ACCEN */
	/* (46) */
	MEMC_R(base, DBWAIT);				/* wait for done */
}

void memc_init(void)
{
	do_memc_init(MEMC_BASE);
	do_memc_init(MEMC2_BASE);
}

void wdt_init(void)
{
	writel(WDTCSR_CODE, WDT_BASE + WDTCSR);
}

void board_reset(void)
{
	writel(WDTST_CODE | 0x1, WDT_BASE + WDTST);
	writel(WDTCSR_WT, WDT_BASE + WDTCSR);
	writel((WDTCSR_TME | WDTCSR_WT), WDT_BASE + WDTCSR);
	while (1)
		;
}
