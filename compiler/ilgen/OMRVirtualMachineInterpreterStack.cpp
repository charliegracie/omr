/*******************************************************************************
 * Copyright (c) 2016, 2016 IBM Corp. and others
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

#include "ilgen/VirtualMachineInterpreterStack.hpp"
#include "ilgen/VirtualMachineRegister.hpp"
#include "compile/Compilation.hpp"
#include "il/SymbolReference.hpp"
#include "il/symbol/AutomaticSymbol.hpp"
#include "ilgen/IlBuilder.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"

OMR::VirtualMachineInterpreterStack::VirtualMachineInterpreterStack(TR::MethodBuilder *mb, TR::VirtualMachineRegister *stackTopRegister, TR::IlType *elementType)
   : TR::VirtualMachineStack(),
   _mb(mb),
   _stackTopRegister(stackTopRegister),
   _elementType(elementType),
   _stackBaseName(NULL)
   {
   init();
   }

// commits the simulated operand stack of values to the virtual machine state
// the given builder object is where the operations to commit the state will be inserted
// the top of the stack is assumed to be managed independently, most likely
//    as a VirtualMachineRegister or a VirtualMachineRegisterInStruct
void
OMR::VirtualMachineInterpreterStack::Commit(TR::IlBuilder *b)
   {
   }

void
OMR::VirtualMachineInterpreterStack::Reload(TR::IlBuilder* b)
   {
   }

void
OMR::VirtualMachineInterpreterStack::MergeInto(TR::VirtualMachineInterpreterStack* other, TR::IlBuilder* b)
   {
   }

// Update the OperandStack_base and _stackTopRegister after the Virtual Machine moves the stack.
// This call will normally be followed by a call to Reload if any of the stack values changed in the move
void
OMR::VirtualMachineInterpreterStack::UpdateStack(TR::IlBuilder *b, TR::IlValue *stack)
   {
   // TODO implement?  Is this even useful for anything?
   }

// Allocate a new operand stack and copy everything in this state
// If VirtualMachineOperandStack is subclassed, this function *must* also be implemented in the subclass!
TR::VirtualMachineState *
OMR::VirtualMachineInterpreterStack::MakeCopy()
   {
   return static_cast<TR::VirtualMachineState *>(this);
   } 

void
OMR::VirtualMachineInterpreterStack::Push(TR::IlBuilder *builder, TR::IlValue *value)
   {
   TR::IlValue *stackAddress = _stackTopRegister->Load(builder);
   _stackTopRegister->Adjust(builder, 1);

   if (_preIncrement)
      {
      stackAddress = _stackTopRegister->Load(builder);
      }

   builder->StoreAt(
               stackAddress,
   builder->   ConvertTo(_elementType, value));

   }

TR::IlValue *
OMR::VirtualMachineInterpreterStack::Top(TR::IlBuilder *builder)
   {
   TR::IlValue *stackAddress = _stackTopRegister->Load(builder);
   TR::IlType *pElementType = _mb->typeDictionary()->PointerTo(_elementType);

   int32_t offset = 0;
   if (!_preIncrement)
      {
      offset = -1;
      }

   TR::IlValue *value =
   builder->LoadAt(pElementType,
   builder->   IndexAt(pElementType,
                  stackAddress,
   builder->      ConstInt32(offset)));
   return value;
   }

TR::IlValue *
OMR::VirtualMachineInterpreterStack::Pop(TR::IlBuilder *builder)
   {
   TR::IlValue *stackAddress = _stackTopRegister->Load(builder);
   _stackTopRegister->Adjust(builder, -1);

   if (!_preIncrement)
      {
      stackAddress = _stackTopRegister->Load(builder);
      }

   TR::IlType *pElementType = _mb->typeDictionary()->PointerTo(_elementType);

   return builder->LoadAt(pElementType, stackAddress);
   }

TR::IlValue *
OMR::VirtualMachineInterpreterStack::Pick(int32_t depth)
   {
   return NULL;
   }

void
OMR::VirtualMachineInterpreterStack::Drop(TR::IlBuilder *b, int32_t depth)
   {

   }

void
OMR::VirtualMachineInterpreterStack::Dup(TR::IlBuilder *builder)
   {
   Push(builder, Top(builder));
   }

void
OMR::VirtualMachineInterpreterStack::init()
   {
   TR::Compilation *comp = TR::comp();
   // Create a temp for the OperandStack base
   TR::SymbolReference *symRef = comp->getSymRefTab()->createTemporary(_mb->methodSymbol(), _mb->typeDictionary()->PointerTo(_elementType)->getPrimitiveType());
   symRef->getSymbol()->setNotCollected();
   char *name = (char *) comp->trMemory()->allocateHeapMemory((11+10+1) * sizeof(char)); // 11 ("_StackBase_") + max 10 digits + trailing zero
   sprintf(name, "_StackBase_%u", symRef->getCPIndex());
   symRef->getSymbol()->getAutoSymbol()->setName(name);
   _mb->defineSymbol(name, symRef);

   _stackBaseName = symRef->getSymbol()->getAutoSymbol()->getName();

   _preIncrement = false;

   // store current operand stack pointer base address so we can use it whenever we need
   // to recreate the stack as the interpreter would have
   _mb->Store(_stackBaseName, _stackTopRegister->Load(_mb));
   }
