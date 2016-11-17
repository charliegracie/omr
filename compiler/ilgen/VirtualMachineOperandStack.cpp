/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2000, 2016
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 *******************************************************************************/

#include "ilgen/VirtualMachineOperandStack.hpp"
#include "ilgen/VirtualMachineRegister.hpp"
#include "compile/Compilation.hpp"
#include "il/SymbolReference.hpp"
#include "il/symbol/AutomaticSymbol.hpp"
#include "ilgen/IlBuilder.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"

namespace OMR
{

// Use this constructor most of the time (see below)
VirtualMachineOperandStack::VirtualMachineOperandStack(TR::MethodBuilder *mb, int32_t sizeHint, TR::IlType *elementType)
   : _mb(mb), _stackMax(sizeHint), _stackTop(-1), _elementType(elementType)
   {
   int32_t numBytes = _stackMax * sizeof(TR::IlValue *);
   _stack = (TR::IlValue **) TR::comp()->trMemory()->allocateHeapMemory(numBytes);
   memset(_stack, 0, numBytes);
   }

// This constructor should only be used at the start of the method builder; it provides the
// register used to hold the stack top address, and this constructor stores the current top
// of stack into a local variable called "OperandStack_base". This variable is used by
// Commit() as the location for the operand stack.
VirtualMachineOperandStack::VirtualMachineOperandStack(TR::MethodBuilder *mb, int32_t sizeHint, TR::IlType *elementType, OMR::VirtualMachineRegister *stackBaseReg)
   : VirtualMachineOperandStack(mb, sizeHint, elementType)
   {
   mb->Store("OperandStack_base", stackBaseReg->Load(mb));
   }

// commits the simulated operand stack of values to the virtual machine state
// the given builder object is the path to the interpreter
// the top of the stack is assumed to be managed independently, most likely
//    as a VirtualMachineRegister or a VirtualMachineRegisterInStruct
void
VirtualMachineOperandStack::Commit(TR::IlBuilder *b)
   {
   TR::IlType *Element = _elementType;
   TR::IlType *pElement = _mb->typeDictionary()->PointerTo(Element);

   TR::IlValue *stack = b->Load("OperandStack_base");
   for (int32_t i = _stackTop;i >= 0;i--)
      {
      // TBD: how to handle hitting the end of stack?

      b->StoreAt(
      b->   IndexAt(pElement,
               stack,
      b->      ConstInt32(i)),
            Pick(i)); // should generalize, delegate element storage ?
      }
   }

void
VirtualMachineOperandStack::MergeInto(OMR::VirtualMachineOperandStack *other, TR::IlBuilder *b)
   {
   TR_ASSERT(_stackTop == other->_stackTop, "stacks are not same size");
   for (int32_t i=_stackTop;i >= 0;i--)
      {
      // only need to do something if the two entries aren't already the same
      if (other->_stack[i]->getCPIndex() != _stack[i]->getCPIndex())
         {
         // what if types don't match? could use ConvertTo, but seems...arbitrary
         // nobody *should* design bytecode set where corresponding elements of stacks from
         // two incoming control flow edges can have different primitive types. objects, sure
         // but not primitive types! Expecting to be disappointed here some day...
         TR_ASSERT(_stack[i]->getSymbol()->getDataType() == other->_stack[i]->getSymbol()->getDataType(), "disappointing stack merge: primitive type mismatch at same depth stack elements");
         b->Store(other->_stack[i]->getSymbol()->getAutoSymbol()->getName(),
               _stack[i]);
         }
      }
   }

// Allocate a new operand stack and copy everything in this state
// If VirtualMachineOperandStack is subclassed, this function *must* also be implemented in the subclass!
VirtualMachineOperandStack *
VirtualMachineOperandStack::MakeCopy()
   {
   VirtualMachineOperandStack *copy = (VirtualMachineOperandStack *) TR::comp()->trMemory()->allocateHeapMemory(sizeof(VirtualMachineOperandStack));
   new (copy) VirtualMachineOperandStack(_mb, _stackMax, _elementType);

   copyTo(copy);

   return copy;
   }

void
VirtualMachineOperandStack::Push(TR::IlValue *value)
   {
   checkSize();
   _stack[++_stackTop] = value;
   }

TR::IlValue *
VirtualMachineOperandStack::Top()
   {
   TR_ASSERT(_stackTop >= 0, "no top: stack empty");
   return _stack[_stackTop];
   }

TR::IlValue *
VirtualMachineOperandStack::Pop()
   {
   TR_ASSERT(_stackTop >= 0, "stack underflow");
   return _stack[_stackTop--];
   }

void
VirtualMachineOperandStack::Drop(int32_t depth)
   {
   TR_ASSERT(_stackTop >= depth-1, "stack underflow");
   _stackTop-=depth;
   }

TR::IlValue *
VirtualMachineOperandStack::Pick(int32_t depth)
   {
   TR_ASSERT(_stackTop >= depth, "pick request exceeds stack depth");
   return _stack[_stackTop - depth];
   }

void
VirtualMachineOperandStack::Dup()
   {
   TR_ASSERT(_stackTop >= 0, "cannot dup: stack empty");
   TR::IlValue *top = _stack[_stackTop];
   checkSize();
   _stack[++_stackTop] = top;
   }

void
VirtualMachineOperandStack::copyTo(VirtualMachineOperandStack *copy)
   {
   int32_t numBytes = _stackMax * sizeof(TR::IlValue *);
   memcpy(copy->_stack, _stack, numBytes);
   copy->_stackTop = _stackTop;
   copy->_stackMax = _stackMax;
   }

void
VirtualMachineOperandStack::checkSize()
   {
   if (_stackTop == _stackMax)
      grow();
   }

void
VirtualMachineOperandStack::grow()
   {
   int32_t numBytes = _stackMax * sizeof(TR::IlValue *);

   int32_t newMax = _stackMax + (_stackMax >> 1);
   int32_t newBytes = newMax * sizeof(TR::IlValue *);
   TR::IlValue ** newStack = (TR::IlValue **) TR::comp()->trMemory()->allocateHeapMemory(newBytes);

   memset(newStack, 0, newBytes);
   memcpy(newStack, _stack, numBytes);

   _stack = newStack;
   _stackMax = newMax;
   }

} // namespace OMR
