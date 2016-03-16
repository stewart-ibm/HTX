/* IBM_PROLOG_BEGIN_TAG */
/* 
 * Copyright 2003,2016 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 		 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* IBM_PROLOG_END_TAG */

/*
 * cs_term.h:
 *
 *	this file contains the connection station's representation
 *	for the termios structure, ioctl values, input modes,
 *	output modes, control modes and line discipline modes.
 *	it is the responsibility of the host software to translate
 *	host ioctl values to appropriate values for the connection
 *	station.
 */

/*
 * note that the cc_t type is different from the typical
 * host include files.  an unsigned short is used to support
 * the POSIX_VDISABLE functionality.  if one of the cc_t chars
 * is set to POSIX_VDISABLE, then no special handling will be
 * done for this feature.  ie. if c_cc[VERASE] is set to
 * POSIX_VDISABLE, then erase is disabled.  we have expanded
 * c_cc to shorts so that we can used 0xffff so no legal
 * character will match it.
 */
typedef unsigned long	cs_tcflag_t;	/* unsigned integral type */
typedef unsigned short	cs_cc_t;	/* unsigned integral type */

#define CS_NCC			8
#define CS_NFF 			5	/* number of termiox reserved shorts */
#define	CS_NCCS			(CS_NCC + 12)
#define CS_POSIX_VDISABLE	0xffff

/*
 * termios structure.  note that both the output and input
 * baud rates have been removed from c_cflag.  the value of
 * the baud rate is passed in c_obaud or c_ibaud.
 */
struct cs_termios {
	cs_tcflag_t	c_iflag;	/* input modes */
	cs_tcflag_t	c_oflag;	/* output modes */
	cs_tcflag_t	c_cflag;	/* control modes */
	cs_tcflag_t	c_lflag;	/* line discipline modes */
	unsigned long	c_obaud;	/* output baud rate */
	unsigned long	c_ibaud;	/* input baud rate */
	cs_cc_t		c_cc[CS_NCCS];	/* control chars */
};

/*
 * termiox structure.  this structure is primarily used to
 * set/clear the various hardware flow control modes.
 */
struct cs_termiox {
	unsigned short	x_hflag;	/* hardware flow control modes */
	unsigned short	x_cflag;	/* clock modes */
	unsigned short	x_rflag[CS_NFF];/* reserved modes */
	unsigned short	x_sflag;	/* spare local modes */
};

/*
 * control characters.
 */
#define	CS_VINTR	0
#define	CS_VQUIT	1
#define	CS_VERASE	2
#define	CS_VKILL	3
#define	CS_VEOF		4
#define	CS_VEOL		5
#define	CS_VMIN		4
#define	CS_VTIME	5
#define	CS_VEOL2	6
#define	CS_VSWTCH	7
#define CS_VCEOF	8
#define CS_VCEOL	9
#define CS_VSUSP	10
#define CS_VSTART	11
#define CS_VSTOP	12
#define	CS_VDSUSP	13
#define	CS_VREPRINT	14
#define	CS_VDISCARD	15
#define	CS_VWERASE	16
#define	CS_VLNEXT	17
#define CS_VISTART	18
#define CS_VISTOP	19

/*
 * default control chars.
 */
#define	CS_CNUL		0
#define	CS_CDEL		0177
#define	CS_CESC		'\\'
#define	CS_CINTR	0177		/* DEL */
#define	CS_CQUIT	034		/* FS, cntl | */
#define	CS_CERASE	010		/* BS,  cntl H */
#define	CS_CKILL	025		/* NAK, cntl U */
#define CS_CEOT		04
#define CS_CEOL		0
#define CS_CEOL2	0
#define	CS_CEOF		04		/* cntl d */
#define	CS_CSTART	021		/* cntl q */
#define	CS_CSTOP	023		/* cntl s */
#define	CS_CSWTCH	032		/* cntl z */
#define	CS_CNSWTCH	0
#define	CS_CISTART	021		/* cntl q */
#define	CS_CISTOP	023		/* cntl s */

