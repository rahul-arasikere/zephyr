/**
 * Copyrights (c) 2024 Rahul Arasikere <arasikere.rahul@gmail.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TI_HERCULES_RM57LX_SOC_H_
#define TI_HERCULES_RM57LX_SOC_H_

#include <zephyr/devicetree.h>
#include <zephyr/types.h>

struct hercules_syscon_1_regs {
	/* 0xFFFFFF00U */
	uint32_t SYSPC1;      /* 0x0000 */
	uint32_t SYSPC2;      /* 0x0004 */
	uint32_t SYSPC3;      /* 0x0008 */
	uint32_t SYSPC4;      /* 0x000C */
	uint32_t SYSPC5;      /* 0x0010 */
	uint32_t SYSPC6;      /* 0x0014 */
	uint32_t SYSPC7;      /* 0x0018 */
	uint32_t SYSPC8;      /* 0x001C */
	uint32_t SYSPC9;      /* 0x0020 */
	uint32_t rsvd1;       /* 0x0024 */
	uint32_t rsvd2;       /* 0x0028 */
	uint32_t rsvd3;       /* 0x002C */
	uint32_t CSDIS;       /* 0x0030 */
	uint32_t CSDISSET;    /* 0x0034 */
	uint32_t CSDISCLR;    /* 0x0038 */
	uint32_t CDDIS;       /* 0x003C */
	uint32_t CDDISSET;    /* 0x0040 */
	uint32_t CDDISCLR;    /* 0x0044 */
	uint32_t GHVSRC;      /* 0x0048 */
	uint32_t VCLKASRC;    /* 0x004C */
	uint32_t RCLKSRC;     /* 0x0050 */
	uint32_t CSVSTAT;     /* 0x0054 */
	uint32_t MSTGCR;      /* 0x0058 */
	uint32_t MINITGCR;    /* 0x005C */
	uint32_t MSINENA;     /* 0x0060 */
	uint32_t MSTFAIL;     /* 0x0064 */
	uint32_t MSTCGSTAT;   /* 0x0068 */
	uint32_t MINISTAT;    /* 0x006C */
	uint32_t PLLCTL1;     /* 0x0070 */
	uint32_t PLLCTL2;     /* 0x0074 */
	uint32_t SYSPC10;     /* 0x0078 */
	uint32_t DIEIDL;      /* 0x007C */
	uint32_t DIEIDH;      /* 0x0080 */
	uint32_t rsvd4;       /* 0x0084 */
	uint32_t LPOMONCTL;   /* 0x0088 */
	uint32_t CLKTEST;     /* 0x008C */
	uint32_t DFTCTRLREG1; /* 0x0090 */
	uint32_t DFTCTRLREG2; /* 0x0094 */
	uint32_t rsvd5;       /* 0x0098 */
	uint32_t rsvd6;       /* 0x009C */
	uint32_t GPREG1;      /* 0x00A0 */
	uint32_t rsvd7;       /* 0x00A4 */
	uint32_t rsvd8;       /* 0x00A8 */
	uint32_t rsvd9;       /* 0x00AC */
	uint32_t SSIR1;       /* 0x00B0 */
	uint32_t SSIR2;       /* 0x00B4 */
	uint32_t SSIR3;       /* 0x00B8 */
	uint32_t SSIR4;       /* 0x00BC */
	uint32_t RAMGCR;      /* 0x00C0 */
	uint32_t BMMCR1;      /* 0x00C4 */
	uint32_t rsvd10;      /* 0x00C8 */
	uint32_t CPURSTCR;    /* 0x00CC */
	uint32_t CLKCNTL;     /* 0x00D0 */
	uint32_t ECPCNTL;     /* 0x00D4 */
	uint32_t rsvd11;      /* 0x00D8 */
	uint32_t DEVCR1;      /* 0x00DC */
	uint32_t SYSECR;      /* 0x00E0 */
	uint32_t SYSESR;      /* 0x00E4 */
	uint32_t SYSTASR;     /* 0x00E8 */
	uint32_t GBLSTAT;     /* 0x00EC */
	uint32_t DEVID;       /* 0x00F0 */
	uint32_t SSIVEC;      /* 0x00F4 */
	uint32_t SSIF;        /* 0x00F8 */
};

struct hercules_syscon_2_regs {
	/* 0xFFFFE100U */
	uint32_t PLLCTL3;     /* 0x0000 */
	uint32_t rsvd1;       /* 0x0004 */
	uint32_t STCCLKDIV;   /* 0x0008 */
	uint32_t rsvd2[6];    /* 0x000C */
	uint32_t ECPCNTL;     /* 0x0024 */
	uint32_t ECPCNTL1;    /* 0x0028 */
	uint32_t rsvd3[4];    /* 0x002C */
	uint32_t CLK2CNTRL;   /* 0x003C */
	uint32_t VCLKACON1;   /* 0x0040 */
	uint32_t rsvd4[4];    /* 0x0044 */
	uint32_t HCLKCNTL;    /* 0x0054 */
	uint32_t rsvd5[6];    /* 0x0058 */
	uint32_t CLKSLIP;     /* 0x0070 */
	uint32_t rsvd6;       /* 0x0074 */
	uint32_t IP1ECCERREN; /* 0x0078 */
	uint32_t rsvd7[28];   /* 0x007C */
	uint32_t EFC_CTLEN;   /* 0x00EC */
	uint32_t DIEIDL_REG0; /* 0x00F0 */
	uint32_t DIEIDH_REG1; /* 0x00F4 */
	uint32_t DIEIDL_REG2; /* 0x00F8 */
	uint32_t DIEIDH_REG3; /* 0x00FC */
};

