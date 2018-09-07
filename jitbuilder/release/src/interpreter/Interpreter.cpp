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
#include "ilgen/VirtualMachineInterpreterStack.hpp"
#include "ilgen/VirtualMachineInterpreterArray.hpp"
#include "ilgen/VirtualMachineState.hpp"
#include "ilgen/VirtualMachineRegister.hpp"
#include "ilgen/VirtualMachineRegisterInStruct.hpp"

#include "InterpreterTypes.h"

#include "InterpreterTypeDictionary.hpp"
#include "Interpreter.hpp"
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

using std::cout;
using std::cerr;

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
   InterpreterTypeDictionary types;

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

   Interpreter interp;
   interp.methods = interpreterMethod._methods;

   Frame frame;
   frame.args = frame.a;
   frame.locals = frame.loc;
   frame.sp = frame.stack;
   frame.bytecodes = interp.methods[0].bytecodes;
   frame.previous = NULL;
   frame.savedPC = 0;
   frame.frameType = FRAME_TYPE_INTERPRETER | FRAME_TYPE_ALLOCATED_ONSTACK;

   memset(frame.stack, 0, sizeof(frame.stack));
   memset(frame.loc, 0, sizeof(frame.loc));

   interp.currentFrame = &frame;

   int64_t result = interpreter(&interp, &frame);

   cout << "interpreter(values) = " << result << "\n";

   cout << "Step 5: shutdown JIT\n";
   shutdownJit();
   }

InterpreterMethod::InterpreterMethod(InterpreterTypeDictionary *d)
   : InterpreterBuilder(d, "bytecodes", d->toIlType<uint8_t>(), "pc", "opcode"),
   _interpTypes(d)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   pInt8 = d->PointerTo(Int8);

   DefineName("Interpreter");

   DefineParameter("interp", _interpTypes->getTypes().pInterpreter);
   DefineParameter("frame", _interpTypes->getTypes().pFrame);

   DefineReturnType(Int64);

   for (int32_t i = 0; i < _methodCount; i++)
      {
      _methods[i].callsUntilJit = 10;
      _methods[i].compiledMethod = NULL;
      }

   _methods[0].bytecodes = _mainMethod;
   _methods[0].name = "MainMethod";
   _methods[0].bytecodeLength = sizeof(_mainMethod);
   _methods[0].argCount = 0;
   _methods[1].bytecodes = _testCallMethod;
   _methods[1].name = "TestCallMethod";
   _methods[1].bytecodeLength = sizeof(_testCallMethod);
   _methods[1].argCount = 0;
   _methods[2].bytecodes = _testDivMethod;
   _methods[2].name = "TestDivMethod";
   _methods[2].bytecodeLength = sizeof(_testDivMethod);
   _methods[2].argCount = 2;
   _methods[3].bytecodes = _testAddMethod;
   _methods[3].name = "TestAddMethod";
   _methods[3].bytecodeLength = sizeof(_testAddMethod);
   _methods[3].argCount = 2;
   _methods[4].bytecodes = _testJMPLMethod;
   _methods[4].name = "TestJMPLMethod";
   _methods[4].bytecodeLength = sizeof(_testJMPLMethod);
   _methods[4].argCount = 1;
   _methods[5].bytecodes = _fib;
   _methods[5].name = "Fib";
   _methods[5].bytecodeLength = sizeof(_fib);
   _methods[5].argCount = 1;
   _methods[6].bytecodes = _iterFib;
   _methods[6].name = "IterFib";
   _methods[6].bytecodeLength = sizeof(_iterFib);
   _methods[6].argCount = 1;
   _methods[7].bytecodes = _testJMPGMethod;
   _methods[7].name = "TestJMPGMethod";
   _methods[7].bytecodeLength = sizeof(_testJMPGMethod);
   _methods[7].argCount = 1;
   }

void
InterpreterMethod::DefineFunctions(TR::MethodBuilder *mb)
   {
   BytecodeHelpers::DefineFunctions(mb);
   }

TR::VirtualMachineState *
InterpreterMethod::createVMState()
   {
   TR::VirtualMachineRegisterInStruct *stackRegister = new TR::VirtualMachineRegisterInStruct(this, "Frame", "frame", "sp", "SP");
   TR::VirtualMachineInterpreterStack *stack = new TR::VirtualMachineInterpreterStack(this, stackRegister, STACKVALUEILTYPE, true, false);

   TR::VirtualMachineRegisterInStruct *localsRegister = new TR::VirtualMachineRegisterInStruct(this, "Frame", "frame", "locals", "LOCALS");
   TR::VirtualMachineInterpreterArray *localsArray = new TR::VirtualMachineInterpreterArray(this, STACKVALUEILTYPE, localsRegister);

   TR::VirtualMachineRegisterInStruct *argsRegister = new TR::VirtualMachineRegisterInStruct(this, "Frame", "frame", "args", "ARGS");
   TR::VirtualMachineInterpreterArray *argsArray = new TR::VirtualMachineInterpreterArray(this, STACKVALUEILTYPE, argsRegister);

   InterpreterVMState *vmState = new InterpreterVMState(stack, stackRegister, localsArray, localsRegister, argsArray, argsRegister);
   return vmState;
   }

