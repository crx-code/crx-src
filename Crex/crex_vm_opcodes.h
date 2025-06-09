/*
   +----------------------------------------------------------------------+
   | Crex Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) Crex Technologies Ltd. (http://www.crex.com)           |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Crex license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.crex.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Crex license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@crex.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@crx.net>                                 |
   |          Zeev Suraski <zeev@crx.net>                                 |
   |          Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_VM_OPCODES_H
#define CREX_VM_OPCODES_H

#define CREX_VM_SPEC		1
#define CREX_VM_LINES		0
#define CREX_VM_KIND_CALL	1
#define CREX_VM_KIND_SWITCH	2
#define CREX_VM_KIND_GOTO	3
#define CREX_VM_KIND_HYBRID	4
/* HYBRID requires support for computed GOTO and global register variables*/
#if (defined(__GNUC__) && defined(HAVE_GCC_GLOBAL_REGS))
# define CREX_VM_KIND		CREX_VM_KIND_HYBRID
#else
# define CREX_VM_KIND		CREX_VM_KIND_CALL
#endif

#if (CREX_VM_KIND == CREX_VM_KIND_HYBRID) && !defined(__SANITIZE_ADDRESS__)
# if ((defined(i386) && !defined(__PIC__)) || defined(__x86_64__) || defined(_M_X64))
#  define CREX_VM_HYBRID_JIT_RED_ZONE_SIZE 16
# endif
#endif

#define CREX_VM_OP_SPEC          0x00000001
#define CREX_VM_OP_CONST         0x00000002
#define CREX_VM_OP_TMPVAR        0x00000004
#define CREX_VM_OP_TMPVARCV      0x00000008
#define CREX_VM_OP_MASK          0x000000f0
#define CREX_VM_OP_NUM           0x00000010
#define CREX_VM_OP_JMP_ADDR      0x00000020
#define CREX_VM_OP_TRY_CATCH     0x00000030
#define CREX_VM_OP_THIS          0x00000050
#define CREX_VM_OP_NEXT          0x00000060
#define CREX_VM_OP_CLASS_FETCH   0x00000070
#define CREX_VM_OP_CONSTRUCTOR   0x00000080
#define CREX_VM_OP_CONST_FETCH   0x00000090
#define CREX_VM_OP_CACHE_SLOT    0x000000a0
#define CREX_VM_EXT_VAR_FETCH    0x00010000
#define CREX_VM_EXT_ISSET        0x00020000
#define CREX_VM_EXT_CACHE_SLOT   0x00040000
#define CREX_VM_EXT_ARRAY_INIT   0x00080000
#define CREX_VM_EXT_REF          0x00100000
#define CREX_VM_EXT_FETCH_REF    0x00200000
#define CREX_VM_EXT_DIM_WRITE    0x00400000
#define CREX_VM_EXT_MASK         0x0f000000
#define CREX_VM_EXT_NUM          0x01000000
#define CREX_VM_EXT_LAST_CATCH   0x02000000
#define CREX_VM_EXT_JMP_ADDR     0x03000000
#define CREX_VM_EXT_OP           0x04000000
#define CREX_VM_EXT_TYPE         0x07000000
#define CREX_VM_EXT_EVAL         0x08000000
#define CREX_VM_EXT_TYPE_MASK    0x09000000
#define CREX_VM_EXT_SRC          0x0b000000
#define CREX_VM_NO_CONST_CONST   0x40000000
#define CREX_VM_COMMUTATIVE      0x80000000
#define CREX_VM_OP1_FLAGS(flags) (flags & 0xff)
#define CREX_VM_OP2_FLAGS(flags) ((flags >> 8) & 0xff)

BEGIN_EXTERN_C()

CREX_API const char* CREX_FASTCALL crex_get_opcode_name(uint8_t opcode);
CREX_API uint32_t CREX_FASTCALL crex_get_opcode_flags(uint8_t opcode);
CREX_API uint8_t crex_get_opcode_id(const char *name, size_t length);

END_EXTERN_C()

