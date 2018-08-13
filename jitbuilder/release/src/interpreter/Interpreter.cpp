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
#include "ilgen/VirtualMachineRegisterInStruct.hpp"

#include "InterpreterTypes.h"

#include "Interpreter.hpp"
#include "PushConstantBuilder.hpp"
#include "DupBuilder.hpp"
#include "MathBuilder.hpp"
#include "RetBuilder.hpp"
#include "ExitBuilder.hpp"
#include "CallBuilder.hpp"
#include "JumpBuilder.hpp"
#include "PopLocalBuilder.hpp"
#include "PushLocalBuilder.hpp"

enum interpreter_opcodes
   {
   PUSH_CONSTANT = OMR::InterpreterBuilder::OPCODES::BC_00,
   DUP = OMR::InterpreterBuilder::OPCODES::BC_01,
   ADD = OMR::InterpreterBuilder::OPCODES::BC_02,
   SUB = OMR::InterpreterBuilder::OPCODES::BC_03,
   MUL = OMR::InterpreterBuilder::OPCODES::BC_04,
   DIV = OMR::InterpreterBuilder::OPCODES::BC_05,
   RET = OMR::InterpreterBuilder::OPCODES::BC_06,
   CALL = OMR::InterpreterBuilder::OPCODES::BC_07,
   EXIT = OMR::InterpreterBuilder::OPCODES::BC_08,
   JMPL = OMR::InterpreterBuilder::OPCODES::BC_09,
   PUSH_LOCAL = OMR::InterpreterBuilder::OPCODES::BC_10,
   POP_LOCAL = OMR::InterpreterBuilder::OPCODES::BC_11,
   FAIL = OMR::InterpreterBuilder::OPCODES::BC_12
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
   InterpreterMethod interpreterMethod(&types);
   uint8_t *entry = 0;
   int32_t rc = compileMethodBuilder(&interpreterMethod, &entry);
   if (rc != 0)
      {
      cerr << "fail: compilation error " << rc << "\n";
      exit(-2);
      }

   cout << "step 4: invoke compiled code and print results\n";
   typedef int64_t (InterpreterMethodFunction)(Frame *);
   InterpreterMethodFunction *interpreter = (InterpreterMethodFunction *) entry;

   int8_t method0[] =
      {
      interpreter_opcodes::PUSH_CONSTANT,3, // push 3
      interpreter_opcodes::PUSH_CONSTANT,5, // push 5
      interpreter_opcodes::ADD,             // add 3 + 5 store 8
      interpreter_opcodes::PUSH_CONSTANT,2, // push 2
      interpreter_opcodes::SUB,             // sub 8 - 2 store 6
      interpreter_opcodes::PUSH_CONSTANT,4, // push 4
      interpreter_opcodes::MUL,             // mul 6 * 4 store 24
      interpreter_opcodes::PUSH_CONSTANT,8, // push 8
      interpreter_opcodes::DIV,             // div 24 / 8 store 3
      interpreter_opcodes::DUP,             // dup 3 store 3
      interpreter_opcodes::ADD,             // add 3 + 3 store 6
      interpreter_opcodes::CALL,1,0,        // call method 1 (call result 8)
      interpreter_opcodes::ADD,             // add 6 + 8 (call result) store 14
      interpreter_opcodes::PUSH_CONSTANT,2, // push 2
      interpreter_opcodes::CALL,2,2,        // call method 2 (call result 7)
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 3)
      interpreter_opcodes::PUSH_CONSTANT,4, // push 4
      interpreter_opcodes::MUL,             // mul 3 * 4 store 12
      interpreter_opcodes::CALL,5,1,        // call method 5 (call result fib(12))

      //interpreter_opcodes::FAIL,-1,

      interpreter_opcodes::EXIT,-1, // return 144 (call result)
      };

   int8_t method1[] =
      {
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::PUSH_CONSTANT,7, // push 7
      interpreter_opcodes::CALL,3,2,        // call method 3 (call result 8)
      interpreter_opcodes::RET,1,           // ret 8
      };

   int8_t method2[] =
      {
      //Expecting 2 args to be pushed!
      interpreter_opcodes::DIV,             // arg 1 (14) / arg2 (2) store 7
      interpreter_opcodes::RET,1,           // ret 7
      };

   int8_t method3[] =
      {
      //Expecting 2 args to be pushed!
      interpreter_opcodes::ADD,             // arg 1 (1) + arg2 (7) store 8
      interpreter_opcodes::RET,1,           // ret 8
      };

   int8_t method4[] =
      {
      //Expecting 1 arg
      interpreter_opcodes::PUSH_CONSTANT,5, // push 5
      interpreter_opcodes::JMPL,8,          // if arg < 5
      interpreter_opcodes::PUSH_CONSTANT,3, // push 1
      interpreter_opcodes::RET,1,           // ret 3
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::RET,1,           // ret 1
      };

   int8_t fib[] =
      {
      // Expecting 1 arg
      // save arg in locals[0]
      interpreter_opcodes::DUP,             // dup arg store arg..........stack is arg,arg
      interpreter_opcodes::POP_LOCAL,0,     // pop arg into local 0.......stack is arg
      // if arg < 3 goto push 1/ret
      interpreter_opcodes::PUSH_CONSTANT,2, // push 2
      interpreter_opcodes::JMPL,34,         // if arg < 2
      // call f(n-1)
      interpreter_opcodes::PUSH_LOCAL,0,    // push local 0...............stack is arg
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1.....................stack is arg,1
      interpreter_opcodes::SUB,             // arg - 1 store (arg - 1)....stack is (arg-1)
      interpreter_opcodes::CALL,5,1,        // call method fib............stack is fib(arg-1)
      interpreter_opcodes::POP_LOCAL,1,     // pop into local 1...........stack is
      // call fib(n-2)
      interpreter_opcodes::PUSH_LOCAL,0,    // push local 0...............stack is arg
      interpreter_opcodes::PUSH_CONSTANT,2, // push 2.....................stack is arg,2
      interpreter_opcodes::SUB,             // arg - 2 store (arg - 2)....stack is (arg-2)
      interpreter_opcodes::CALL,5,1,        // call method fib............stack is fib(arg-2)
      interpreter_opcodes::POP_LOCAL,0,     // pop into local 0...........stack is
      // add fib(n-1) + fib(n-2)
      interpreter_opcodes::PUSH_LOCAL,1,    // push local 1..............stack is fib(arg-1)
      interpreter_opcodes::PUSH_LOCAL,0,    // push local 0..............stack is fib(arg-2)
      interpreter_opcodes::ADD,             // fib(arg-1) + fib(arg-2)...stack is (fib(arg-1) + fib(arg-2))
      interpreter_opcodes::RET,1,           // ret fib(arg-1) + fib(arg-2)
      // return 1
      interpreter_opcodes::PUSH_LOCAL,0,    // push arg
      interpreter_opcodes::RET,1,           // ret arg
      };

   int8_t *methods[6];
   methods[0] = method0;
   methods[1] = method1;
   methods[2] = method2;
   methods[3] = method3;
   methods[4] = method4;
   methods[5] = fib;

   Frame frame;
   frame.methods = methods;
   frame.locals = frame.loc;
   frame.sp = frame.stack;
   frame.bytecodes = method0;
   frame.previous = NULL;
   frame.savedPC = 0;

   memset(frame.stack, 0, sizeof(frame.stack));
   memset(frame.loc, 0, sizeof(frame.loc));

   int64_t result = interpreter(&frame);

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

   frame = d->DefineStruct("Frame");
   pFrame = d->PointerTo(frame);
   d->DefineField("Frame", "previous", pFrame, offsetof(Frame, previous));
   d->DefineField("Frame", "savedPC", pFrame, offsetof(Frame, savedPC));
   d->DefineField("Frame", "bytecodes", pInt8, offsetof(Frame, bytecodes));
   d->DefineField("Frame", "locals", d->PointerTo(STACKVALUEILTYPE), offsetof(Frame, locals));
   d->DefineField("Frame", "sp", d->PointerTo(STACKVALUEILTYPE), offsetof(Frame, sp));
   d->CloseStruct("Frame");

   DefineParameter("frame", pFrame);

   DefineLocal("pc", Int32);
   DefineLocal("opcode", Int32);

   DefineReturnType(Int64);
   }