void
InterpreterMethod::loadBytecodes(TR::IlBuilder *builder)
   {
   TR::IlValue *frame = builder->Load("frame");
   TR::IlValue *bytecodesAddress = builder->StructFieldInstanceAddress("Frame", "bytecodes", frame);
   TR::IlType *ppInt8 = _types->PointerTo(pInt8);
   TR::IlValue *bytecodes = builder->LoadAt(ppInt8, bytecodesAddress);

   setBytecodes(builder, bytecodes);
   }

void
InterpreterMethod::loadPC(TR::IlBuilder *builder)
   {
   TR::IlValue *frame = builder->Load("frame");
   TR::IlValue *pcAddress = builder->StructFieldInstanceAddress("Frame", "savedPC", frame);
   TR::IlType *pInt32 = _types->PointerTo(Int32);
   TR::IlValue *pc = builder->LoadAt(pInt32, pcAddress);

   setPC(builder, pc);
   }

void
InterpreterMethod::savePC(TR::IlBuilder *builder, TR::IlValue *pc)
   {
   TR::IlValue *frame = builder->Load("frame");
   TR::IlValue *pcAddress = builder->StructFieldInstanceAddress("Frame", "savedPC", frame);
   builder->StoreAt(pcAddress, pc);
   }

void
InterpreterMethod::registerBytecodeBuilders()
   {
   registerBytecodeBuilder(OrphanBytecodeBuilder<PushConstantBuilder>(interpreter_opcodes::PUSH_CONSTANT, "PUSH_CONSTNT"));
   registerBytecodeBuilder(OrphanBytecodeBuilder<PushArgBuilder>(interpreter_opcodes::PUSH_ARG, "PUSH_ARG"));
   registerBytecodeBuilder(OrphanBytecodeBuilder<PushLocalBuilder>(interpreter_opcodes::PUSH_LOCAL, "PUSH_LOCAL"));
   registerBytecodeBuilder(OrphanBytecodeBuilder<PopLocalBuilder>(interpreter_opcodes::POP_LOCAL, "POP_LOCAL"));
   registerBytecodeBuilder(OrphanBytecodeBuilder<DupBuilder>(interpreter_opcodes::DUP, "DUP"));

   MathBuilder *math = OrphanBytecodeBuilder<MathBuilder>(interpreter_opcodes::ADD, "ADD");
   math->setFunction(&MathBuilder::add);
   registerBytecodeBuilder(math);

   math = OrphanBytecodeBuilder<MathBuilder>(interpreter_opcodes::SUB, "SUB");
   math->setFunction(&MathBuilder::sub);
   registerBytecodeBuilder(math);

   math = OrphanBytecodeBuilder<MathBuilder>(interpreter_opcodes::MUL, "MUL");
   math->setFunction(&MathBuilder::mul);
   registerBytecodeBuilder(math);

   math = OrphanBytecodeBuilder<MathBuilder>(interpreter_opcodes::DIV, "DIV");
   math->setFunction(&MathBuilder::div);
   registerBytecodeBuilder(math);

   JumpIfBuilder *jump = OrphanBytecodeBuilder<JumpIfBuilder>(interpreter_opcodes::JMPL, "JMPL");
   jump->setFunction(&JumpIfBuilder::lessThan);
   registerBytecodeBuilder(jump);

   jump = OrphanBytecodeBuilder<JumpIfBuilder>(interpreter_opcodes::JMPG, "JMPG");
   jump->setFunction(&JumpIfBuilder::greaterThan);
   registerBytecodeBuilder(jump);

   registerBytecodeBuilder(OrphanBytecodeBuilder<CallBuilder>(interpreter_opcodes::CALL, "CALL"));
   registerBytecodeBuilder(OrphanBytecodeBuilder<RetBuilder>(interpreter_opcodes::RET, "RET"));
   registerBytecodeBuilder(OrphanBytecodeBuilder<ExitBuilder>(interpreter_opcodes::EXIT, "EXIT"));
   }

void
InterpreterMethod::handleInterpreterExit(TR::IlBuilder *builder)
   {
   builder->Return(
   builder->   ConstInt64(-1));
   }
