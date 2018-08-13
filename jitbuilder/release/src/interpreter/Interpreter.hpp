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


#ifndef INTERPRETER_INCL
#define INTERPRETER_INCL

#include "ilgen/InterpreterBuilder.hpp"

class InterpreterMethod : public TR::InterpreterBuilder
   {
   public:
   InterpreterMethod(TR::TypeDictionary *d);
   virtual void registerBytecodeBuilders();
   virtual void handleReturn(TR::IlBuilder *builder);
   virtual TR::VirtualMachineInterpreterStack *createStack();
   virtual void loadOpcodeArray();

   protected:

   private:
   TR::IlType *pInt8;
   TR::IlType *frame;
   TR::IlType *pFrame;

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
      FAIL = OMR::InterpreterBuilder::OPCODES::BC_12
      };

   public:
   const int8_t _mainMethod[36] =
      {
      interpreter_opcodes::PUSH_CONSTANT,3, // push 3
      interpreter_opcodes::PUSH_CONSTANT,5, // push 5
      interpreter_opcodes::ADD,             // add 3 + 5 store 8
      interpreter_opcodes::PUSH_CONSTANT,2, // push 2
      interpreter_opcodes::SUB,             // sub 8 - 2 store 6
      interpreter_opcodes::PUSH_CONSTANT,4, // push 4
      interpreter_opcodes::MUL,             // mul 6 * 4 store 24
      interpreter_opcodes::PUSH_CONSTANT,8, // push 8
      interpreter_opcodes::DIV,             // div 24 / 8 store 3
      interpreter_opcodes::DUP,             // dup 3 store 3
      interpreter_opcodes::ADD,             // add 3 + 3 store 6
      interpreter_opcodes::CALL,1,0,        // call method 1 (call result 8)
      interpreter_opcodes::ADD,             // add 6 + 8 (call result) store 14
      interpreter_opcodes::PUSH_CONSTANT,2, // push 2
      interpreter_opcodes::CALL,2,2,        // call method 2 (call result 7)
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 3)
      interpreter_opcodes::PUSH_CONSTANT,4, // push 4
      interpreter_opcodes::MUL,             // mul 3 * 4 store 12
      interpreter_opcodes::CALL,5,1,        // call method 5 (call result fib(12))

      //interpreter_opcodes::FAIL,-1,

      interpreter_opcodes::EXIT,-1, // return 144 (call result)
      };

   const int8_t _testCallMethod[9] =
      {
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::PUSH_CONSTANT,7, // push 7
      interpreter_opcodes::CALL,3,2,        // call method 3 (call result 8)
      interpreter_opcodes::RET,1,           // ret 8
      };

   const int8_t _testDivMethod[3] =
      {
      //Expecting 2 args to be pushed!
      interpreter_opcodes::DIV,             // arg 1 (14) / arg2 (2) store 7
      interpreter_opcodes::RET,1,           // ret 7
      };

   const int8_t _testAddMethod[3] =
      {
      //Expecting 2 args to be pushed!
      interpreter_opcodes::ADD,             // arg 1 (1) + arg2 (7) store 8
      interpreter_opcodes::RET,1,           // ret 8
      };

   const int8_t _testJMPLMethod[12] =
      {
      //Expecting 1 arg
      interpreter_opcodes::PUSH_CONSTANT,5, // push 5
      interpreter_opcodes::JMPL,8,          // if arg < 5
      interpreter_opcodes::PUSH_CONSTANT,3, // push 1
      interpreter_opcodes::RET,1,           // ret 3
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::RET,1,           // ret 1
      };

   const int8_t _fib[38] =
      {
      // Expecting 1 arg
      // save arg in locals[0]
      interpreter_opcodes::DUP,             // dup arg store arg..........stack is arg,arg
      interpreter_opcodes::POP_LOCAL,0,     // pop arg into local 0.......stack is arg
      // if arg < 3 goto push 1/ret
      interpreter_opcodes::PUSH_CONSTANT,2, // push 2
      interpreter_opcodes::JMPL,34,         // if arg < 2
      // call f(n-1)
      interpreter_opcodes::PUSH_LOCAL,0,    // push local 0...............stack is arg
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1.....................stack is arg,1
      interpreter_opcodes::SUB,             // arg - 1 store (arg - 1)....stack is (arg-1)
      interpreter_opcodes::CALL,5,1,        // call method fib............stack is fib(arg-1)
      interpreter_opcodes::POP_LOCAL,1,     // pop into local 1...........stack is
      // call fib(n-2)
      interpreter_opcodes::PUSH_LOCAL,0,    // push local 0...............stack is arg
      interpreter_opcodes::PUSH_CONSTANT,2, // push 2.....................stack is arg,2
      interpreter_opcodes::SUB,             // arg - 2 store (arg - 2)....stack is (arg-2)
      interpreter_opcodes::CALL,5,1,        // call method fib............stack is fib(arg-2)
      interpreter_opcodes::POP_LOCAL,0,     // pop into local 0...........stack is
      // add fib(n-1) + fib(n-2)
      interpreter_opcodes::PUSH_LOCAL,1,    // push local 1..............stack is fib(arg-1)
      interpreter_opcodes::PUSH_LOCAL,0,    // push local 0..............stack is fib(arg-2)
      interpreter_opcodes::ADD,             // fib(arg-1) + fib(arg-2)...stack is (fib(arg-1) + fib(arg-2))
      interpreter_opcodes::RET,1,           // ret fib(arg-1) + fib(arg-2)
      // return 1
      interpreter_opcodes::PUSH_LOCAL,0,    // push arg
      interpreter_opcodes::RET,1,           // ret arg
      };

   };

#endif // !defined(INTERPRETER_INCL)
