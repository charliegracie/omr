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

#include "Jit.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/VirtualMachineInterpreterStack.hpp"

#include "InterpreterTypes.h"
#include "InterpreterTypeDictionary.hpp"
#include "CallBuilder.hpp"
#include "JitMethod.hpp"

static Frame *allocateFrame()
   {
#define ALLOCATEFRAME_LINE LINETOSTR(__LINE__)

   Frame *newFrame = (Frame *)malloc(sizeof(Frame));
   if (NULL == newFrame)
      {
      int *x = 0;
      fprintf(stderr, "Unable to allocate frame for call\n");
      *x = 0;
      }

   newFrame->locals = newFrame->loc;
   newFrame->sp = newFrame->stack;
#if FRAME_ZERO
   memset(newFrame, 0, sizeof(Frame));
#endif

   return newFrame;
   }

static void setupArgs(Frame *newFrame, Frame *frame, int8_t argCount)
   {
#define SETUPARGS_LINE LINETOSTR(__LINE__)

   for (int8_t i = 0; i < argCount; i++)
      {
      frame->sp--;
      newFrame->stack[argCount - 1 - i] = *frame->sp;
      }
   }

static Frame *transitionToJIT(Interpreter *interp, JitMethodFunction *func)
   {
   (*func)(interp, interp->currentFrame);
   return interp->currentFrame;
   }

static Frame *i2jTransition(Interpreter *interp, JitMethodFunction *func)
   {
#define I2JTRANSITION_LINE LINETOSTR(__LINE__)
   return transitionToJIT(interp, func);
   }

static Frame *j2jTransition(Interpreter *interp, JitMethodFunction *func)
   {
#define J2JTRANSITION_LINE LINETOSTR(__LINE__)
   return transitionToJIT(interp, func);
   }

static Frame *j2iTransition(Interpreter *interp)
   {
#define J2ITRANSITION_LINE LINETOSTR(__LINE__)
   int *x = 0;
   fprintf(stderr, "j2iTransition not handled\n");
   *x = 0;
   return NULL;
   }

static Frame *i2iTransition(Interpreter *interp, int8_t argCount)
   {
#define I2ITRANSITION_LINE LINETOSTR(__LINE__)
   interp->currentFrame->sp+=argCount;
   return interp->currentFrame;
   }

static void compileMethod(Interpreter *interp, int8_t methodIndex)
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

static void debug(Interpreter *interp, Frame *frame, int8_t *bytecodes, int8_t methodIndex)
   {
#define DEBUG_LINE LINETOSTR(__LINE__)
   fprintf(stderr, "debug interp %p frame %p bytecodesFromSender %p methodIndex %d bytecodesInFrame %p previousPC %d frame->stack[0] %lld method->name %s\n", interp, frame, bytecodes, methodIndex, frame->bytecodes, frame->previous->savedPC, frame->stack[0], interp->methods[methodIndex].name);
   }

CallBuilder::CallBuilder(TR::MethodBuilder *methodBuilder, int32_t bcIndex, TR::IlType *frameType)
   : BytecodeBuilder(methodBuilder, bcIndex, "CALL"),
   _frameType(frameType)
   {
   }

void
CallBuilder::DefineFunctions(TR::MethodBuilder *methodBuilder, TR::IlType *interpType, TR::IlType *frameType)
   {
   TR::IlType *voidType = methodBuilder->typeDictionary()->toIlType<void>();
   TR::IlType *pVoidType = methodBuilder->typeDictionary()->PointerTo(voidType);
   TR::IlType *Int8 = methodBuilder->typeDictionary()->PrimitiveType(TR::Int8);
   TR::IlType *pInt8 = methodBuilder->typeDictionary()->PointerTo(Int8);

   methodBuilder->DefineFunction((char *)"allocateFrame", (char *)__FILE__, (char *)ALLOCATEFRAME_LINE, (void *)&allocateFrame, frameType, 0);
   methodBuilder->DefineFunction((char *)"setupArgs", (char *)__FILE__, (char *)SETUPARGS_LINE, (void *)&setupArgs, voidType, 3, frameType, frameType, Int8);
   methodBuilder->DefineFunction((char *)"j2iTransition", (char *)__FILE__, (char *)J2ITRANSITION_LINE, (void *)&j2iTransition, frameType, 1, interpType);
   methodBuilder->DefineFunction((char *)"j2jTransition", (char *)__FILE__, (char *)J2JTRANSITION_LINE, (void *)&j2jTransition, frameType, 2, interpType, pVoidType);
   methodBuilder->DefineFunction((char *)"i2jTransition", (char *)__FILE__, (char *)I2JTRANSITION_LINE, (void *)&i2jTransition, frameType, 2, interpType, pVoidType);
   methodBuilder->DefineFunction((char *)"i2iTransition", (char *)__FILE__, (char *)I2ITRANSITION_LINE, (void *)&i2iTransition, frameType, 2, interpType, Int8);
   methodBuilder->DefineFunction((char *)"compileMethod", (char *)__FILE__, (char *)COMPILEMETHOD_LINE, (void *)&compileMethod, voidType, 2, interpType, Int8);

   methodBuilder->DefineFunction((char *)"debug", (char *)__FILE__, (char *)DEBUG_LINE, (void *)&debug, voidType, 4, interpType, frameType, pInt8, Int8);
   }

