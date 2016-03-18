/* @(#)95   1.1  src/htx/usr/lpp/htx/bin/hxefpu64/miscomp_debug.h, exer_fpu, htxubuntu 3/4/16 00:59:55 */

#ifndef _MISCMP_DEBUG_H
#define _MISCMP_DEBUG_H

#include "framework.h"

#define MISCMP_TYPE_VSR     (1)
#define MISCMP_TYPE_GPR 	(2)
#define MISCMP_TYPE_FPSCR   (3)
#define MISCMP_TYPE_VSCR    (4)
#define MISCMP_TYPE_CR      (5)
#define MISCMP_TYPE_LS      (6)


#define INSTR_TYPE_LOAD 	(1)
#define INSTR_TYPE_STORE 	(2)
#define INSTR_TYPE_OTHER	(0)

#define VSR_DTYPE			(1)
#define GPR_DTYPE			(2)
#define	OTHER_DTYPE			(4)

void decode_tc_instructions(int cno);

struct reguse *prepare_ls_call_list(FILE *dump, int cno, int cmp_offset);

struct reguse *create_instr_tree(FILE *dump, int cno, int reg_num, int tgt_dtype, int start_index);

void delete_reg_use_list(struct reguse *reg_node);

#endif /* _MISCMP_DEBUG_H */