#define CREX_NOP                          0
#define CREX_ADD                          1
#define CREX_SUB                          2
#define CREX_MUL                          3
#define CREX_DIV                          4
#define CREX_MOD                          5
#define CREX_SL                           6
#define CREX_SR                           7
#define CREX_CONCAT                       8
#define CREX_BW_OR                        9
#define CREX_BW_AND                      10
#define CREX_BW_XOR                      11
#define CREX_POW                         12
#define CREX_BW_NOT                      13
#define CREX_BOOL_NOT                    14
#define CREX_BOOL_XOR                    15
#define CREX_IS_IDENTICAL                16
#define CREX_IS_NOT_IDENTICAL            17
#define CREX_IS_EQUAL                    18
#define CREX_IS_NOT_EQUAL                19
#define CREX_IS_SMALLER                  20
#define CREX_IS_SMALLER_OR_EQUAL         21
#define CREX_ASSIGN                      22
#define CREX_ASSIGN_DIM                  23
#define CREX_ASSIGN_OBJ                  24
#define CREX_ASSIGN_STATIC_PROP          25
#define CREX_ASSIGN_OP                   26
#define CREX_ASSIGN_DIM_OP               27
#define CREX_ASSIGN_OBJ_OP               28
#define CREX_ASSIGN_STATIC_PROP_OP       29
#define CREX_ASSIGN_REF                  30
#define CREX_QM_ASSIGN                   31
#define CREX_ASSIGN_OBJ_REF              32
#define CREX_ASSIGN_STATIC_PROP_REF      33
#define CREX_PRE_INC                     34
#define CREX_PRE_DEC                     35
#define CREX_POST_INC                    36
#define CREX_POST_DEC                    37
#define CREX_PRE_INC_STATIC_PROP         38
#define CREX_PRE_DEC_STATIC_PROP         39
#define CREX_POST_INC_STATIC_PROP        40
#define CREX_POST_DEC_STATIC_PROP        41
#define CREX_JMP                         42
#define CREX_JMPZ                        43
#define CREX_JMPNZ                       44
#define CREX_JMPC_EX                     46
#define CREX_JMPNC_EX                    47
#define CREX_CASE                        48
#define CREX_CHECK_VAR                   49
#define CREX_SEND_VAR_NO_REF_EX          50
#define CREX_CAST                        51
#define CREX_BOOL                        52
#define CREX_FAST_CONCAT                 53
#define CREX_ROPE_INIT                   54
#define CREX_ROPE_ADD                    55
#define CREX_ROPE_END                    56
#define CREX_BEGIN_SILENCE               57
#define CREX_END_SILENCE                 58
#define CREX_INIT_FCALL_BY_NAME          59
#define CREX_DO_FCALL                    60
#define CREX_INIT_FCALL                  61
#define CREX_RETURN                      62
#define CREX_RECV                        63
#define CREX_RECV_INIT                   64
#define CREX_SEND_VAL                    65
#define CREX_SEND_VAR_EX                 66
#define CREX_SEND_REF                    67
#define CREX_NEW                         68
#define CREX_INIT_NS_FCALL_BY_NAME       69
#define CREX_FREE                        70
#define CREX_INIT_ARRAY                  71
#define CREX_ADD_ARRAY_ELEMENT           72
#define CREX_INCLUDE_OR_EVAL             73
#define CREX_UNSET_VAR                   74
#define CREX_UNSET_DIM                   75
#define CREX_UNSET_OBJ                   76
#define CREX_FE_RESET_R                  77
#define CREX_FE_FETCH_R                  78
#define CREX_EXIT                        79
#define CREX_FETCH_R                     80
#define CREX_FETCH_DIM_R                 81
#define CREX_FETCH_OBJ_R                 82
#define CREX_FETCH_W                     83
#define CREX_FETCH_DIM_W                 84
#define CREX_FETCH_OBJ_W                 85
#define CREX_FETCH_RW                    86
#define CREX_FETCH_DIM_RW                87
#define CREX_FETCH_OBJ_RW                88
#define CREX_FETCH_IS                    89
#define CREX_FETCH_DIM_IS                90
#define CREX_FETCH_OBJ_IS                91
#define CREX_FETCH_FUNC_ARG              92
#define CREX_FETCH_DIM_FUNC_ARG          93
#define CREX_FETCH_OBJ_FUNC_ARG          94
#define CREX_FETCH_UNSET                 95
#define CREX_FETCH_DIM_UNSET             96
#define CREX_FETCH_OBJ_UNSET             97
#define CREX_FETCH_LIST_R                98
#define CREX_FETCH_CONSTANT              99
#define CREX_CHECK_FUNC_ARG             100
#define CREX_EXT_STMT                   101
#define CREX_EXT_FCALL_BEGIN            102
#define CREX_EXT_FCALL_END              103
#define CREX_EXT_NOP                    104
#define CREX_TICKS                      105
#define CREX_SEND_VAR_NO_REF            106
#define CREX_CATCH                      107
#define CREX_THROW                      108
#define CREX_FETCH_CLASS                109
#define CREX_CLONE                      110
#define CREX_RETURN_BY_REF              111
#define CREX_INIT_METHOD_CALL           112
#define CREX_INIT_STATIC_METHOD_CALL    113
#define CREX_ISSET_ISEMPTY_VAR          114
#define CREX_ISSET_ISEMPTY_DIM_OBJ      115
#define CREX_SEND_VAL_EX                116
#define CREX_SEND_VAR                   117
#define CREX_INIT_USER_CALL             118
#define CREX_SEND_ARRAY                 119
#define CREX_SEND_USER                  120
#define CREX_STRLEN                     121
#define CREX_DEFINED                    122
#define CREX_TYPE_CHECK                 123
#define CREX_VERIFY_RETURN_TYPE         124
#define CREX_FE_RESET_RW                125
#define CREX_FE_FETCH_RW                126
#define CREX_FE_FREE                    127
#define CREX_INIT_DYNAMIC_CALL          128
#define CREX_DO_ICALL                   129
#define CREX_DO_UCALL                   130
#define CREX_DO_FCALL_BY_NAME           131
#define CREX_PRE_INC_OBJ                132
#define CREX_PRE_DEC_OBJ                133
#define CREX_POST_INC_OBJ               134
#define CREX_POST_DEC_OBJ               135
#define CREX_ECHO                       136
#define CREX_OP_DATA                    137
#define CREX_INSTANCEOF                 138
#define CREX_GENERATOR_CREATE           139
#define CREX_MAKE_REF                   140
#define CREX_DECLARE_FUNCTION           141
#define CREX_DECLARE_LAMBDA_FUNCTION    142
#define CREX_DECLARE_CONST              143
#define CREX_DECLARE_CLASS              144
#define CREX_DECLARE_CLASS_DELAYED      145
#define CREX_DECLARE_ANON_CLASS         146
#define CREX_ADD_ARRAY_UNPACK           147
#define CREX_ISSET_ISEMPTY_PROP_OBJ     148
#define CREX_HANDLE_EXCEPTION           149
#define CREX_USER_OPCODE                150
#define CREX_ASSERT_CHECK               151
#define CREX_JMP_SET                    152
#define CREX_UNSET_CV                   153
#define CREX_ISSET_ISEMPTY_CV           154
#define CREX_FETCH_LIST_W               155
#define CREX_SEPARATE                   156
#define CREX_FETCH_CLASS_NAME           157
#define CREX_CALL_TRAMPOLINE            158
#define CREX_DISCARD_EXCEPTION          159
#define CREX_YIELD                      160
#define CREX_GENERATOR_RETURN           161
#define CREX_FAST_CALL                  162
#define CREX_FAST_RET                   163
#define CREX_RECV_VARIADIC              164
#define CREX_SEND_UNPACK                165
#define CREX_YIELD_FROM                 166
#define CREX_COPY_TMP                   167
#define CREX_BIND_GLOBAL                168
#define CREX_COALESCE                   169
#define CREX_SPACESHIP                  170
#define CREX_FUNC_NUM_ARGS              171
#define CREX_FUNC_GET_ARGS              172
#define CREX_FETCH_STATIC_PROP_R        173
#define CREX_FETCH_STATIC_PROP_W        174
#define CREX_FETCH_STATIC_PROP_RW       175
#define CREX_FETCH_STATIC_PROP_IS       176
#define CREX_FETCH_STATIC_PROP_FUNC_ARG 177
#define CREX_FETCH_STATIC_PROP_UNSET    178
#define CREX_UNSET_STATIC_PROP          179
#define CREX_ISSET_ISEMPTY_STATIC_PROP  180
#define CREX_FETCH_CLASS_CONSTANT       181
#define CREX_BIND_LEXICAL               182
#define CREX_BIND_STATIC                183
#define CREX_FETCH_THIS                 184
#define CREX_SEND_FUNC_ARG              185
#define CREX_ISSET_ISEMPTY_THIS         186
#define CREX_SWITCH_LONG                187
#define CREX_SWITCH_STRING              188
#define CREX_IN_ARRAY                   189
#define CREX_COUNT                      190
#define CREX_GET_CLASS                  191
#define CREX_GET_CALLED_CLASS           192
#define CREX_GET_TYPE                   193
#define CREX_ARRAY_KEY_EXISTS           194
#define CREX_MATCH                      195
#define CREX_CASE_STRICT                196
#define CREX_MATCH_ERROR                197
#define CREX_JMP_NULL                   198
#define CREX_CHECK_UNDEF_ARGS           199
#define CREX_FETCH_GLOBALS              200
#define CREX_VERIFY_NEVER_TYPE          201
#define CREX_CALLABLE_CONVERT           202
#define CREX_BIND_INIT_STATIC_OR_JMP    203

#define CREX_VM_LAST_OPCODE             203

#endif
