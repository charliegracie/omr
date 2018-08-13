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
#include "RetBuilder.hpp"

static Frame * retHelper(Frame *frame)
   {
   #define RETHELPER_LINE LINETOSTR(__LINE__)

   Frame *previous = frame->previous;

   free(frame);

   return previous;
   }

RetBuilder::RetBuilder(TR::MethodBuilder *methodBuilder, int32_t bcIndex)
   : BytecodeBuilder(methodBuilder, bcIndex, "RET")
   {
   }

RetBuilder *
RetBuilder::OrphanBytecodeBuilder(TR::MethodBuilder *methodBuilder, int32_t bcIndex, TR::IlType *frameType)
   {
   RetBuilder *orphan = new RetBuilder(methodBuilder, bcIndex);
   methodBuilder->InitializeBytecodeBuilder(orphan);

   methodBuilder->DefineFunction((char *)"retHelper", (char *)__FILE__, (char *)RETHELPER_LINE, (void *)&retHelper, frameType, 1, frameType);

   return orphan;
   }

void
RetBuilder::execute()
   {
   TR::VirtualMachineInterpreterStack *state = (TR::VirtualMachineInterpreterStack*)vmState();

   TR::IlValue *retVal = state->Pop(this);
   state->Commit(this);

   TR::IlValue *newFrame = Call("retHelper", 1, Load("frame"));
   Store("frame", newFrame);

   TR::IlValue *bytecodesAddress = StructFieldInstanceAddress("Frame", "bytecodes", newFrame);
   TR::IlValue *bytecodes = LoadAt(_types->PointerTo(_types->PointerTo(Int8)), bytecodesAddress);
   Store("bytecodes", bytecodes);

   TR::IlValue *pcAddress = StructFieldInstanceAddress("Frame", "savedPC", newFrame);
   TR::IlValue *pc = LoadAt(_types->PointerTo(Int32), pcAddress);
   Store("pc", pc);

   state->Reload(this);

   state->Push(this, retVal);
   }

