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

#ifndef OMR_VIRTUALMACHINEOPERANDSTACK_INCL
#define OMR_VIRTUALMACHINEOPERANDSTACK_INCL

#include <stdint.h>
#include "ilgen/VirtualMachineRegister.hpp"
#include "ilgen/IlBuilder.hpp"

namespace TR { class MethodBuilder; }

namespace OMR
{

// VirtualMachineOperandStack simulates the operand stack used by many bytecode
// based virtual machines. In such virtual machines, the operand stack holds the
// intermediate expression values computed by the bytecodes. Ther compiler
// simulates this operand stack as well, but what is pushed to and popped from
// the simulated operand stack are expression nodes that represent the value
// computed by the bytecodes. As each bytecode pops expression nodes off the
// operand stack, it combines them to form more complicated expressions which
// are then pushed back onto the operand stack for consumption by other bytecodes.
// The stack is represented as an array of pointers to TR::IlValue's, making it
// easy to use IlBuilder services to consume and compute new values. Note that,
// unlike VirtualMachineRegister, the simulated operand stack is *not* maintained
// by the method code as part of the method's stack frame. This approach requires
// modelling the state of the operand stack at all program points, which means
// there cannot only be one VirtualMachineOperandStack object.
// Language compilers will typically need to create *two* VirtualMachineOperandStack
// objects for *each* BytecodeBuilder object: one object to represent the state of
// the operand stack at the beginning of the bytecode builder (which will be used
// to implement control flow merges correctly), and another to track how the operand
// stack's state changes as the code of the bytecode executes, and will be used to
// propagate the state of the operand stack as control flows from one bytecode
// to others.
// The current implementation does not share anything among different
// VirtualMachineOperandStack objects. Possibly, some of the state could be
// shared to save some memory. For now, simplicity is the goal.
//
// VirtualMachineOperandStack implements the VirtualMachineState interface:
// Commit() simply iterates over the simulated operand stack and stores each
//   value onto the virtual machine's operand stack (more details at definition).
// Reload() is left empty; assumption is that each BytecodeBuilder handler will
//   update the state of the operand stack appropriately on return from the
//   interpreter.
// MergeInto() is slightly subtle. Operations may have been already created
//   below the merge point, and those operations will have assumed the
//   expressions are stored in the TR::IlValue's for the state being Merged
//   *to*. So the purpose of Merge() is to store the values of the current
//   state into the same variables as in the "other" state.
// 
// VirtualMachineOperandStack provides several stack-y operations:
//   Push() pushes a TR::IlValue onto the stack
//   Pop() pops and returns a TR::IlValue from the stack
//   Top() returns the TR::IlValue at the top of the stack
//   Pick() returns the TR::IlValue "depth" elements from the top
//   Drop() discards "depth" elements from the stack
//   Dup() is a convenience function for Push(Top())
//
// To facilitate propagating an operand stack as control flows from one bytecode
// to another, VirtualMachineOperandStack provides a function MakeCopy() which
// allocates a new VirtualMachineOperandStack and makes it an identical
// copy of the current operand stack. It is typically used to 

class VirtualMachineOperandStack : public ::OMR::VirtualMachineState
   {
   public:
   // must be instantiated inside a compilation because uses heap memory
   VirtualMachineOperandStack(TR::MethodBuilder *mb, int32_t sizeHint, TR::IlType *elementType);
   VirtualMachineOperandStack(TR::MethodBuilder *mb, int32_t sizeHint, TR::IlType *elementType, OMR::VirtualMachineRegister *stackBaseReg);

   // Commit() writes out the values currently on the simulated
   // operand stack to the actual virtual machine state, typically done in preparation
   // for transition to the interpreter.
   virtual void Commit(TR::IlBuilder *b);

   // Merge() will insert operations into the given builder object to make this
   // virtual machine state match another virtual machine state, typically done at a
   // merge point. Routing control to and from the builder "b" is the caller's
   // responsibility.
   virtual void MergeInto(OMR::VirtualMachineOperandStack *other, TR::IlBuilder *b);

   // MakeCopy() creates an identical copy of the current object, typically
   // used to provide the input state at the destination of a control flow edge
   // Any subclasses *must* implement this function
   virtual VirtualMachineOperandStack *MakeCopy();

   // the usual stack operation to Push an expression onto the operand stack
   virtual void Push(TR::IlValue *value);

   // the usual destructive Pop() and non-destructive Top() calls for a stack
   virtual TR::IlValue *Pop();
   virtual TR::IlValue *Top();

   // Pick nondestructively returns the value that would be returned by Top() after
   // "depth" successive Pop()s, except that the stack is not changed at all
   virtual TR::IlValue *Pick(int32_t depth);

   // drop the top "depth" elements from the stack: same as calling Pop "depth" times
   virtual void Drop(int32_t depth);

   // the usual stack operation to duplicate the top element on the stack
   virtual void Dup();

   protected:
   void copyTo(OMR::VirtualMachineOperandStack *copy);
   void checkSize();
   void grow();

   private:
   TR::MethodBuilder *_mb;
   int32_t _stackMax;
   int32_t _stackTop;
   TR::IlValue **_stack;
   TR::IlType *_elementType;
   };
}

#endif // !defined(OMR_VIRTUALMACHINEOPERANDSTACK_INCL
