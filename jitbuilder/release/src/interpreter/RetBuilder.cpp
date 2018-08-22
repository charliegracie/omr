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

#include <new>

#include "ilgen/RuntimeBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/VirtualMachineInterpreterStack.hpp"

#include "InterpreterTypes.h"
#include "RetBuilder.hpp"

static void freeFrame(Frame *frame)
   {
   #define FREEFRAME_LINE LINETOSTR(__LINE__)

   //TODO add check to see if malloc'd
   free(frame);
   }

static void j2iReturn(Interpreter *interp, Frame *frame, int64_t retVal)
   {
#define J2IRETURN_LINE LINETOSTR(__LINE__)
   *frame->sp = retVal;
   frame->sp++;
   frame->savedPC += 3;
   }

static void j2jReturn(Interpreter *interp, Frame *frame, int64_t retVal)
   {
#define J2JRETURN_LINE LINETOSTR(__LINE__)
   *frame->sp = retVal;
   frame->sp++;
   }

static void i2iReturn(Interpreter *interp, Frame *frame, int64_t retVal)
   {
#define I2IRETURN_LINE LINETOSTR(__LINE__)
   *frame->sp = retVal;
   frame->sp++;
   }

static void i2jReturn(Interpreter *interp, Frame *frame, int64_t retVal)
   {
#define I2JRETURN_LINE LINETOSTR(__LINE__)
   int *x = 0;
   fprintf(stderr, "i2jReturn not handled\n");
   *x = 0;
   }

RetBuilder::RetBuilder(TR::RuntimeBuilder *runtimeBuilder, int32_t bcIndex, TR::IlType *frameType)
   : BytecodeBuilder(runtimeBuilder, bcIndex, "RET", 2),
   _runtimeBuilder(runtimeBuilder),
   _frameType(frameType)
   {
   }

void
RetBuilder::DefineFunctions(TR::RuntimeBuilder *runtimeBuilder, TR::IlType *interpType, TR::IlType *frameType)
   {
   TR::IlType *voidType = runtimeBuilder->typeDictionary()->toIlType<void>();
   TR::IlType *Int64Type = runtimeBuilder->typeDictionary()->toIlType<int64_t>();

   runtimeBuilder->DefineFunction((char *)"freeFrame", (char *)__FILE__, (char *)FREEFRAME_LINE, (void *)&freeFrame, voidType, 1, frameType);
   runtimeBuilder->DefineFunction((char *)"j2iReturn", (char *)__FILE__, (char *)J2IRETURN_LINE, (void *)&j2iReturn, voidType, 3, interpType, frameType, Int64Type);
   runtimeBuilder->DefineFunction((char *)"j2jReturn", (char *)__FILE__, (char *)J2JRETURN_LINE, (void *)&j2jReturn, voidType, 3, interpType, frameType, Int64Type);
   runtimeBuilder->DefineFunction((char *)"i2iReturn", (char *)__FILE__, (char *)I2IRETURN_LINE, (void *)&i2iReturn, voidType, 3, interpType, frameType, Int64Type);
   runtimeBuilder->DefineFunction((char *)"i2jReturn", (char *)__FILE__, (char *)I2JRETURN_LINE, (void *)&i2jReturn, voidType, 3, interpType, frameType, Int64Type);
   }

RetBuilder *
RetBuilder::OrphanBytecodeBuilder(TR::RuntimeBuilder *runtimeBuilder, int32_t bcIndex, TR::IlType *frameType)
   {
   RetBuilder *orphan = new RetBuilder(runtimeBuilder, bcIndex, frameType);
   runtimeBuilder->InitializeBytecodeBuilder(orphan);
   return orphan;
   }

void
RetBuilder::execute()
   {
   InterpreterVMState *state = (InterpreterVMState*)vmState();
   TR::VirtualMachineStack *stack = state->_stack;

   TR::IlValue *retVal = stack->Pop(this);
   state->Commit(this);

   TR::IlValue *currentFrameAddress = StructFieldInstanceAddress("Interpreter", "currentFrame", Load("interp"));
   TR::IlValue *currentFrame = LoadAt(_types->PointerTo(_frameType), currentFrameAddress);

   TR::IlValue *previousAddress = StructFieldInstanceAddress("Frame", "previous", currentFrame);
   TR::IlValue *previous = LoadAt(_types->PointerTo(_frameType), previousAddress);

   TR::IlValue *currentFrameFrameTypeAddress = StructFieldInstanceAddress("Frame", "frameType", currentFrame);
   TR::IlValue *currentFrameFrameType = LoadAt(_types->PointerTo(Int32), currentFrameFrameTypeAddress);

   Call("freeFrame", 1, currentFrame);

   StoreAt(currentFrameAddress, previous);
   Store("frame", previous);

   TR::IlValue *bytecodesAddress = StructFieldInstanceAddress("Frame", "bytecodes", previous);
   TR::IlValue *bytecodes = LoadAt(_types->PointerTo(_types->PointerTo(Int8)), bytecodesAddress);
   Store("bytecodes", bytecodes);

//   TR::IlValue *pcAddress = StructFieldInstanceAddress("Frame", "savedPC", previous);
//   TR::IlValue *pc = LoadAt(_types->PointerTo(Int32), pcAddress);
//   Store("pc", pc);

   TR::IlValue *previousFrameFrameTypeAddress = StructFieldInstanceAddress("Frame", "frameType", previous);
   TR::IlValue *previousFrameFrameType = LoadAt(_types->PointerTo(Int32), previousFrameFrameTypeAddress);

   TR::IlBuilder *i2_Transition = NULL;
   TR::IlBuilder *j2_Transition = NULL;
   IfThenElse(&i2_Transition, &j2_Transition,
      EqualTo(currentFrameFrameType,
         ConstInt32(INTERPRETER)));

   //transition from the interpreter
   TR::IlBuilder *i2iTransition = NULL;
   TR::IlBuilder *i2jTransition = NULL;
   i2_Transition->IfThenElse(&i2iTransition, &i2jTransition,
   i2_Transition->   EqualTo(previousFrameFrameType,
   i2_Transition->     ConstInt32(INTERPRETER)));

   //i2i return
   i2iTransition->Call("i2iReturn", 3, i2iTransition->Load("interp"), i2iTransition->Load("frame"), retVal);

   //i2j return
   i2jTransition->Call("i2jReturn", 3, i2jTransition->Load("interp"), i2jTransition->Load("frame"), retVal);

   //transition from the jit
   TR::IlBuilder *j2iTransition = NULL;
   TR::IlBuilder *j2jTransition = NULL;
   j2_Transition->IfThenElse(&j2iTransition, &j2jTransition,
   j2_Transition->   EqualTo(previousFrameFrameType,
   j2_Transition->     ConstInt32(INTERPRETER)));

   //j2i return
   j2iTransition->Call("j2iReturn", 3, j2iTransition->Load("interp"), j2iTransition->Load("frame"), retVal);

   //j2j return
   j2jTransition->Call("j2jReturn", 3, j2jTransition->Load("interp"), j2jTransition->Load("frame"), retVal);

   state->Reload(this);

   _runtimeBuilder->ReturnTarget(this);
   }

