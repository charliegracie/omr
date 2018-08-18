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
#include "PopLocalBuilder.hpp"

PopLocalBuilder::PopLocalBuilder(TR::MethodBuilder *methodBuilder, int32_t bcIndex)
   : BytecodeBuilder(methodBuilder, bcIndex, "POP_LOCAL")
   {
   }

PopLocalBuilder *
PopLocalBuilder::OrphanBytecodeBuilder(TR::MethodBuilder *methodBuilder, int32_t bcIndex)
   {
   PopLocalBuilder *orphan = new PopLocalBuilder(methodBuilder, bcIndex);
   methodBuilder->InitializeBytecodeBuilder(orphan);
   return orphan;
   }

void
PopLocalBuilder::execute()
   {
   TR::VirtualMachineStack *state = ((InterpreterVMState*)vmState())->_stack;
   TR::IlType *pInt8 = _types->PointerTo(Int8);
   TR::IlType *pStackType = _types->PointerTo(STACKVALUEILTYPE);

   TR::IlValue *poppedValue = state->Pop(this);
   TR::IlValue *frame = Load("frame");

   TR::IlValue *localIndex =
   LoadAt(pInt8,
      IndexAt(pInt8,
         Load("bytecodes"),
         Add(
            Load("pc"),
            ConstInt32(1))));

   TR::IlValue *locals = LoadIndirect("Frame", "locals", frame);

   StoreAt(
      IndexAt(pStackType,
         locals,
         localIndex),
      poppedValue);
   }

