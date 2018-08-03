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


#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include "Jit.hpp"
#include "ilgen/InterpreterBuilder.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/VirtualMachineInterpreterStack.hpp"
#include "ilgen/VirtualMachineState.hpp"
#include "ilgen/VirtualMachineRegister.hpp"

#include "Interpreter.hpp"
#include "PushBuilder.hpp"
#include "DupBuilder.hpp"
#include "MathBuilder.hpp"
#include "RetBuilder.hpp"

enum interpreter_opcodes
   {
   PUSH_CONSTANT = OMR::InterpreterBuilder::OPCODES::BC_00,
   DUP = OMR::InterpreterBuilder::OPCODES::BC_01,
   ADD = OMR::InterpreterBuilder::OPCODES::BC_02,
   SUB = OMR::InterpreterBuilder::OPCODES::BC_03,
   MUL = OMR::InterpreterBuilder::OPCODES::BC_04,
   DIV = OMR::InterpreterBuilder::OPCODES::BC_05,
   RET = OMR::InterpreterBuilder::OPCODES::BC_06,
   FAIL = OMR::InterpreterBuilder::OPCODES::BC_07
   };

using std::cout;
using std::cerr;

//#define PRINTSTRING(builder, value) ((builder)->Call("printString", 1, (builder)->ConstInt64((int64_t)value)))
//#define PRINTINT64(builder, value) ((builder)->Call("printInt64", 1, (builder)->ConstInt64((int64_t)value))
//#define PRINTINT64VALUE(builder, value) ((builder)->Call("printInt64", 1, value))

int
main(int argc, char *argv[])
   {

   cout << "Step 1: initialize JIT\n";
   bool initialized = initializeJit();
   if (!initialized)
      {
      cerr << "FAIL: could not initialize JIT\n";
      exit(-1);
      }

   cout << "Step 2: define type dictionary\n";
   TR::TypeDictionary types;

   cout << "Step 3: compile method builder\n";
   InterpreterMethod method(&types);
   uint8_t *entry = 0;
   int32_t rc = compileMethodBuilder(&method, &entry);
   if (rc != 0)
      {
      cerr << "fail: compilation error " << rc << "\n";
      exit(-2);
      }

   cout << "step 4: invoke compiled code and print results\n";
   typedef int64_t (InterpreterMethodFunction)(int64_t **, int8_t *);
   InterpreterMethodFunction *interpreter = (InterpreterMethodFunction *) entry;
   int64_t *stack = (int64_t *)malloc(sizeof(int64_t) * 10);
   if (NULL == stack)
      {
      cerr << "fail: failed to allocated stack\n";
      exit(-3);
      }
   int8_t bytecodes[] =
      {
      interpreter_opcodes::PUSH_CONSTANT,3, // push 3
      interpreter_opcodes::PUSH_CONSTANT,5, // push 5
      interpreter_opcodes::ADD,-1,          // add 3 + 5 store 8
      interpreter_opcodes::PUSH_CONSTANT,2, // push 2
      interpreter_opcodes::SUB,-1,          // sub 8 - 2 store 6
      interpreter_opcodes::PUSH_CONSTANT,4, // push 4
      interpreter_opcodes::MUL,-1,          // mul 6 * 4 store 24
      interpreter_opcodes::PUSH_CONSTANT,8, // push 8
      interpreter_opcodes::DIV,-1,          // div 24 / 8 store 3
      interpreter_opcodes::DUP,-1,          // dup 3 store 3
      interpreter_opcodes::ADD,-1,          // add 3 + 3 store 6

      //interpreter_opcodes::FAIL,-1,

      interpreter_opcodes::RET,-1, // return 6
      };

   int64_t result = interpreter(&stack, bytecodes);

   free(stack);

   cout << "interpreter(values) = " << result << "\n";

   cout << "Step 5: shutdown JIT\n";
   shutdownJit();
   }

InterpreterMethod::InterpreterMethod(TR::TypeDictionary *d)
   : InterpreterBuilder(d, "bytecodes", d->toIlType<uint8_t>(), "pc", "opcode")
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   pInt8 = d->PointerTo(Int8);

   DefineName("Interpreter");

   DefineParameter("stackPtrPtr", d->PointerTo(d->PointerTo(Int64)));
   DefineParameter("bytecodes", pInt8);

   DefineLocal("pc", Int32);
   DefineLocal("opcode", Int32);

   DefineReturnType(Int64);
   }

TR::VirtualMachineInterpreterStack *
InterpreterMethod::createStack()
   {
   TR::VirtualMachineRegister *stackRegister = new TR::VirtualMachineRegister(this, "_STACK_", _types->PointerTo(_types->PointerTo(Int64)), sizeof(Int64), Load("stackPtrPtr"));
   return new TR::VirtualMachineInterpreterStack(this, stackRegister, Int64);
   }

void
InterpreterMethod::handleOpcodes()
   {
   registerOpcodeBuilder(PushBuilder::OrphanOpcodeBuilder(this, interpreter_opcodes::PUSH_CONSTANT));
   registerOpcodeBuilder(DupBuilder::OrphanOpcodeBuilder(this, interpreter_opcodes::DUP));
   registerOpcodeBuilder(MathBuilder::OrphanOpcodeBuilder(this, interpreter_opcodes::ADD, &MathBuilder::add));
   registerOpcodeBuilder(MathBuilder::OrphanOpcodeBuilder(this, interpreter_opcodes::SUB, &MathBuilder::sub));
   registerOpcodeBuilder(MathBuilder::OrphanOpcodeBuilder(this, interpreter_opcodes::MUL, &MathBuilder::mul));
   registerOpcodeBuilder(MathBuilder::OrphanOpcodeBuilder(this, interpreter_opcodes::DIV, &MathBuilder::div));
   registerOpcodeBuilder(RetBuilder::OrphanOpcodeBuilder(this, interpreter_opcodes::RET));
   }

void
InterpreterMethod::handleReturn(TR::IlBuilder *builder)
   {
   builder->Return(
   builder->   ConstInt64(-1));
   }

