/*******************************************************************************
 * Copyright (c) 2016, 2016 IBM Corp. and others
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

#ifndef INTERPRETERTYPES_INCL
#define INTERPRETERTYPES_INCL

#include "ilgen/InterpreterBuilder.hpp"
#include "ilgen/VirtualMachineState.hpp"
#include "ilgen/VirtualMachineRegister.hpp"
#include "ilgen/VirtualMachineStack.hpp"

#define STACKVALUEILTYPE Int64
#define STACKVALUETYPE int64_t

typedef int64_t (InterpreterMethodFunction)(struct Interpreter *, struct Frame *);
typedef int64_t (JitMethodFunction)(struct Interpreter *, struct Frame *);

typedef struct Method
   {
   int32_t callsUntilJit;
   int32_t argCount;
   int32_t bytecodeLength;
   int8_t const *bytecodes;
   char const *name;
   JitMethodFunction *compiledMethod;
   } Method;

enum FrameTypes
   {
   INTERPRETER,
   JIT
   };

typedef struct Frame
   {
   Frame *previous;
   int32_t savedPC;
   int8_t const *bytecodes;
   int32_t frameType;
   STACKVALUETYPE *locals;
   STACKVALUETYPE loc[10];
   STACKVALUETYPE *sp;
   STACKVALUETYPE stack[10];
   } Frame;

typedef struct Interpreter
   {
   Frame *currentFrame;
   Method *methods;
   } Interpreter;

typedef TR::IlValue * (*MathFuncType)(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right);
typedef TR::IlValue * (*BooleanFuncType)(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right);

enum interpreter_opcodes
   {
   PUSH_CONSTANT = OMR::InterpreterBuilder::OPCODES::BC_00,
   DUP = OMR::InterpreterBuilder::OPCODES::BC_01,
   ADD = OMR::InterpreterBuilder::OPCODES::BC_02,
   SUB = OMR::InterpreterBuilder::OPCODES::BC_03,
   MUL = OMR::InterpreterBuilder::OPCODES::BC_04,
   DIV = OMR::InterpreterBuilder::OPCODES::BC_05,
   RET = OMR::InterpreterBuilder::OPCODES::BC_06,
   CALL = OMR::InterpreterBuilder::OPCODES::BC_07,
   EXIT = OMR::InterpreterBuilder::OPCODES::BC_08,
   JMPL = OMR::InterpreterBuilder::OPCODES::BC_09,
   PUSH_LOCAL = OMR::InterpreterBuilder::OPCODES::BC_10,
   POP_LOCAL = OMR::InterpreterBuilder::OPCODES::BC_11,
   FAIL = OMR::InterpreterBuilder::OPCODES::BC_12,
   COUNT = OMR::InterpreterBuilder::OPCODES::BC_13
   };

class InterpreterVMState : public TR::VirtualMachineState
   {
   public:
   InterpreterVMState()
      : TR::VirtualMachineState(),
      _stack(nullptr),
      _stackTop(nullptr)
      { }

   InterpreterVMState(TR::VirtualMachineStack *stack, TR::VirtualMachineRegister *stackTop)
      : TR::VirtualMachineState(),
      _stack(stack),
      _stackTop(stackTop)
      {
      }

   virtual void Commit(TR::IlBuilder *b)
      {
      _stack->Commit(b);
      _stackTop->Commit(b);
      }

   virtual void Reload(TR::IlBuilder *b)
      {
      _stackTop->Reload(b);
      _stack->Reload(b);
      }

   virtual VirtualMachineState *MakeCopy()
      {
      InterpreterVMState *newState = new InterpreterVMState();
      newState->_stack = (TR::VirtualMachineStack *)_stack->MakeCopy();
      newState->_stackTop = (TR::VirtualMachineRegister *) _stackTop->MakeCopy();
      return newState;
      }

   virtual void MergeInto(VirtualMachineState *other, TR::IlBuilder *b)
      {
      InterpreterVMState *otherState = (InterpreterVMState *)other;
      _stack->MergeInto(otherState->_stack, b);
      _stackTop->MergeInto(otherState->_stackTop, b);
      }

   TR::VirtualMachineStack * _stack;
   TR::VirtualMachineRegister * _stackTop;
   };

#endif // !defined(INTERPRETERTYPES_INCL)