TR::VirtualMachineInterpreterStack *
InterpreterMethod::createStack()
   {
   TR::VirtualMachineRegisterInStruct *stackRegister = new TR::VirtualMachineRegisterInStruct(this, "Frame", "frame", "sp", "SP");
   return new TR::VirtualMachineInterpreterStack(this, stackRegister, STACKVALUEILTYPE);
   }

void
InterpreterMethod::loadOpcodeArray()
   {
   TR::IlValue *bytecodesAddress = StructFieldInstanceAddress("Frame", "bytecodes", Load("frame"));
   TR::IlValue *bytecodes = LoadAt(_types->PointerTo(pInt8), bytecodesAddress);
   Store("bytecodes", bytecodes);
   }

void
InterpreterMethod::registerBytecodeBuilders()
   {
   registerBytecodeBuilder(PushConstantBuilder::OrphanBytecodeBuilder(this, interpreter_opcodes::PUSH_CONSTANT), 2);
   registerBytecodeBuilder(DupBuilder::OrphanBytecodeBuilder(this, interpreter_opcodes::DUP), 1);
   registerBytecodeBuilder(MathBuilder::OrphanBytecodeBuilder(this, interpreter_opcodes::ADD, &MathBuilder::add), 1);
   registerBytecodeBuilder(MathBuilder::OrphanBytecodeBuilder(this, interpreter_opcodes::SUB, &MathBuilder::sub), 1);
   registerBytecodeBuilder(MathBuilder::OrphanBytecodeBuilder(this, interpreter_opcodes::MUL, &MathBuilder::mul), 1);
   registerBytecodeBuilder(MathBuilder::OrphanBytecodeBuilder(this, interpreter_opcodes::DIV, &MathBuilder::div), 1);
   registerBytecodeBuilder(RetBuilder::OrphanBytecodeBuilder(this, interpreter_opcodes::RET, pFrame), 3); //PC increment of 3 to handle previous call
   registerBytecodeBuilder(CallBuilder::OrphanBytecodeBuilder(this, interpreter_opcodes::CALL, pFrame), 0); //PC increment of 0 to handle starting at pc 0
   registerBytecodeBuilder(JumpBuilder::OrphanBytecodeBuilder(this, interpreter_opcodes::JMPL), 0); //PC increment of 0 since Jump sets PC
   registerBytecodeBuilder(PopLocalBuilder::OrphanBytecodeBuilder(this, interpreter_opcodes::POP_LOCAL), 2);
   registerBytecodeBuilder(PushLocalBuilder::OrphanBytecodeBuilder(this, interpreter_opcodes::PUSH_LOCAL), 2);
   registerBytecodeBuilder(ExitBuilder::OrphanBytecodeBuilder(this, interpreter_opcodes::EXIT), 2);
   }

void
InterpreterMethod::handleReturn(TR::IlBuilder *builder)
   {
   builder->Return(
   builder->   ConstInt64(-1));
   }

