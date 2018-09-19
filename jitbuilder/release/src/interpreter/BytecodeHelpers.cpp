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
#include <time.h>
#include <sys/time.h>

#include "Jit.hpp"
#include "InterpreterTypes.h"
#include "BytecodeHelpers.hpp"
#include "InterpreterTypeDictionary.hpp"
#include "JitMethod.hpp"

Frame *
BytecodeHelpers::allocateFrame()
   {
#define ALLOCATEFRAME_LINE LINETOSTR(__LINE__)

   Frame *newFrame = (Frame *)malloc(sizeof(Frame));
   if (NULL == newFrame)
      {
      int *x = 0;
      fprintf(stderr, "Unable to allocate frame for call\n");
      *x = 0;
      }

#if FRAME_ZERO
   memset(newFrame, 0, sizeof(Frame));
#endif

   newFrame->args = newFrame->a;
   newFrame->locals = newFrame->loc;
   newFrame->sp = newFrame->stack;
   newFrame->savedPC = 0;
   newFrame->frameType = FRAME_TYPE_ALLOCATED_MALLOC;

   return newFrame;
   }

void
BytecodeHelpers::freeFrame(Frame *frame)
   {
   #define FREEFRAME_LINE LINETOSTR(__LINE__)
   if (frame->frameType & FRAME_TYPE_ALLOCATED_MALLOC)
      {
      free(frame);
      }
   }

void
BytecodeHelpers::setupArgs(Frame *newFrame, Frame *frame, int8_t argCount)
   {
#define SETUPARGS_LINE LINETOSTR(__LINE__)

   for (int8_t i = 0; i < argCount; i++)
      {
      frame->sp--;
      newFrame->args[argCount - 1 - i] = *frame->sp;
      }
   }

void
BytecodeHelpers::pushReturn(Frame *frame, int64_t retVal)
   {
#define PUSHRETURN_LINE LINETOSTR(__LINE__)
   *frame->sp = retVal;
   frame->sp++;
   }

Frame *
BytecodeHelpers::i2jTransition(Interpreter *interp, JitMethodFunction *func)
   {
#define I2JTRANSITION_LINE LINETOSTR(__LINE__)
   return transitionToJIT(interp, func);
   }

Frame *
BytecodeHelpers::j2jTransition(Interpreter *interp, JitMethodFunction *func)
   {
#define J2JTRANSITION_LINE LINETOSTR(__LINE__)
   return transitionToJIT(interp, func);
   }

Frame *
BytecodeHelpers::j2iTransition(Interpreter *interp)
   {
#define J2ITRANSITION_LINE LINETOSTR(__LINE__)
   int *x = 0;
   fprintf(stderr, "j2iTransition not handled\n");
   *x = 0;
   return NULL;
   }

Frame *
BytecodeHelpers::i2iTransition(Interpreter *interp, int8_t argCount)
   {
#define I2ITRANSITION_LINE LINETOSTR(__LINE__)
   return interp->currentFrame;
   }

void
BytecodeHelpers::compileMethod(Interpreter *interp, int8_t methodIndex)
   {
#define COMPILEMETHOD_LINE LINETOSTR(__LINE__)
   fprintf(stderr, "Compile of methodIndex %s requested\n", interp->methods[methodIndex].name);

   InterpreterTypeDictionary types;
   JitMethod jitMethod(&types, &interp->methods[methodIndex]);
   uint8_t *entry = 0;
   int32_t rc = compileMethodBuilder(&jitMethod, &entry);
   if (0 == rc)
      {
      fprintf(stderr, "\tCompile succeeded\n");
      interp->methods[methodIndex].compiledMethod = (JitMethodFunction *)entry;
      }
   else
      {
      fprintf(stderr, "\tCompile failed\n");
      }
   }

void
BytecodeHelpers::DefineFunctions(TR::MethodBuilder *mb)
   {
   InterpreterTypeDictionary *types = (InterpreterTypeDictionary*)mb->typeDictionary();

   TR::IlType *voidType = types->toIlType<void>();
   TR::IlType *pVoidType = types->PointerTo(voidType);
   TR::IlType *valueType = types->getTypes().value;
   TR::IlType *interpType = types->getTypes().interpreter;
   TR::IlType *frameType = types->getTypes().frame;
   TR::IlType *bytecodesType = types->getTypes().bytecodes;

   mb->DefineFunction((char *)"allocateFrame", (char *)__FILE__, (char *)ALLOCATEFRAME_LINE, (void *)&BytecodeHelpers::allocateFrame, frameType, 0);
   mb->DefineFunction((char *)"freeFrame", (char *)__FILE__, (char *)FREEFRAME_LINE, (void *)&BytecodeHelpers::freeFrame, voidType, 1, frameType);
   mb->DefineFunction((char *)"setupArgs", (char *)__FILE__, (char *)SETUPARGS_LINE, (void *)&BytecodeHelpers::setupArgs, voidType, 3, frameType, frameType, bytecodesType);
   mb->DefineFunction((char *)"pushReturn", (char *)__FILE__, (char *)PUSHRETURN_LINE, (void *)&BytecodeHelpers::pushReturn, voidType, 2, frameType, valueType);
   mb->DefineFunction((char *)"j2iTransition", (char *)__FILE__, (char *)J2ITRANSITION_LINE, (void *)&BytecodeHelpers::j2iTransition, frameType, 1, interpType);
   mb->DefineFunction((char *)"j2jTransition", (char *)__FILE__, (char *)J2JTRANSITION_LINE, (void *)&BytecodeHelpers::j2jTransition, frameType, 2, interpType, pVoidType);
   mb->DefineFunction((char *)"i2jTransition", (char *)__FILE__, (char *)I2JTRANSITION_LINE, (void *)&BytecodeHelpers::i2jTransition, frameType, 2, interpType, pVoidType);
   mb->DefineFunction((char *)"i2iTransition", (char *)__FILE__, (char *)I2ITRANSITION_LINE, (void *)&BytecodeHelpers::i2iTransition, frameType, 2, interpType, bytecodesType);
   mb->DefineFunction((char *)"compileMethod", (char *)__FILE__, (char *)COMPILEMETHOD_LINE, (void *)&BytecodeHelpers::compileMethod, voidType, 2, interpType, bytecodesType);
   }

int64_t
BytecodeHelpers::currentTime()
{
   struct timeval tp;

   gettimeofday(&tp, NULL);
   return ((int64_t)tp.tv_sec) * 1000 + tp.tv_usec / 1000;
}

void
BytecodeHelpers::printTime(int64_t)
{
   fprintf(stderr, "time taken")
}

Frame *
BytecodeHelpers::transitionToJIT(Interpreter *interp, JitMethodFunction *func)
   {
   (*func)(interp, interp->currentFrame);
   return interp->currentFrame;
   }

