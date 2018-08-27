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

RetBuilder::RetBuilder(TR::RuntimeBuilder *runtimeBuilder, int32_t bcIndex, TR::IlType *frameType)
   : BytecodeBuilder(runtimeBuilder, bcIndex, "RET", 2),
   _runtimeBuilder(runtimeBuilder),
   _frameType(frameType)
   {
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
   // no reason to commit the state since this frame is getting freed

   TR::IlValue *currentFrameAddress = StructFieldInstanceAddress("Interpreter", "currentFrame", Load("interp"));
   TR::IlValue *currentFrame = LoadAt(_types->PointerTo(_frameType), currentFrameAddress);

   TR::IlValue *previousAddress = StructFieldInstanceAddress("Frame", "previous", currentFrame);
   TR::IlValue *previous = LoadAt(_types->PointerTo(_frameType), previousAddress);

   Call("freeFrame", 1, currentFrame);

   StoreAt(currentFrameAddress, previous);
   Store("frame", previous);

   // The state/stack variable can not be used here since a Reload()/Push()/Commit() would not work on the VirtualMachineOperandStack
   Call("pushReturn", 2, Load("frame"), retVal);

   _runtimeBuilder->ReturnTarget(this);
   }

