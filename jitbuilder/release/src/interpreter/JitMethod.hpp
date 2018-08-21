/*******************************************************************************
 * Copyright (c) 2016, 2016 IBM Corp. and others
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


#ifndef JITMETHOD_INCL
#define JITMETHOD_INCL

#include "ilgen/RuntimeBuilder.hpp"

#include "InterpreterTypes.h"
#include "InterpreterTypeDictionary.hpp"

#include "PushConstantBuilder.hpp"
#include "DupBuilder.hpp"
#include "MathBuilder.hpp"
#include "RetBuilder.hpp"
#include "ExitBuilder.hpp"
#include "CallBuilder.hpp"
#include "JumpIfBuilder.hpp"
#include "PopLocalBuilder.hpp"
#include "PushLocalBuilder.hpp"

namespace TR { class BytecodeBuilder;}

class JitMethod : public TR::RuntimeBuilder
   {
   public:
   JitMethod(InterpreterTypeDictionary *d, Method *method);
   virtual TR::IlValue *GetImmediate(TR::BytecodeBuilder *builder, int32_t pcOffset);
   virtual void DefaultFallthroughTarget(TR::BytecodeBuilder *builder);
   virtual void SetJumpIfTarget(TR::BytecodeBuilder *builder, TR::IlValue *condition, TR::IlValue *jumpTarget);
   virtual void ReturnTarget(TR::BytecodeBuilder *builder);
   virtual bool buildIL();

   protected:

   private:
   Method *_method;
   InterpreterTypeDictionary *_interpTypes;
   TR::BytecodeBuilder **_builders;

   TR::BytecodeBuilder* createBuilder(interpreter_opcodes opcode, int32_t bcIndex)
      {
      switch(opcode)
         {
         case PUSH_CONSTANT:
            return PushConstantBuilder::OrphanBytecodeBuilder(this, bcIndex);
         case DUP:
            return DupBuilder::OrphanBytecodeBuilder(this, bcIndex);
         case ADD:
            return MathBuilder::OrphanBytecodeBuilder(this, bcIndex, &MathBuilder::add);
         case SUB:
            return MathBuilder::OrphanBytecodeBuilder(this, bcIndex, &MathBuilder::sub);
         case MUL:
            return MathBuilder::OrphanBytecodeBuilder(this, bcIndex, &MathBuilder::mul);
         case DIV:
            return MathBuilder::OrphanBytecodeBuilder(this, bcIndex, &MathBuilder::div);
         case RET:
            return RetBuilder::OrphanBytecodeBuilder(this, bcIndex, _interpTypes->getTypes().pFrame);
         case JMPL:
            return JumpIfBuilder::OrphanBytecodeBuilder(this, bcIndex, &JumpIfBuilder::lessThan);
         case JMPG:
            return JumpIfBuilder::OrphanBytecodeBuilder(this, bcIndex, &JumpIfBuilder::greaterThan);
         case PUSH_LOCAL:
            return PushLocalBuilder::OrphanBytecodeBuilder(this, bcIndex);
         case POP_LOCAL:
            return PopLocalBuilder::OrphanBytecodeBuilder(this, bcIndex);
         case CALL:
            return CallBuilder::OrphanBytecodeBuilder(this, bcIndex, _interpTypes->getTypes().pInterpreter, _interpTypes->getTypes().pFrame);
         case EXIT:
            return ExitBuilder::OrphanBytecodeBuilder(this, bcIndex);
         default:
            {
            int *x = 0;
            fprintf(stderr, "unknown bytecode\n");
            *x = 0;
            return NULL;
            }
         }
      }
   };

#endif // !defined(JITMETHOD_INCL)
