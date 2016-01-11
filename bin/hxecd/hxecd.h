/*  "@(#)30  1.17.4.2  src/htx/usr/lpp/htx/bin/hxecd/hxecd.h, exer_cd, htxubuntu 1/20/14 05:23:38" */
/******************************************************************************
 *   COMPONENT_NAME: exer_cd
 *
 *   MODULE NAME: hxecd.h
 *
 *   FUNCTIONS: none
 *
 ******************************************************************************/

#include <sys/signal.h>
#include "hxihtx.h"

#define MAX_BLKNO 269250               /* maximum block number                */
#define MIN_BLKNO 050570               /* minimum block number                */
#define BUF_SIZE  (512*1024)           /* read/write buffer size              */
#define HARD 1
#define SOFT 4
#define INFO 7
#define SYSERR 0
#define CD_MSF_MODE  1          /*- define minutes, seconds, frames lba mode -*/
#define DEFAULT_TRK_INDEX 0x01;     /*- define track index for play audio trk */
#define cd_BLOCKSIZE    2048                  /* Disk blocksize               */
#define cd_M2F1_BLKSIZE 2048                  /* The block size for CD-ROM    */
                                              /* XA data mode 2 form 1.       */
#define cd_M2F2_BLKSIZE 2336                  /* The block size for CD-ROM    */
                                              /* XA data mode 2 form 2.       */
#define cd_CDDA_BLKSIZE 2352                  /* The block size for CD-DA     */

#define cd_LASTLBA      269249
#define dvd_LASTLBA     4169951

#define cd_TOC_ENTRY_LBA  264675

#ifndef __HTX_LINUX__
extern char *sys_errlist[]; /* system defined err msgs, indexed by errno      */
#else
extern const char *const sys_errlist[]; /* system defined err msgs,
                                           indexed by errno */
#endif
extern int sys_nerr;        /* max value for errno                            */

struct ruleinfo {
  int   op_in_progress;     /* operation in progress used with SIGTERM shutdwn*/
                            /* 0 = undefined operation, no shutdown needed    */
                            /* 1 = Toshiba vendor unique play audio active    */
                            /* 2 = scsi play play audio active                */
                            /* 3 = device driver CD_PLAY_AUDIO active         */
  int   master_audio_type;  /* 0 = standard play audio scsi commands          */
                            /* 1 = vendor unique audio cmds - TOSHIBA - 3101  */
                            /*     this model does not support read toc x'43' */
                            /* 2 = vndr unique audio cmds - TOSHIBA SLIM-LINE */
  struct cd_pns {
   char  disc_pn[20];       /* part number found fron toc data                */
   char  rule_disc_pn[20];  /* part number specified in rules file            */
  } cds;                    /*  with DISC_PN = xxxxxxxx                       */
  char  rule_id[9];         /* Rule Id                                        */
  char  pattern_id[9];      /* pattern library id                             */
  char  mode[10];           /* cd-rom mode for mode select                    */
  char  retries[5];         /* ON, OFF, BOTH                                  */
  char  addr_type[7];       /* SEQ, RANDOM                                    */
  int   num_oper;           /* number of operations to be performed           */
  char  oper[5];            /* type of operation to be performed              */
  char  starting_block[9];  /* n,mm/ss/bb,TOP,MID,BOT                         */
  int   first_block;        /* starting block converted to numeric            */
  int   msf_mode;           /* true = mm:ss:ff format for lba                 */
                            /* false = use track/block number block length    */
  char  direction[5];       /* UP, DOWN, IN, OUT           * seq oper only    */
  int   increment;          /* number of blocks to skip    *                  */
  char  type_length[7];     /* FIXED, RANDOM                                  */
  int   num_blks;           /* length of data to read in blocks               */
  int   fildes;             /* file descriptor for device to be exercised     */
  unsigned int dlen;        /* length of data to read in bytes                */
  long  tot_blks;           /* Total number of blocks on disc                 */
  long  min_blkno;          /* lowest block to be used                        */
  long  max_blkno;          /* highest block to be used                       */
  int   bytpsec;            /* number of bytes per sector ie. block length    */
  char  cmd_list[250];      /* array to hold commands to be run from shell    */
} ;


/* Header files different for AIX and Linux */

#ifndef __HTX_LINUX__     /* AIX */

#include <sys/cfgodm.h>
#include <sys/devinfo.h>
#include <sys/cdrom.h>
#include <sys/ide.h>
#include <sys/scdisk.h>
#include <sys/scsi.h>
#include <sys/mode.h>

#else                     /* Linux */

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <scsi/sg.h>
#include <linux/cdrom.h>
/* #include <asm/page.h> */

void do_trap_htx64(unsigned long, ...);

/* Macro to page align pointer to next page boundary */

#ifdef __DEFINE_PAGE_MACROS__
#define PAGE_SIZE 4096
#define PAGE_MASK (~(PAGE_SIZE-1))
#endif

#define HTX_PAGE_ALIGN(_X_) ((((unsigned long)_X_)+PAGE_SIZE-1)&PAGE_MASK)

/* MODE DEFINITIONS */

#define CD_MODE1           0x1
#define CD_MODE2_FORM1     0x2
#define CD_MODE2_FORM2     0x3
#define CD_DA              0x4

#endif


/* Linux compatiblity layer for AIX-HTX function calls */
#ifdef __HTX_LINUX__

/* Crash-on-miss should call xmon thru miscex */

#define trap(_P1_, _P2_, _P3_, _P4_, _P5_, _P6_) do_trap_htx64(_P2_, _P3_, _P4_, _P5_, _P6_)

/* Drop extended attributes with openx */
#define openx(_P1_, _P2_, _P3_, _P4_) open(_P1_, _P2_)

/* POSIX Compliant macros for string functions */
#include <string.h>

#define strlen(_X_)\
        ( (size_t) ( (_X_) ? strlen(_X_) : NULL ) )

#define strcpy(_D_,_S_)\
        ( (_D_) ? (_S_) ? strcpy(_D_,_S_) : (_D_) : NULL )

#define strncpy(_D_,_S_,_N_)\
        ( (_D_) ? (_S_) ? strncpy(_D_,_S_, _N_) : (_D_) : NULL )

#define strcat(_D_,_S_)\
        ( (_D_) ? (_S_) ? strcat(_D_,_S_) : (_D_) : NULL )

#define strncat(_D_,_S_,_N_)\
        ( (_D_) ? (_S_) ? strncat(_D_,_S_, _N_) : (_D_) : NULL )

#define strcmp(_S1_,_S2_)\
        ( (int) ( (_S1_) ? (_S2_) ? strcmp(_S1_,_S2_) : 1 : -1 ) )

#define strncmp(_S1_,_S2_,_N_)\
        ( (int) ( (_S1_) ? (_S2_) ? strncmp(_S1_,_S2_,_N_) : 1 : -1  ) )

#endif
