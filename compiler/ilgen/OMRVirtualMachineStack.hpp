/*******************************************************************************
 * Copyright (c) 2016, 2018 IBM Corp. and others
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

#ifndef OMR_VIRTUALMACHINESTACK_INCL
#define OMR_VIRTUALMACHINESTACK_INCL

#include <stdint.h>

#include "ilgen/VirtualMachineState.hpp"

namespace TR { class IlBuilder; }
namespace TR { class IlValue; }
namespace TR { class IlType; }
namespace TR { class MethodBuilder; }
namespace TR { class VirtualMachineStack; }
namespace TR { class VirtualMachineRegister; }

namespace OMR
{

/**
 * @brief simulates an operand stack used by many bytecode based virtual machines
 * In such virtual machines, the operand stack holds the intermediate expression values
 * computed by the bytecodes. The compiler simulates this operand stack as well, but
 * what is pushed to and popped from the simulated operand stack are expression nodes
 * that represent the value computed by the bytecodes. As each bytecode pops expression
 * nodes off the operand stack, it combines them to form more complicated expressions
 * which are then pushed back onto the operand stack for consumption by other bytecodes.
 *
 * The stack is represented as an array of pointers to TR::IlValue's, making it
 * easy to use IlBuilder services to consume and compute new values. Note that,
 * unlike VirtualMachineRegister, the simulated operand stack is *not* maintained
 * by the method code as part of the method's stack frame. This approach requires
 * modelling the state of the operand stack at all program points, which means
 * there cannot only be one VirtualMachineOperandStack object.
 *
 * The current implementation does not share anything among different
 * VirtualMachineOperandStack objects. Possibly, some of the state could be
 * shared to save some memory. For now, simplicity is the goal.
 *
 * VirtualMachineOperandStack implements VirtualMachineState:
 * Commit() simply iterates over the simulated operand stack and stores each
 *   value onto the virtual machine's operand stack (more details at definition).
 * Reload() is left empty; assumption is that each BytecodeBuilder handler will
 *   update the state of the operand stack appropriately on return from the
 *   interpreter.
 * MakeCopy() copies the state of the operand stack
 * MergeInto() is slightly subtle. Operations may have been already created
 *   below the merge point, and those operations will have assumed the
 *   expressions are stored in the TR::IlValue's for the state being merged
 *   *to*. So the purpose of MergeInto() is to store the values of the current
 *   state into the same variables as in the "other" state.
 * 
 * VirtualMachineOperandStack provides several stack-y operations:
 *   Push() pushes a TR::IlValue onto the stack
 *   Pop() pops and returns a TR::IlValue from the stack
 *   Top() returns the TR::IlValue at the top of the stack
 *   Pick() returns the TR::IlValue "depth" elements from the top
 *   Drop() discards "depth" elements from the stack
 *   Dup() is a convenience function for Push(Top())
 *
 */

class VirtualMachineStack : public TR::VirtualMachineState
   {
   public:
   VirtualMachineStack()
      : TR::VirtualMachineState()
      {
      }

   /**
    * @brief update the values used to read and write the virtual machine stack
    * @param b the builder where the values will be placed
    * @param stack the new stack base address.  It is assumed that the address is already adjusted to _stackOffset
    */
   virtual void UpdateStack(TR::IlBuilder *b, TR::IlValue *stack) = 0;

   /**
    * @brief Push an expression onto the simulated operand stack
    * @param b builder object to use for any operations used to implement the push (e.g. update the top of stack)
    * @param value expression to push onto the simulated operand stack
    */
   virtual void Push(TR::IlBuilder *b, TR::IlValue *value) = 0;

   /**
    * @brief Pops an expression from the top of the simulated operand stack
    * @param b builder object to use for any operations used to implement the pop (e.g. to update the top of stack)
    * @returns expression popped from the stack
    */
   virtual TR::IlValue *Pop(TR::IlBuilder *b) = 0;

   /**
    * @brief Returns the expression at the top of the simulated operand stack
    * @param b builder object to use for any operations used to implement the pop (e.g. to update the top of stack)
    * @returns the expression at the top of the operand stack
    */
   virtual TR::IlValue *Top(TR::IlBuilder *b) = 0;

   /**
    * @brief Returns an expression below the top of the simulated operand stack
    * @param depth number of values below top (Pick(0) is same as Top())
    * @returns the requested expression from the operand stack
    */
   virtual TR::IlValue *Pick(TR::IlBuilder *b, int32_t depth) = 0;

   /**
    * @brief Returns an expression below the top of the simulated operand stack
    * @param depth number of values below top (Pick(0) is same as Top())
    * @returns the requested expression from the operand stack
    */
   virtual TR::IlValue *Pick(TR::IlBuilder *b, TR::IlValue *depth) = 0;

   /**
    * @brief Removes some number of expressions from the operand stack
    * @param b builder object to use for any operations used to implement the drop (e.g. to update the top of stack)
    * @param depth how many values to drop from the stack
    */
   virtual void Drop(TR::IlBuilder *b, int32_t depth) = 0;

   /**
    * @brief Removes some number of expressions from the operand stack
    * @param b builder object to use for any operations used to implement the drop (e.g. to update the top of stack)
    * @param depth how many values to drop from the stack
    */
   virtual void Drop(TR::IlBuilder *b, TR::IlValue *depth) = 0;

   /**
    * @brief Duplicates the expression on top of the simulated operand stack
    * @param b builder object to use for any operations used to duplicate the expression (e.g. to update the top of stack)
    */
   virtual void Dup(TR::IlBuilder *b) = 0;

   protected:
   private:
   };
}

#endif // !defined(OMR_VIRTUALMACHINESTACK_INCL)