struct hercules_pcr_regs {
	uint32_t PMPROTSET0;    /* 0x0000 */
	uint32_t PMPROTSET1;    /* 0x0004 */
	uint32_t rsvd1[2];      /* 0x0008 */
	uint32_t PMPROTCLR0;    /* 0x0010 */
	uint32_t PMPROTCLR1;    /* 0x0014 */
	uint32_t rsvd2[2];      /* 0x0018 */
	uint32_t PPROTSET0;     /* 0x0020 */
	uint32_t PPROTSET1;     /* 0x0024 */
	uint32_t PPROTSET2;     /* 0x0028 */
	uint32_t PPROTSET3;     /* 0x002C */
	uint32_t rsvd3[4];      /* 0x0030 */
	uint32_t PPROTCLR0;     /* 0x0040 */
	uint32_t PPROTCLR1;     /* 0x0044 */
	uint32_t PPROTCLR2;     /* 0x0048 */
	uint32_t PPROTCLR3;     /* 0x004C */
	uint32_t rsvd4[4];      /* 0x0050 */
	uint32_t PCSPWRDWNSET0; /* 0x0060 */
	uint32_t PCSPWRDWNSET1; /* 0x0064 */
	uint32_t rsvd5[2];      /* 0x0068 */
	uint32_t PCSPWRDWNCLR0; /* 0x0070 */
	uint32_t PCSPWRDWNCLR1; /* 0x0074 */
	uint32_t rsvd6[2];      /* 0x0078 */
	uint32_t PSPWRDWNSET0;  /* 0x0080 */
	uint32_t PSPWRDWNSET1;  /* 0x0084 */
	uint32_t PSPWRDWNSET2;  /* 0x0088 */
	uint32_t PSPWRDWNSET3;  /* 0x008C */
	uint32_t rsvd7[4];      /* 0x0090 */
	uint32_t PSPWRDWNCLR0;  /* 0x00A0 */
	uint32_t PSPWRDWNCLR1;  /* 0x00A4 */
	uint32_t PSPWRDWNCLR2;  /* 0x00A8 */
	uint32_t PSPWRDWNCLR3;  /* 0x00AC */
	uint32_t rsvd8[4];      /* 0x00B0 */
	uint32_t PDPWRDWNSET;   /* 0x00C0 */
	uint32_t PDPWRDWNCLR;   /* 0x00C4 */
	uint32_t rsvd9[78];     /* 0x00C8 */
	uint32_t MSTIDWRENA;    /* 0x0200 */
	uint32_t MSTIDENA;      /* 0x0204 */
	uint32_t MSTIDDIAGCTRL; /* 0x0208 */
	uint32_t rsvd10[61];    /* 0x020C */
	struct {
		uint32_t PSxMSTID_L;
		uint32_t PSxMSTID_H;
	} PSxMSTID[32]; /* 0x0300 */
	struct {
		uint32_t PPSxMSTID_L;
		uint32_t PPSxMSTID_H;
	} PPSxMSTID[8]; /* 0x0400 */
	struct {
		uint32_t PPSExMSTID_L;
		uint32_t PPSExMSTID_H;
	} PPSExMSTID[32];       /* 0x0440 */
	uint32_t PCSxMSTID[32]; /* 0x0540 */
	uint32_t PPCSxMSTID[8]; /* 0x05C0 */
};

struct hercules_esm_regs {
	uint32_t EEPAPR1; /* 0x0000                 */
	uint32_t DEPAPR1; /* 0x0004                 */
	uint32_t IESR1;   /* 0x0008                 */
	uint32_t IECR1;   /* 0x000C                 */
	uint32_t ILSR1;   /* 0x0010                 */
	uint32_t ILCR1;   /* 0x0014                 */
	uint32_t SR1[3U]; /* 0x0018, 0x001C, 0x0020 */
	uint32_t EPSR;    /* 0x0024                 */
	uint32_t IOFFHR;  /* 0x0028                 */
	uint32_t IOFFLR;  /* 0x002C                 */
	uint32_t LTCR;    /* 0x0030                 */
	uint32_t LTCPR;   /* 0x0034                 */
	uint32_t EKR;     /* 0x0038                 */
	uint32_t SSR2;    /* 0x003C                 */
	uint32_t IEPSR4;  /* 0x0040                 */
	uint32_t IEPCR4;  /* 0x0044                 */
	uint32_t IESR4;   /* 0x0048                 */
	uint32_t IECR4;   /* 0x004C                 */
	uint32_t ILSR4;   /* 0x0050                 */
	uint32_t ILCR4;   /* 0x0054                 */
	uint32_t SR4[3U]; /* 0x0058, 0x005C, 0x0060 */
};

#define SYS1_NODE DT_NODELABEL(sys1)
#define SYS2_NODE DT_NODELABEL(sys2)
#define PCR1_NODE DT_NODELABEL(pcr1)
#define PCR2_NODE DT_NODELABEL(pcr2)
#define PCR3_NODE DT_NODELABEL(pcr3)
#define ESM_NODE  DT_NODELABEL(esm)

#endif /* TI_HERCULES_RM57LX_SOC_H_ */
