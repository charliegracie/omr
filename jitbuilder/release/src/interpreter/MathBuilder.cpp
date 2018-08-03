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
#include "MathBuilder.hpp"

MathBuilder::MathBuilder(TR::MethodBuilder *methodBuilder, int32_t bcIndex, MathFuncType mathFunction)
   : OpcodeBuilder(methodBuilder, bcIndex, "DUP"),
   _mathFunction(mathFunction)
   {
   }

MathBuilder *
MathBuilder::OrphanOpcodeBuilder(TR::MethodBuilder *methodBuilder, int32_t bcIndex, MathFuncType mathFunction)
   {
   MathBuilder *orphan = new MathBuilder(methodBuilder, bcIndex, mathFunction);
   methodBuilder->InitializeOpcodeBuilder(orphan);
   return orphan;
   }

void
MathBuilder::execute()
   {
   TR::VirtualMachineInterpreterStack *state = (TR::VirtualMachineInterpreterStack*)vmState();

   TR::IlValue *right = state->Pop(this);
   TR::IlValue *left = state->Pop(this);

   TR::IlValue *value = (*_mathFunction)(this, left, right);

   state->Push(this, value);
   }

TR::IlValue *
MathBuilder::add(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right)
   {
   return builder->Add(left, right);
   }

TR::IlValue *
MathBuilder::sub(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right)
   {
   return builder->Sub(left, right);
   }

TR::IlValue *
MathBuilder::mul(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right)
   {
   return builder->Mul(left, right);
   }

TR::IlValue *
MathBuilder::div(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right)
   {
   return builder->Div(left, right);
   }

