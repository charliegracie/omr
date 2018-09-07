/*******************************************************************************
 * Copyright (c) 2016, 2018 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution and
 * is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following
 * Secondary Licenses when the conditions for such availability set
 * forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
 * General Public License, version 2 with the GNU Classpath
 * Exception [1] and GNU General Public License, version 2 with the
 * OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

#include "Jit.hpp"
#include "ilgen/InterpreterBuilder.hpp"
#include "ilgen/JitMethodBuilder.hpp"
#include "ilgen/VirtualMachineOperandArray.hpp"
#include "ilgen/VirtualMachineOperandStack.hpp"
#include "ilgen/VirtualMachineState.hpp"
#include "ilgen/VirtualMachineRegister.hpp"
#include "ilgen/VirtualMachineRegisterInStruct.hpp"

#include "InterpreterTypes.h"

#include "InterpreterTypeDictionary.hpp"
#include "JitMethod.hpp"
#include "BytecodeHelpers.hpp"
#include "PushConstantBuilder.hpp"
#include "DupBuilder.hpp"
#include "MathBuilder.hpp"
#include "RetBuilder.hpp"
#include "ExitBuilder.hpp"
#include "CallBuilder.hpp"
#include "JumpIfBuilder.hpp"
#include "PopLocalBuilder.hpp"
#include "PushLocalBuilder.hpp"
#include "PushArgBuilder.hpp"

static void debug(Interpreter *interp, Frame *frame)
   {
#define DEBUG_LINE LINETOSTR(__LINE__)
   fprintf(stderr, "debug interp %p frame %p frame->stack[0] %lld \n", interp, frame, frame->stack[0]);
   }

JitMethod::JitMethod(InterpreterTypeDictionary *d, Method *method)
   : JitMethodBuilder(d),
   _interpTypes(d),
   _method(method)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   DefineName(method->name);

   TR::IlType *pInt8 = d->PointerTo(Int8);

   DefineParameter("interp", _interpTypes->getTypes().pInterpreter);
   DefineParameter("frame", _interpTypes->getTypes().pFrame);

   TR::IlType *voidType = d->toIlType<void>();
   DefineFunction((char *)"debug2", (char *)__FILE__, (char *)DEBUG_LINE, (void *)&debug, voidType, 2, _interpTypes->getTypes().pInterpreter, _interpTypes->getTypes().pFrame);

   DefineReturnType(Int64);
   }

void
JitMethod::DefineFunctions(TR::MethodBuilder *mb)
   {
   BytecodeHelpers::DefineFunctions(mb);
   }

TR::VirtualMachineState *
JitMethod::createVMState()
   {
   TR::VirtualMachineRegisterInStruct *stackRegister = new TR::VirtualMachineRegisterInStruct(this, "Frame", "frame", "sp", "SP");
   TR::VirtualMachineOperandStack *stack = new TR::VirtualMachineOperandStack(this, 10, STACKVALUEILTYPE, stackRegister, true, 0);

   TR::VirtualMachineRegisterInStruct *localsRegister = new TR::VirtualMachineRegisterInStruct(this, "Frame", "frame", "locals", "LOCALS");
   TR::VirtualMachineOperandArray *localsArray = new TR::VirtualMachineOperandArray(this, 10, STACKVALUEILTYPE, localsRegister);

   TR::VirtualMachineRegisterInStruct *argsRegister = new TR::VirtualMachineRegisterInStruct(this, "Frame", "frame", "args", "ARGS");
   TR::VirtualMachineOperandArray *argsArray = new TR::VirtualMachineOperandArray(this, _method->argCount, STACKVALUEILTYPE, argsRegister);

   argsArray->Reload(this);

   InterpreterVMState *vmState = new InterpreterVMState(stack, stackRegister, localsArray, localsRegister, argsArray, argsRegister);

   return vmState;
   }

bool
JitMethod::hasMoreBytecodes(int32_t currentBytecodeIndex)
   {
   return (currentBytecodeIndex < _method->bytecodeLength);
   }

TR::IlValue *
JitMethod::getBytecodeAtIndex(TR::IlBuilder *builder, int32_t index)
   {
   return builder->ConstInt8(_method->bytecodes[index]);
   }

TR::BytecodeBuilder*
JitMethod::createBuilder(int32_t bcIndex)
   {
   interpreter_opcodes opcode = (interpreter_opcodes)_method->bytecodes[bcIndex];
   switch(opcode)
      {
      case PUSH_CONSTANT:
         return OrphanBytecodeBuilder<PushConstantBuilder>(bcIndex, "PUSH_CONSTNT");
      case PUSH_ARG:
         return OrphanBytecodeBuilder<PushArgBuilder>(bcIndex, "PUSH_ARG");
      case PUSH_LOCAL:
         return OrphanBytecodeBuilder<PushLocalBuilder>(bcIndex, "PUSH_LOCAL");
      case POP_LOCAL:
         return OrphanBytecodeBuilder<PopLocalBuilder>(bcIndex, "POP_LOCAL");
      case DUP:
         return OrphanBytecodeBuilder<DupBuilder>(bcIndex, "DUP");
      case ADD:
         {
         MathBuilder *math = OrphanBytecodeBuilder<MathBuilder>(bcIndex, "ADD");
         math->setFunction(&MathBuilder::add);
         return math;
         }
      case SUB:
         {
         MathBuilder *math = OrphanBytecodeBuilder<MathBuilder>(bcIndex, "SUB");
         math->setFunction(&MathBuilder::sub);
         return math;
         }
      case MUL:
         {
         MathBuilder *math = OrphanBytecodeBuilder<MathBuilder>(bcIndex, "MUL");
         math->setFunction(&MathBuilder::mul);
         return math;
         }
      case DIV:
         {
         MathBuilder *math = OrphanBytecodeBuilder<MathBuilder>(bcIndex, "DIV");
         math->setFunction(&MathBuilder::div);
         return math;
         }
      case JMPL:
         {
         JumpIfBuilder *jump = OrphanBytecodeBuilder<JumpIfBuilder>(bcIndex, "JMPL");
         jump->setFunction(&JumpIfBuilder::lessThan);
         return jump;
         }
      case JMPG:
         {
         JumpIfBuilder *jump = OrphanBytecodeBuilder<JumpIfBuilder>(bcIndex, "JMPL");
         jump->setFunction(&JumpIfBuilder::lessThan);
         return jump;
         }
      case CALL:
         return OrphanBytecodeBuilder<CallBuilder>(bcIndex, "CALL");
      case RET:
         return OrphanBytecodeBuilder<RetBuilder>(bcIndex, "RET");
      case EXIT:
      default:
         {
         return NULL;
         }
      }
   }