#define	CS_CTRL(c)	(c&037)

#define	CS_CSUSP	CS_CTRL('z')
#define	CS_CDSUSP	CS_CTRL('y')
#define	CS_CRPRNT	CS_CTRL('r')
#define	CS_CFLUSH	CS_CTRL('o')
#define	CS_CWERASE	CS_CTRL('w')
#define	CS_CLNEXT	CS_CTRL('v')

/*
 * input modes.
 */
#define	CS_IGNBRK	0x00000001
#define	CS_BRKINT	0x00000002
#define	CS_IGNPAR	0x00000004
#define	CS_PARMRK	0x00000008
#define	CS_INPCK	0x00000010
#define	CS_ISTRIP	0x00000020
#define	CS_INLCR	0x00000040
#define	CS_IGNCR	0x00000080
#define	CS_ICRNL	0x00000100
#define	CS_IUCLC	0x00000200
#define	CS_IXON		0x00000400
#define	CS_IXANY	0x00000800
#define	CS_IXOFF	0x00001000
#define CS_IMAXBEL	0x00002000

/*
 * output modes.
 */
#define	CS_OPOST	0x00000001
#define	CS_OLCUC	0x00000002
#define	CS_ONLCR	0x00000004
#define	CS_OCRNL	0x00000008
#define	CS_ONOCR	0x00000010
#define	CS_ONLRET	0x00000020
#define	CS_OFILL	0x00000040
#define	CS_OFDEL	0x00000080
#define	CS_NLDLY	0x00000100
#define	CS_NL0		0x00000000
#define	CS_NL1		0x00000100
#define	CS_CRDLY	0x00000600
#define	CS_CR0		0x00000000
#define	CS_CR1		0x00000200
#define	CS_CR2		0x00000400
#define	CS_CR3		0x00000600
#define	CS_TABDLY	0x00001800
#define	CS_TAB0		0x00000000
#define	CS_TAB1		0x00000800
#define	CS_TAB2		0x00001000
#define	CS_TAB3		0x00001800
#define	CS_XTABS	0x00001800
#define	CS_BSDLY	0x00002000
#define	CS_BS0		0x00000000
#define	CS_BS1		0x00002000
#define	CS_VTDLY	0x00004000
#define	CS_VT0		0x00000000
#define	CS_VT1		0x00004000
#define	CS_FFDLY	0x00008000
#define	CS_FF0		0x00000000
#define	CS_FF1		0x00008000

/*
 * control modes.
 */
#define	CS_CSIZE	0x00000030
#define	CS_CS5		0x00000000
#define	CS_CS6		0x00000010
#define	CS_CS7		0x00000020
#define	CS_CS8		0x00000030
#define	CS_CSTOPB	0x00000040
#define	CS_CREAD	0x00000080
#define	CS_PARENB	0x00000100
#define	CS_PARODD	0x00000200
#define	CS_HUPCL	0x00000400
#define	CS_CLOCAL	0x00000800
#define	CS_CTSFLOW	0x00002000
#define	CS_RTSFLOW	0x00004000
#define CS_PAREXT	0x00008000
#define CS_SCORTS	0x00010000
#define CS_CSTOPBEXT	0x00020000

/*
 * line discipline modes.
 */
#define	CS_ISIG		0x00000001
#define	CS_ICANON	0x00000002
#define	CS_XCASE	0x00000004
#define	CS_ECHO		0x00000008
#define	CS_ECHOE	0x00000010
#define	CS_ECHOK	0x00000020
#define	CS_ECHONL	0x00000040
#define	CS_NOFLSH	0x00000080
#define	CS_IEXTEN       0x00000100
#define	CS_TOSTOP       0x00000200
#define	CS_ECHOCTL	0x00000400
#define	CS_ECHOPRT	0x00000800
#define	CS_ECHOKE	0x00001000
#define	CS_FLUSHO	0x00002000
#define	CS_PENDIN	0x00004000

/* 
 * Arguments for TCXONC.
 */
#define CS_TCOOFF	0
#define CS_TCOON	1
#define CS_TCIOFF	2
#define CS_TCION	3

