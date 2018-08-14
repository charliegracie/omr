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

#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/VirtualMachineInterpreterStack.hpp"

#include "InterpreterTypes.h"
#include "CallBuilder.hpp"

static Frame* callHelper(Frame *frame, int32_t pc, int8_t methodIndex, int8_t argCount)
   {
   #define CALLHELPER_LINE LINETOSTR(__LINE__)

   Frame *newFrame = (Frame *)malloc(sizeof(Frame));
   if (NULL == frame)
      {
      int *x = 0;
      fprintf(stderr, "Unable to allocate frame for call\n");
      *x = 0;
      }

   Method *methodToCall = &frame->methods[methodIndex];

   frame->savedPC = pc;

   int32_t callsUntilJit = methodToCall->callsUntilJit;
   if (callsUntilJit > 0)
      {
      methodToCall->callsUntilJit = callsUntilJit - 1;
      }
   else if (0 == callsUntilJit)
      {
      //TODO register for compilation
      }

   newFrame->methods = frame->methods;
   newFrame->locals = newFrame->loc;
   newFrame->sp = newFrame->stack;
   newFrame->bytecodes = methodToCall->bytecodes;
   newFrame->previous = frame;

   memset(newFrame->loc, 0, sizeof(newFrame->loc));
   memset(newFrame->stack, 0, sizeof(newFrame->stack));

   for (int8_t i = 0; i < argCount; i++)
      {
      frame->sp--;
      newFrame->stack[argCount - 1 - i] = *frame->sp;
      newFrame->sp++;
      }

   if (NULL != methodToCall->compiledMethod)
      {
      newFrame->frameType = JIT;
      JitMethodFunction *compiledMethod = methodToCall->compiledMethod;
      compiledMethod(newFrame);
      return frame;
      }
   else
      {
      newFrame->frameType = JIT;
      return newFrame;
      }
   }

CallBuilder::CallBuilder(TR::MethodBuilder *methodBuilder, int32_t bcIndex, TR::IlType *frameType)
   : BytecodeBuilder(methodBuilder, bcIndex, "CALL"),
   _frameType(frameType)
   {
   }

CallBuilder *
CallBuilder::OrphanBytecodeBuilder(TR::MethodBuilder *methodBuilder, int32_t bcIndex, TR::IlType *frameType)
   {
   CallBuilder *orphan = new CallBuilder(methodBuilder, bcIndex, frameType);
   methodBuilder->InitializeBytecodeBuilder(orphan);

   methodBuilder->DefineFunction((char *)"callHelper", (char *)__FILE__, (char *)CALLHELPER_LINE, (void *)&callHelper, frameType, 4, frameType, methodBuilder->typeDictionary()->PrimitiveType(TR::Int32), methodBuilder->typeDictionary()->PrimitiveType(TR::Int8), methodBuilder->typeDictionary()->PrimitiveType(TR::Int8));

   return orphan;
   }

void
CallBuilder::execute()
   {
   TR::VirtualMachineInterpreterStack *state = (TR::VirtualMachineInterpreterStack*)vmState();
   TR::IlType *pInt8 = _types->PointerTo(Int8);
   TR::IlValue *pc = Load("pc");

   TR::IlValue *methodIndex =
   LoadAt(pInt8,
      IndexAt(pInt8,
         Load("bytecodes"),
         Add(
            pc,
            ConstInt32(1))));

   TR::IlValue *argCount =
   LoadAt(pInt8,
      IndexAt(pInt8,
         Load("bytecodes"),
         Add(
            pc,
            ConstInt32(2))));

   state->Commit(this);

   TR::IlValue *newFrame = Call("callHelper", 4, Load("frame"), pc, methodIndex, argCount);
   Store("frame", newFrame);

   TR::IlValue *bytecodesAddress = StructFieldInstanceAddress("Frame", "bytecodes", newFrame);
   TR::IlValue *bytecodes = LoadAt(_types->PointerTo(_types->PointerTo(Int8)), bytecodesAddress);
   Store("bytecodes", bytecodes);

   Store("pc", ConstInt32(0));//TODO verify that PC should be reset here

   state->Reload(this);
   }

