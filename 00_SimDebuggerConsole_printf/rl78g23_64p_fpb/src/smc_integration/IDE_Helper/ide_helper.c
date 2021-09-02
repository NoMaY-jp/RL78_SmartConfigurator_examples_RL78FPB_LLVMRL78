#include "ide_helper.h"

#if defined(__CCRL__)

typedef struct
{
    unsigned char no0:1;
    unsigned char no1:1;
    unsigned char no2:1;
    unsigned char no3:1;
    unsigned char no4:1;
    unsigned char no5:1;
    unsigned char no6:1;
    unsigned char no7:1;
} __bitf_T;
#define PP   __near
#define CP   __far
#define BIT  __bitf_T
#define BYTE unsigned char
#define BASE 0x0FF00

#elif defined(__ICCRL78__)

typedef struct
{
    unsigned char no0:1;
    unsigned char no1:1;
    unsigned char no2:1;
    unsigned char no3:1;
    unsigned char no4:1;
    unsigned char no5:1;
    unsigned char no6:1;
    unsigned char no7:1;
} __BITS8;
#define PP   __far
#define CP   __far
#define BIT  __BITS8
#define BYTE unsigned char
#define BASE 0xFFF00

#elif defined(__GNUC__) /* And also in case of LLVM */

typedef struct
{
    unsigned char no0:1;
    unsigned char no1:1;
    unsigned char no2:1;
    unsigned char no3:1;
    unsigned char no4:1;
    unsigned char no5:1;
    unsigned char no6:1;
    unsigned char no7:1;
} __BITS8;
#define PP
#define CP   __far
#define BIT  __BITS8
#define BYTE unsigned char
#define BASE 0xFFF00

#endif

volatile BYTE PP * const CP P0  = (BYTE PP *)(BASE + 0);
volatile BYTE PP * const CP P1  = (BYTE PP *)(BASE + 1);
volatile BYTE PP * const CP P2  = (BYTE PP *)(BASE + 2);
volatile BYTE PP * const CP P3  = (BYTE PP *)(BASE + 3);
volatile BYTE PP * const CP P4  = (BYTE PP *)(BASE + 4);
volatile BYTE PP * const CP P5  = (BYTE PP *)(BASE + 5);
volatile BYTE PP * const CP P6  = (BYTE PP *)(BASE + 6);
volatile BYTE PP * const CP P7  = (BYTE PP *)(BASE + 7);
volatile BYTE PP * const CP P8  = (BYTE PP *)(BASE + 8);
volatile BYTE PP * const CP P9  = (BYTE PP *)(BASE + 9);
volatile BYTE PP * const CP P10 = (BYTE PP *)(BASE + 10);
volatile BYTE PP * const CP P11 = (BYTE PP *)(BASE + 11);
volatile BYTE PP * const CP P12 = (BYTE PP *)(BASE + 12);
volatile BYTE PP * const CP P13 = (BYTE PP *)(BASE + 13);
volatile BYTE PP * const CP P14 = (BYTE PP *)(BASE + 14);
volatile BYTE PP * const CP P15 = (BYTE PP *)(BASE + 15);

volatile BIT PP * const CP P0_bit  = (BIT PP *)(BASE + 0);
volatile BIT PP * const CP P1_bit  = (BIT PP *)(BASE + 1);
volatile BIT PP * const CP P2_bit  = (BIT PP *)(BASE + 2);
volatile BIT PP * const CP P3_bit  = (BIT PP *)(BASE + 3);
volatile BIT PP * const CP P4_bit  = (BIT PP *)(BASE + 4);
volatile BIT PP * const CP P5_bit  = (BIT PP *)(BASE + 5);
volatile BIT PP * const CP P6_bit  = (BIT PP *)(BASE + 6);
volatile BIT PP * const CP P7_bit  = (BIT PP *)(BASE + 7);
volatile BIT PP * const CP P8_bit  = (BIT PP *)(BASE + 8);
volatile BIT PP * const CP P9_bit  = (BIT PP *)(BASE + 9);
volatile BIT PP * const CP P10_bit = (BIT PP *)(BASE + 10);
volatile BIT PP * const CP P11_bit = (BIT PP *)(BASE + 11);
volatile BIT PP * const CP P12_bit = (BIT PP *)(BASE + 12);
volatile BIT PP * const CP P13_bit = (BIT PP *)(BASE + 13);
volatile BIT PP * const CP P14_bit = (BIT PP *)(BASE + 14);
volatile BIT PP * const CP P15_bit = (BIT PP *)(BASE + 15);

static const void CP * const CP e2_studio_visual_expression_view_helper_v[] = 
{
	(const void CP *) &P0,
	(const void CP *) &P1,
	(const void CP *) &P2,
	(const void CP *) &P3,
	(const void CP *) &P4,
	(const void CP *) &P5,
	(const void CP *) &P6,
	(const void CP *) &P7,
	(const void CP *) &P8,
	(const void CP *) &P9,
	(const void CP *) &P10,
	(const void CP *) &P11,
	(const void CP *) &P12,
	(const void CP *) &P13,
	(const void CP *) &P14,
	(const void CP *) &P15,
	(const void CP *) &P0_bit,
	(const void CP *) &P1_bit,
	(const void CP *) &P2_bit,
	(const void CP *) &P3_bit,
	(const void CP *) &P4_bit,
	(const void CP *) &P5_bit,
	(const void CP *) &P6_bit,
	(const void CP *) &P7_bit,
	(const void CP *) &P8_bit,
	(const void CP *) &P9_bit,
	(const void CP *) &P10_bit,
	(const void CP *) &P11_bit,
	(const void CP *) &P12_bit,
	(const void CP *) &P13_bit,
	(const void CP *) &P14_bit,
	(const void CP *) &P15_bit
};

/* The following function is just to prevent the symbol from getting optimized away
 * for e2 studio's Visual Expression View. */
void e2_studio_visual_expression_view_helper(void)
{
    void CP * volatile e;

    e = (void CP *)e2_studio_visual_expression_view_helper_v;

    (void) e;
}
