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
   | Authors: Dmitry Stogov <dmitry@crx.net>                              |
   +----------------------------------------------------------------------+
*/

#ifndef CREX_VM_H
#define CREX_VM_H

BEGIN_EXTERN_C()

CREX_API void CREX_FASTCALL crex_vm_set_opcode_handler(crex_op* opcode);
CREX_API void CREX_FASTCALL crex_vm_set_opcode_handler_ex(crex_op* opcode, uint32_t op1_info, uint32_t op2_info, uint32_t res_info);
CREX_API void CREX_FASTCALL crex_serialize_opcode_handler(crex_op *op);
CREX_API void CREX_FASTCALL crex_deserialize_opcode_handler(crex_op *op);
CREX_API const void* CREX_FASTCALL crex_get_opcode_handler_func(const crex_op *op);
CREX_API const crex_op *crex_get_halt_op(void);
CREX_API int CREX_FASTCALL crex_vm_call_opcode_handler(crex_execute_data *ex);
CREX_API int crex_vm_kind(void);
CREX_API bool crex_gcc_global_regs(void);

void crex_vm_init(void);
void crex_vm_dtor(void);

END_EXTERN_C()

#define CREX_VM_SET_OPCODE_HANDLER(opline) crex_vm_set_opcode_handler(opline)

#endif
