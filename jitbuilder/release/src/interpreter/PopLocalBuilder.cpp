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
#include "PopLocalBuilder.hpp"

PopLocalBuilder::PopLocalBuilder(TR::RuntimeBuilder *runtimeBuilder, int32_t bcIndex)
   : BytecodeBuilder(runtimeBuilder, bcIndex, "POP_LOCAL", 2),
   _runtimeBuilder(runtimeBuilder)
   {
   }

PopLocalBuilder *
PopLocalBuilder::OrphanBytecodeBuilder(TR::RuntimeBuilder *runtimeBuilder, int32_t bcIndex)
   {
   PopLocalBuilder *orphan = new PopLocalBuilder(runtimeBuilder, bcIndex);
   runtimeBuilder->InitializeBytecodeBuilder(orphan);
   return orphan;
   }

void
PopLocalBuilder::execute()
   {
   TR::VirtualMachineStack *stack = ((InterpreterVMState*)vmState())->_stack;
   TR::VirtualMachineArray *locals = ((InterpreterVMState*)vmState())->_locals;

   TR::IlValue *localIndex = _runtimeBuilder->GetImmediate(this);
   TR::IlValue *poppedValue = stack->Pop(this);

   locals->Set(this, localIndex, poppedValue);

   _runtimeBuilder->DefaultFallthroughTarget(this);
   }

