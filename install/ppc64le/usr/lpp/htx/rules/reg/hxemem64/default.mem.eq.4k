* CREATE 256 MB SHM using 4K pages: Mode normal
rule_id = EQ_TST
pattern_id = HEXFF(8) HEXZEROS(8) 0xAAAA5555AAAA5555 0x5555AAAA5555AAAA 0xCCCCCCCCCCCCCCCC 0x3333333333333333 0x0F0F0F0F0F0F0F0F 0x3C3C3C3C3C3C3C3C 0x5A5A5A5A5A5A5A5A
max_mem = NO
startup_delay = 0
compare = yes
num_oper = 256
num_writes = 1
num_read_only = 1
num_read_comp = 1
switch_pat_per_seg = all
mode = normal
bind_proc = yes
oper = mem
width = 8
num_seg_4k = 1
seg_size_4k = 268435456
num_seg_64k = 0
num_seg_16g = 0
num_seg_16m = 0

