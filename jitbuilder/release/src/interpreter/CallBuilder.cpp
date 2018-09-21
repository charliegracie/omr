/*******************************************************************************
 * Copyright (c) 2018, 2018 IBM Corp. and others
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

#include "Jit.hpp"
#include "ilgen/RuntimeBuilder.hpp"
#include "ilgen/VirtualMachineStack.hpp"

#include "InterpreterTypes.h"
#include "InterpreterTypeDictionary.hpp"
#include "CallBuilder.hpp"
#include "JitMethod.hpp"

CallBuilder::CallBuilder(TR::MethodBuilder *methodBuilder, int32_t bcIndex, char *name)
   : BytecodeBuilder(methodBuilder, bcIndex, name, 3),
   _runtimeBuilder((TR::RuntimeBuilder *)methodBuilder),
   _frameType(((InterpreterTypeDictionary*)methodBuilder->typeDictionary())->getTypes().pFrame)
   {
   }

void
CallBuilder::execute()
   {
   InterpreterVMState *state = (InterpreterVMState*)vmState();
   TR::IlValue *methodIndex = _runtimeBuilder->GetImmediate(this);
   TR::IlValue *argCount = _runtimeBuilder->GetImmediate(this);
   TR::IlValue *interp = Load("interp");

   //Get interp->methods[methodIndex]
   TR::IlValue *methodToCall = getMethodAtIndex(this, interp, methodIndex);

   state->Commit(this);
   _runtimeBuilder->Commit(this);

   //Get interp->currentFrame
   TR::IlValue *currentFrameAddress = StructFieldInstanceAddress("Interpreter", "currentFrame", interp);
   TR::IlValue *currentFrame = LoadAt(_types->PointerTo(_frameType), currentFrameAddress);

   TR::IlValue *currentFrameFrameTypeAddress = StructFieldInstanceAddress("Frame", "frameType", currentFrame);
   TR::IlValue *currentFrameFrameType = LoadAt(_types->PointerTo(Int32), currentFrameFrameTypeAddress);

   TR::IlBuilder *isInterpreter = NULL;
   TR::IlBuilder *isJit = NULL;
   IfThenElse(&isInterpreter, &isJit,
      And(currentFrameFrameType,
         ConstInt32(FRAME_TYPE_INTERPRETER)));

   isInterpreter->Store("newFrame",
   isInterpreter->   Call("allocateFrame", 0));

   isJit->Store("newFrame",
   isJit->   CreateLocalArray(sizeof(Frame), Int8));
   isJit->Call("initializeFrame", 1,
   isJit->   Load("newFrame"));

   //Allocate newFrame
   TR::IlValue *newFrame = Load("newFrame");

   //Initialize new frame
   //Set newFrame->previous = currentFrame
   TR::IlValue *previousAddress = StructFieldInstanceAddress("Frame", "previous", newFrame);
   StoreAt(previousAddress, currentFrame);

   // fetch methodToCall->bytecodes
   TR::IlValue *methodBytecodeAddress = StructFieldInstanceAddress("Method", "bytecodes", methodToCall);
   TR::IlValue *methodBytecodes = LoadAt(_types->PointerTo(_types->PointerTo(Int8)), methodBytecodeAddress);

   //Pop the argCount values from currentFrame->sp into newFrame->sp
   Call("setupArgs", 3, newFrame, currentFrame, argCount);

   //Set frame->bytecodes = methodBytecodes
   TR::IlValue *bytecodesAddress = StructFieldInstanceAddress("Frame", "bytecodes", newFrame);
   StoreAt(bytecodesAddress, methodBytecodes);
   Store("bytecodes", methodBytecodes);
   //end of frame initialization

   //Set interp->currentFrame = newFrame;
   StoreAt(currentFrameAddress, newFrame);

   //Check to see if it is time to compile the method being called
   TR::IlValue *callsUntilJitAddress = StructFieldInstanceAddress("Method", "callsUntilJit", methodToCall);
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
   TR::IlValue *methodCompiledMethodAddress = StructFieldInstanceAddress("Method", "compiledMethod", methodToCall);
   TR::IlValue *compiledMethod = LoadAt(_types->PointerTo(Address), methodCompiledMethodAddress);

//   TR::IlValue *currentFrameFrameTypeAddress = StructFieldInstanceAddress("Frame", "frameType", currentFrame);
//   TR::IlValue *currentFrameFrameType = LoadAt(_types->PointerTo(Int32), currentFrameFrameTypeAddress);

   TR::IlValue *newFrameFrameTypeAddress = StructFieldInstanceAddress("Frame", "frameType", newFrame);
   TR::IlValue *newFrameFrameType = LoadAt(_types->PointerTo(Int32), newFrameFrameTypeAddress);

   TR::IlBuilder *methodCompiled = NULL;
   TR::IlBuilder *methodNotCompiled = NULL;
   IfThenElse(&methodNotCompiled, &methodCompiled,
      EqualTo(compiledMethod, NullAddress()));

   //method being called is not JIT'd so dispatch to the interpreter
   //Set newFrame->frameType |= FRAME_TYPE_JIT
   TR::IlValue *frameType = methodCompiled->ConstInt32(FRAME_TYPE_JIT);
   TR::IlValue *newFrameType = methodCompiled->Or(newFrameFrameType, frameType);
   methodCompiled->StoreAt(newFrameFrameTypeAddress, newFrameType);

   TR::IlBuilder *i2j = NULL;
   TR::IlBuilder *j2j = NULL;
   methodCompiled->IfThenElse(&i2j, &j2j,
   methodCompiled->   And(currentFrameFrameType,
   methodCompiled->      ConstInt32(FRAME_TYPE_INTERPRETER)));

   //interpreter frame is calling JIT'd frame. Call i2j transition and store the result frame
   i2j->Store("frame",
   i2j->   Call("i2jTransition", 2, interp, compiledMethod));

   //JIT frame is calling JIT'd frame. Call j2j transition and store the result frame
   j2j->Store("frame",
   j2j->   Call("j2jTransition", 2, interp, compiledMethod));

   //method being called is not JIT'd so dispatch to the interpreter
   //Set newFrame->frameType |= FRAME_TYPE_INTERPRETER
   frameType = methodNotCompiled->ConstInt32(FRAME_TYPE_INTERPRETER);
   newFrameType = methodNotCompiled->Or(newFrameFrameType, frameType);
   methodNotCompiled->StoreAt(newFrameFrameTypeAddress, newFrameType);

   TR::IlBuilder *i2i = NULL;
   TR::IlBuilder *j2i = NULL;
   methodNotCompiled->IfThenElse(&i2i, &j2i,
   methodNotCompiled->   And(currentFrameFrameType,
   methodNotCompiled->      ConstInt32(FRAME_TYPE_INTERPRETER)));

   //interpreter frame is calling interpreter frame. No transition required so just store frame = newFrame
   i2i->Store("frame",
   i2i->   Call("i2iTransition", 2, interp, argCount));

   //JIT'd frame is calling interpreter. Call j2iTransition and store the result frame
   j2i->Store("frame",
   j2i->   Call("j2iTransition", 1, interp));

   _runtimeBuilder->Reload(this);
   state->Reload(this);

   _runtimeBuilder->DefaultFallthroughTarget(this);
   }

TR::IlValue *
CallBuilder::getMethodAtIndex(TR::IlBuilder *builder, TR::IlValue *interp, TR::IlValue *methodIndex)
   {
   TR::IlType *methodType = _types->LookupStruct("Method");
   TR::IlType *pMethodType = _types->PointerTo(methodType);

   TR::IlValue *methodsAddresses = builder->StructFieldInstanceAddress("Interpreter", "methods", interp);
   TR::IlValue *methodsAddress = builder->LoadAt(pMethodType, methodsAddresses);
   TR::IlValue *method = builder->IndexAt(pMethodType, methodsAddress, methodIndex);

   return method;
   }

