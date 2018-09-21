/*******************************************************************************
 * Copyright (c) 2017, 2018 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

#include "ilgen/VirtualMachineInterpreterArray.hpp"

#include "compile/Compilation.hpp"
#include "il/SymbolReference.hpp"
#include "il/symbol/AutomaticSymbol.hpp"
#include "ilgen/BytecodeBuilder.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/VirtualMachineRegister.hpp"
#include "ilgen/VirtualMachineState.hpp"

OMR::VirtualMachineInterpreterArray::VirtualMachineInterpreterArray(TR::MethodBuilder *mb, TR::IlType *elementType, TR::VirtualMachineRegister *arrayBaseRegister)
   : TR::VirtualMachineArray(),
   _mb(mb),
   _elementType(elementType),
   _arrayBaseRegister(arrayBaseRegister)
   {
   }

// commits the simulated operand array of values to the virtual machine state
// the given builder object is where the operations to commit the state will be inserted
// into the array which is assumed to be managed independently, most likely
void
OMR::VirtualMachineInterpreterArray::Commit(TR::IlBuilder *b)
   {
   }

void
OMR::VirtualMachineInterpreterArray::Reload(TR::IlBuilder* b)
   {
   }

void
OMR::VirtualMachineInterpreterArray::MergeInto(TR::VirtualMachineState *o, TR::IlBuilder *b)
   {
   }

// Update the OperandArray_base after the Virtual Machine moves the array.
// This call will normally be followed by a call to Reload if any of the array values changed in the move
void
OMR::VirtualMachineInterpreterArray::UpdateArray(TR::IlBuilder *b, TR::IlValue *array)
   {
   _arrayBaseRegister->Store(b, array);
   }

// Allocate a new operand array and copy everything in this state
// If VirtualMachineOperandArray is subclassed, this function *must* also be implemented in the subclass!
TR::VirtualMachineState *
OMR::VirtualMachineInterpreterArray::MakeCopy()
   {
   return static_cast<TR::VirtualMachineState *>(this);
   }

TR::IlValue *
OMR::VirtualMachineInterpreterArray::Get(TR::IlBuilder *b, int32_t index)
   {
   return Get(b, b->ConstInt32(index));
   }

TR::IlValue *
OMR::VirtualMachineInterpreterArray::Get(TR::IlBuilder *b, TR::IlValue *index)
   {
   TR::IlType *pElementType = _mb->typeDictionary()->PointerTo(_elementType);
   TR::IlValue *arrayBase = _arrayBaseRegister->Load(b);

   TR::IlValue *arrayValue =
   b->LoadAt(pElementType,
   b->   IndexAt(pElementType,
            arrayBase,
            index));

   return arrayValue;
   }

void
OMR::VirtualMachineInterpreterArray::Set(TR::IlBuilder *b, int32_t index, TR::IlValue *value)
   {
   Set(b, b->ConstInt32(index), value);
   }

void
OMR::VirtualMachineInterpreterArray::Set(TR::IlBuilder *b, TR::IlValue *index, TR::IlValue *value)
   {
   TR::IlType *pElementType = _mb->typeDictionary()->PointerTo(_elementType);
   TR::IlValue *arrayBase = _arrayBaseRegister->Load(b);

   b->StoreAt(
   b->   IndexAt(pElementType,
            arrayBase,
            index),
         value);
   }

void
OMR::VirtualMachineInterpreterArray::Move(TR::IlBuilder *b, int32_t dstIndex, int32_t srcIndex)
   {
   TR::IlValue *dst = b->ConstInt32(dstIndex);
   TR::IlValue *src = b->ConstInt32(srcIndex);
   Move(b, dst, src);
   }

void
OMR::VirtualMachineInterpreterArray::Move(TR::IlBuilder *b, TR::IlValue *dstIndex, TR::IlValue *srcIndex)
   {
   TR::IlValue *value = Get(b, srcIndex);
   Set(b, dstIndex, value);
   }