/*
 * posix ioctls.
 */
#define	CS_TIOC		(('C' << 16) | ('S' << 8))
#define	CS_TCSBRK	(CS_TIOC|5)
#define	CS_TCXONC	(CS_TIOC|6)
#define	CS_TCFLSH	(CS_TIOC|7)
#define	CS_TCGETS	(CS_TIOC|13)
#define	CS_TCSETS	(CS_TIOC|14)
#define	CS_TCSETSW	(CS_TIOC|15)
#define	CS_TCSETSF	(CS_TIOC|16)
#define	CS_TIOCMSET	(CS_TIOC|120)
#define	CS_TIOCMBIS	(CS_TIOC|121)
#define	CS_TIOCMBIC	(CS_TIOC|122)
#define	CS_TIOCMGET	(CS_TIOC|123)

/*
 * termiox ioctls.
 */
#if 1
#define	CS_TIOCX	(('C' << 24) | ('S' << 16) | ('X' << 8))
#else
#define	CS_TIOCX	('X' << 8)
#endif
#define	CS_TCGETX	(CS_TIOCX|1)
#define	CS_TCSETX	(CS_TIOCX|2)
#define	CS_TCSETXW	(CS_TIOCX|3)
#define	CS_TCSETXF	(CS_TIOCX|4)
#define	CS_TCBISX	(CS_TIOCX|5)	/* set x_sflag - CI proprietary */
#define	CS_TCBICX	(CS_TIOCX|6)	/* clear x_sflag - CI proprietary */

/*
 * buddy mode controls - use x_sflag of termiox.
 */
#define	CS_BUDDY		0x1	/* turn on buddy mode */
#define	CS_ANALOG_LOOPBACK	0x2	/* turn on a loopback if buddy */
#define	CS_DIGITAL_LOOPBACK	0x4	/* turn on digital loopback if b */

/*
 * miscellaneous setable parameters in x_sflag.
 */
#define	CS_X1			0x8	/* turn on x1 clock (high speed) */
#define	CS_BAUD			0x10	/* BRG in x_rflag[0,1] */
#define	CS_HIBAUD		0x20	/* extended (hi baud rates) */

/*
 * slew rate controls in x_sflag.
 * note this set groups of 4 channels.
 */
#define	CS_SLEW			0x300	/* slew rate mask */
#define	CS_SLEW_SLOW		0x000	/* slowest slew rate */
#define	CS_SLEW_MEDIUM		0x100	/* medium slew rate */
#define	CS_SLEW_FAST		0x200	/* fast slew rate */
#define	CS_SLEW_SUPER		0x300	/* fastest slew rate */

/*
 * hardware flow control modes.
 */
#define	CS_RTSXOFF	0x00000001	/* enable RTS flow control on input */
#define	CS_CTSXON	0x00000002	/* enable CTS flow control on output */
#define	CS_DTRXOFF	0x00000004	/* enable DTR flow control on input */
#define	CS_CDXON	0x00000008	/* enable CD flow control on output */
#define	CS_ISXOFF	0x00000010	/* enable iso. flow control on input */

/*
 * clock modes.
 */
