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

#include "ilgen/MethodBuilder.hpp"

#include "InterpreterTypes.h"
#include "InterpreterTypeDictionary.hpp"

#include "PushConstantBuilder.hpp"
#include "DupBuilder.hpp"
#include "MathBuilder.hpp"
#include "RetBuilder.hpp"
#include "ExitBuilder.hpp"
#include "CallBuilder.hpp"
#include "JumpBuilder.hpp"
#include "PopLocalBuilder.hpp"
#include "PushLocalBuilder.hpp"

class JitMethod : public TR::MethodBuilder
   {
   public:
   JitMethod(InterpreterTypeDictionary *d, Method *method);
   virtual bool buildIL();

   protected:

   private:
   Method *_method;
   InterpreterTypeDictionary *_interpTypes;
   TR::BytecodeBuilder **_builders;

   char* getBytecodeName(interpreter_opcodes opcode)
      {
      switch(opcode)
         {
         case PUSH_CONSTANT:
            return "PUSH_CONSTANT";
         case DUP:
            return "DUP";
         case ADD:
            return "ADD";
         case SUB:
            return "SUB";
         case MUL:
            return "MUL";
         case DIV:
            return "DIV";
         case RET:
            return "RET";
         case CALL:
            return "CALL";
         case EXIT:
            return "EXIT";
         case JMPL:
            return "JMPL";
         case PUSH_LOCAL:
            return "PUSH_LOCAL";
         case POP_LOCAL:
            return "POP_LOCAL";
         default:
            {
            int *x = 0;
            fprintf(stderr, "unknown bytecode\n");
            *x = 0;
            return NULL;
            }
         }
      }

   int32_t getBytecodeLength(interpreter_opcodes opcode)
      {
      switch(opcode)
         {
         case PUSH_CONSTANT:
            return 2;
         case DUP:
            return 1;
         case ADD:
            return 1;
         case SUB:
            return 1;
         case MUL:
            return 1;
         case DIV:
            return 1;
         case RET:
            return 2;
         case CALL:
            return 3;
         case EXIT:
            return 2;
         case JMPL:
            return 2;
         case PUSH_LOCAL:
            return 2;
         case POP_LOCAL:
            return 2;
         default:
            {
            int *x = 0;
            fprintf(stderr, "unknown bytecode\n");
            *x = 0;
            return 0;
            }
         }
      }

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
            return JumpBuilder::OrphanBytecodeBuilder(this, bcIndex);
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