CallBuilder *
CallBuilder::OrphanBytecodeBuilder(TR::MethodBuilder *methodBuilder, int32_t bcIndex, TR::IlType *interpType, TR::IlType *frameType)
   {
   CallBuilder *orphan = new CallBuilder(methodBuilder, bcIndex, frameType);
   methodBuilder->InitializeBytecodeBuilder(orphan);
   return orphan;
   }

void
CallBuilder::execute()
   {
   InterpreterVMState *state = (InterpreterVMState*)vmState();
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

   TR::IlValue *interp = Load("interp");

   //Get interp->currentFrame
   TR::IlValue *currentFrameAddress = StructFieldInstanceAddress("Interpreter", "currentFrame", interp);
   TR::IlValue *currentFrame = LoadAt(_types->PointerTo(_frameType), currentFrameAddress);

   //Set currentFrame->savedPC = pc
   TR::IlValue *pcAddress = StructFieldInstanceAddress("Frame", "savedPC", currentFrame);
   StoreAt(pcAddress, pc);

   //Allocate newFrame
   TR::IlValue *newFrame = Call("allocateFrame", 0);

   //Set newFrame->previous = currentFrame
   TR::IlValue *previousAddress = StructFieldInstanceAddress("Frame", "previous", newFrame);
   StoreAt(previousAddress, currentFrame);

   //Get interp->methods[methodIndex]
   TR::IlType *methodType = _types->LookupStruct("Method");
   TR::IlType *pMethodType = _types->PointerTo(methodType);
   TR::IlValue *methodsAddresses = StructFieldInstanceAddress("Interpreter", "methods", interp);
   TR::IlValue *methodsAddress = LoadAt(pMethodType, methodsAddresses);
   TR::IlValue *methodAddressAtIndex = IndexAt(pMethodType, methodsAddress, methodIndex);
   TR::IlValue *methodBytecodeAddress = StructFieldInstanceAddress("Method", "bytecodes", methodAddressAtIndex);
   TR::IlValue *methodBytecodes = LoadAt(_types->PointerTo(_types->PointerTo(Int8)), methodBytecodeAddress);

   //Pop the argCount values from currentFrame->sp into newFrame->sp
   Call("setupArgs", 3, newFrame, currentFrame, argCount);

   //TR::IlValue *bogusBytecodes = Load("bytecodes");

   //Set frame->bytecodes = methodBytecodes
   TR::IlValue *bytecodesAddress = StructFieldInstanceAddress("Frame", "bytecodes", newFrame);
   StoreAt(bytecodesAddress, methodBytecodes);
   Store("bytecodes", methodBytecodes);

   //Set frame->savedPC = 0
   pcAddress = StructFieldInstanceAddress("Frame", "savedPC", newFrame);
   StoreAt(pcAddress, ConstInt32(0));

   //Set interp->currentFrame = newFrame;
   StoreAt(currentFrameAddress, newFrame);

   //Check to see if it is time to compile the method being called
   TR::IlValue *callsUntilJitAddress = StructFieldInstanceAddress("Method", "callsUntilJit", methodAddressAtIndex);
   TR::IlValue *callsUntilJit = LoadAt(_types->PointerTo(Int32), callsUntilJitAddress);

   TR::IlBuilder *decrementCounter = NULL;
   IfThen(&decrementCounter,
      GreaterThan(callsUntilJit,
         ConstInt32(0)));

   TR::IlValue *newCallsUntilJit =
   decrementCounter->Sub(callsUntilJit,
   decrementCounter->   ConstInt32(1));

   decrementCounter->StoreAt(callsUntilJitAddress, newCallsUntilJit);
   TR::IlBuilder *compile = NULL;
   decrementCounter->IfThen(&compile,
   decrementCounter->   EqualTo(newCallsUntilJit,
   decrementCounter->      ConstInt32(0)));

   compile->Call("compileMethod", 2, interp, methodIndex);

   //Check if there is a compiled version of the method be called
   TR::IlValue *methodCompiledMethodAddress = StructFieldInstanceAddress("Method", "compiledMethod", methodAddressAtIndex);
   TR::IlValue *compiledMethod = LoadAt(_types->PointerTo(Address), methodCompiledMethodAddress);

   TR::IlValue *currentFrameFrameTypeAddress = StructFieldInstanceAddress("Frame", "frameType", currentFrame);
   TR::IlValue *currentFrameFrameType = LoadAt(_types->PointerTo(Int32), currentFrameFrameTypeAddress);

   TR::IlValue *newFrameFrameTypeAddress = StructFieldInstanceAddress("Frame", "frameType", newFrame);

   //Call("debug", 4, interp, newFrame, bogusBytecodes, methodIndex);

   TR::IlBuilder *methodCompiled = NULL;
   TR::IlBuilder *methodNotCompiled = NULL;
   IfThenElse(&methodNotCompiled, &methodCompiled,
      EqualTo(compiledMethod, NullAddress()));

   //method being called is not JIT'd so dispatch to the interpreter
   //Set newFrame->frameType = INTERPRETER
   methodCompiled->StoreAt(newFrameFrameTypeAddress,
   methodCompiled->   ConstInt32(JIT));

   TR::IlBuilder *i2j = NULL;
   TR::IlBuilder *j2j = NULL;
   methodCompiled->IfThenElse(&i2j, &j2j,
   methodCompiled->   EqualTo(currentFrameFrameType,
   methodCompiled->      ConstInt32(INTERPRETER)));

   //interpreter frame is calling JIT'd frame. Call i2j transition and store the result frame
   i2j->Store("frame",
   i2j->   Call("i2jTransition", 2, interp, compiledMethod));

   //JIT frame is calling JIT'd frame. Call j2j transition and store the result frame
   j2j->Store("frame",
   j2j->   Call("j2jTransition", 2, interp, compiledMethod));

   //method being called is not JIT'd so dispatch to the interpreter
   //Set newFrame->frameType = INTERPRETER
   methodNotCompiled->StoreAt(newFrameFrameTypeAddress,
   methodNotCompiled->   ConstInt32(INTERPRETER));

   TR::IlBuilder *i2i = NULL;
   TR::IlBuilder *j2i = NULL;
   methodNotCompiled->IfThenElse(&i2i, &j2i,
   methodNotCompiled->   EqualTo(currentFrameFrameType,
   methodNotCompiled->      ConstInt32(INTERPRETER)));

   //interpreter frame is calling interpreter frame. No transition required so just store frame = newFrame
   i2i->Store("frame",
   i2i->   Call("i2iTransition", 2, interp, argCount));

   //JIT'd frame is calling interpreter. Call j2iTransition and store the result frame
   j2i->Store("frame",
   j2i->   Call("j2iTransition", 1, interp));

   //Set pc to the frame->savedPC
   pcAddress = StructFieldInstanceAddress("Frame", "savedPC", Load("frame"));
   Store("pc",
      LoadAt(_types->PointerTo(Int32), pcAddress));

   //Set frame->bytecodes = methodBytecodes
   bytecodesAddress = StructFieldInstanceAddress("Frame", "bytecodes", Load("frame"));
   TR::IlValue *postTransitionFrameBytecodes = LoadAt(_types->PointerTo(_types->PointerTo(Int8)), bytecodesAddress);
   Store("bytecodes", postTransitionFrameBytecodes);

   state->Reload(this);
   }