#define	CS_XMTCLK	0x00000007	/* transmit clock source mask */
#define	CS_XCIBRG	0x00000000	/* transmit clock from int. baud rate */
#define	CS_XCTSET	0x00000001	/* transmit clock from DCE xmt source */
#define	CS_XCRSET	0x00000002	/* transmit clock from DCE rcv source */
#define	CS_RCVCLK	0x00000070	/* receive clock mask */
#define	CS_RCIBRG	0x00000000	/* receive clock from int. baud rate */
#define	CS_RCTSET	0x00000010	/* receive clock from DCE xmt source */
#define	CS_RCRSET	0x00000020	/* receive clock from DCE rcv source */
#define	CS_TSETCLK	0x00000700	/* mask for circuit 113 control */
#define	CS_TSETCOFF	0x00000000	/* TSET clock not provided */
#define	CS_TSETCRBRG	0x00000100	/* Output rcv brg on circuit 113 */
#define	CS_TSETCTBRG	0x00000200	/* Output xmt brg on circuit 113 */
#define	CS_TSETCTSET	0x00000300	/* Output xmt timing on circuit 113 */
#define	CS_TSETCRSET	0x00000400	/* Output rcv timing on circuit 113 */
#define	CS_RSETCLK	0x00007000	/* mask for circuit 128 control */
#define	CS_RSETCOFF	0x00000000	/* RSET clock not provided */
#define	CS_RSETCRBRG	0x00001000	/* Output rcv brg on circuit 128 */
#define	CS_RSETCTBRG	0x00002000	/* Output xmt brg on circuit 128 */
#define	CS_RSETCTSET	0x00003000	/* Output xmt timing on circuit 128 */
#define	CS_RSETCRSET	0x00004000	/* Output rcv timing on circuit 128 */

/*
 * modem control flags.
 */
#define	CS_TIOCM_LE	0x00000001	/* line enable */
#define	CS_TIOCM_DTR	0x00000002	/* data terminal ready */
#define	CS_TIOCM_RTS	0x00000004	/* request to send */
#define	CS_TIOCM_ST	0x00000008	/* secondary transmit */
#define	CS_TIOCM_SR	0x00000010	/* secondary receive */
#define	CS_TIOCM_CTS	0x00000020	/* clear to send */
#define	CS_TIOCM_CAR	0x00000040	/* carrier detect */
#define	CS_TIOCM_CD	0x00000040
#define	CS_TIOCM_RNG	0x00000080	/* ring */
#define	CS_TIOCM_RI	0x00000080
#define	CS_TIOCM_DSR	0x00000100	/* data set ready */

/*
 * default flag values.
 */
#define	CS_IFLAG_DFLT	0		/* default input flags */
#define	CS_OFLAG_DFLT	0		/* default output flags */
#define	CS_CFLAG_DFLT	(CS_CS8|CS_CREAD|CS_HUPCL)	/* ctrl flags */
#define	CS_LFLAG_DFLT	0		/* default line discipline flags */
#define	CS_OBAUD_DFLT	9600	/* default output speed */
#define	CS_IBAUD_DFLT	9600	/* default input speed */

/*
 * standard signals.
 */
#define	CS_SIGINT	2		/* interrupt signal */
#define	CS_SIGQUIT	3		/* quit signal */
#define	CS_SIGTSTP	18		/* interactive stop signal */

/*
 * printer control flags.
 */
#define	CS_LIOCM_SEL	0x00000001	/* Printer select (in) */
#define	CS_LIOCM_SELI	0x00000002	/* Printer selected (out) */
#define	CS_LIOCM_BSY	0x00000004	/* Printer busy (in) */
#define	CS_LIOCM_ERR	0x00000008	/* Printer error (in) */
#define	CS_LIOCM_INIT	0x00000010	/* Printer init (out) */
#define	CS_LIOCM_FEED	0x00000020	/* Printer autofeed (out) */
#define	CS_LIOCM_PAPEROUT	0x00000040	/* Printer paper out (in) */

/*
 * Default flag values.
 */
#define CS_IFLAG_DEFAULT	0	/* default input flags */
#define CS_OFLAG_DEFAULT	0	/* default output flags */
#define CS_CFLAG_DEFAULT	(CS_CS8|CS_CREAD|CS_HUPCL)	/* control flags */
#define CS_LFLAG_DEFAULT	0	/* default line discipline flags */
#define CS_OBAUD_DEFAULT	CS_B9600	/* default output speed */
#define CS_IBAUD_DEFAULT	CS_B9600	/* default input speed */

/*
 * Read/Write flags
*/
/* Flag values accessible to open(2) and fcntl(2) */
/*  (The first three can only be set by open) */

#define CS_O_RDONLY        0001
#define CS_O_WRONLY        0002
#define CS_O_RDWR          0003
#define CS_O_ACCMODE       0003    /* Mask for O_RDONLY | O_WRONLY | O_RDWR */

