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
#include "PushConstantBuilder.hpp"

PushConstantBuilder::PushConstantBuilder(TR::RuntimeBuilder *runtimeBuilder, int32_t bcIndex)
   : BytecodeBuilder(runtimeBuilder, bcIndex, "PUSH_CONSTANT"),
   _runtimeBuilder(runtimeBuilder)
   {
   }

PushConstantBuilder *
PushConstantBuilder::OrphanBytecodeBuilder(TR::RuntimeBuilder *runtimeBuilder, int32_t bcIndex)
   {
   PushConstantBuilder *orphan = new PushConstantBuilder(runtimeBuilder, bcIndex);
   runtimeBuilder->InitializeBytecodeBuilder(orphan);
   return orphan;
   }

void
PushConstantBuilder::execute()
   {
   TR::VirtualMachineStack *state = ((InterpreterVMState*)vmState())->_stack;
   TR::IlValue *value = _runtimeBuilder->GetImmediate(this, 1);
   value = ConvertTo(STACKVALUEILTYPE, value);

   state->Push(this, value);
   }

