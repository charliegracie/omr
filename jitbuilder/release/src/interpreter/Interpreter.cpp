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
   InterpreterMethodFunction *interpreter = (InterpreterMethodFunction *) entry;

   Frame frame;
   frame.methods = interpreterMethod._methods;
   frame.locals = frame.loc;
   frame.sp = frame.stack;
   frame.bytecodes = interpreterMethod._methods[0].bytecodes;
   frame.previous = NULL;
   frame.savedPC = 0;
   frame.frameType = INTERPRETER;

   memset(frame.stack, 0, sizeof(frame.stack));
   memset(frame.loc, 0, sizeof(frame.loc));

   int64_t result = interpreter(NULL, &frame);

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

   TR::IlType *meth = d->DefineStruct("Method");
   TR::IlType *pMeth = d->PointerTo(meth);
   d->DefineField("Method", "callsUntilJit", Int32, offsetof(Method, callsUntilJit));
   d->DefineField("Method", "bytecodes", pInt8, offsetof(Method, bytecodes));
   d->DefineField("Method", "compiledMethod", Address, offsetof(Method, compiledMethod));
   d->CloseStruct("Method");

   frame = d->DefineStruct("Frame");
   pFrame = d->PointerTo(frame);
   d->DefineField("Frame", "previous", pFrame, offsetof(Frame, previous));
   d->DefineField("Frame", "savedPC", pFrame, offsetof(Frame, savedPC));
   d->DefineField("Frame", "bytecodes", pInt8, offsetof(Frame, bytecodes));
   d->DefineField("Frame", "locals", d->PointerTo(STACKVALUEILTYPE), offsetof(Frame, locals));
   d->DefineField("Frame", "sp", d->PointerTo(STACKVALUEILTYPE), offsetof(Frame, sp));
   d->CloseStruct("Frame");

   TR::IlType *interp = d->DefineStruct("Interpreter");
   TR::IlType *pInterp = d->PointerTo(interp);
   d->DefineField("Interpreter", "currentFrame", pFrame, offsetof(Interpreter, currentFrame));
   d->DefineField("Interpreter", "methods", pMeth, offsetof(Interpreter, methods));
   d->CloseStruct("Interpreter");

   DefineParameter("interp", pInterp);
   DefineParameter("frame", pFrame);

   DefineLocal("pc", Int32);
   DefineLocal("opcode", Int32);

   DefineReturnType(Int64);

   for (int32_t i = 0; i < _methodCount; i++)
      {
      _methods[i].callsUntilJit = 10;
      _methods[i].compiledMethod = NULL;
      }

   _methods[0].bytecodes = _mainMethod;
   _methods[1].bytecodes = _testCallMethod;
   _methods[2].bytecodes = _testDivMethod;
   _methods[3].bytecodes = _testAddMethod;
   _methods[4].bytecodes = _testJMPLMethod;
   _methods[5].bytecodes = _fib;
   _methods[6].bytecodes = _iterFib;
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

