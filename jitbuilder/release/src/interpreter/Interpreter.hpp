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

class InterpreterTypeDictionary;

class InterpreterMethod : public TR::InterpreterBuilder
   {
   public:
   InterpreterMethod(InterpreterTypeDictionary *d);

   //TR::RuntimeBuilder overrides
   virtual void DefineFunctions(TR::MethodBuilder *mb);

   //TR::InterpreterBuilder overrides
   virtual void registerBytecodeBuilders();
   virtual void handleInterpreterExit(TR::IlBuilder *builder);
   virtual TR::VirtualMachineState *createVMState();
   virtual void loadBytecodes(TR::IlBuilder *builder);
   virtual void loadPC(TR::IlBuilder *builder);
   virtual void savePC(TR::IlBuilder *builder, TR::IlValue *pc);

   protected:

   private:
   TR::IlType *pInt8;
   InterpreterTypeDictionary *_interpTypes;

   public:
   //const int8_t _mainMethod[52] =
   //const int8_t _mainMethod[102] =
   const int8_t _mainMethod[170] =
      {
      interpreter_opcodes::PRINT_STRING,0,  // print "HelloWorld!"
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
#if 1
      interpreter_opcodes::CALL,1,0,        // call method 1 (call result 8)
#if 1
      interpreter_opcodes::ADD,             // add 6 + 8 (call result) store 14
      interpreter_opcodes::PUSH_CONSTANT,2, // push 2
      interpreter_opcodes::CALL,2,2,        // call method 2 (call result 7)
#if 1
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::CALL,2,2,        // call method 2 (call result 7)
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::CALL,2,2,        // call method 2 (call result 7)
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::CALL,2,2,        // call method 2 (call result 7)
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::CALL,2,2,        // call method 2 (call result 7)
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::CALL,2,2,        // call method 2 (call result 7)
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::CALL,2,2,        // call method 2 (call result 7)
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::CALL,2,2,        // call method 2 (call result 7)
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::CALL,2,2,        // call method 2 (call result 7)
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::CALL,2,2,        // call method 2 (call result 7)
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::CALL,2,2,        // call method 2 (call result 7)
#endif
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 3)

#if 1
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 1)
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 1)
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 1)
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 1)
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 1)
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 1)
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 1)
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 1)
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 1)
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 1)
      interpreter_opcodes::PUSH_CONSTANT,7, // push 7
      interpreter_opcodes::ADD,             // add 1 + 7 store 10
      interpreter_opcodes::CALL,4,1,        // call method 3 (call result 3)
      //interpreter_opcodes::PUSH_CONSTANT,2, // push 2
      //interpreter_opcodes::ADD,             // add 1 + 1 store 3
#endif
#if 1
      interpreter_opcodes::PUSH_CONSTANT,4, // push 4
      interpreter_opcodes::MUL,             // mul 3 * 4 store 12
      interpreter_opcodes::CALL,5,1,        // call method 5 (call result fib(12)) store 144
      interpreter_opcodes::PUSH_CONSTANT,12,// push 12
      interpreter_opcodes::DIV,             // div 144 / 12 store 12
      interpreter_opcodes::PUSH_CONSTANT,3, // push 3
      interpreter_opcodes::SUB,             // sub 12 - 3 store 9
      interpreter_opcodes::CALL,6,1,        // call method 6 (call iterFib(9)) store 34
#endif
#endif
#endif
#if 1
      interpreter_opcodes::PUSH_CONSTANT,2, // push 2
      interpreter_opcodes::ADD,             // add 34 + 2 store 36
      interpreter_opcodes::CURRENT_TIME,    // push current time in ms
      interpreter_opcodes::POP_LOCAL,0,     // pop time into local 0
      interpreter_opcodes::CALL,5,1,        // call method 5 (call result fib(36)) store 14930352
      interpreter_opcodes::CURRENT_TIME,    // push current time in ms
      interpreter_opcodes::PUSH_LOCAL,0,    // push local 0
      interpreter_opcodes::SUB,             // sub timeEnd - timeStart = timeTaken
      interpreter_opcodes::PRINT_STRING,3,  // print "Time taken for Fib "
      interpreter_opcodes::PRINT_INT64,     // printInt64 timeTaken
      interpreter_opcodes::PRINT_STRING,4,  // print "ms"
      interpreter_opcodes::PRINT_STRING,1,  // print "\n"
#endif
      //interpreter_opcodes::FAIL,-1,
#if 0
      interpreter_opcodes::CALL,7,1, // call _testJMPGMethod with arg 34 store 1
      interpreter_opcodes::CALL,7,1, // call _testJMPGMethod with arg 1 store 3
      interpreter_opcodes::CALL,7,1, // call _testJMPGMethod with arg 1 store 3
      interpreter_opcodes::CALL,7,1, // call _testJMPGMethod with arg 1 store 3
      interpreter_opcodes::CALL,7,1, // call _testJMPGMethod with arg 1 store 3
      interpreter_opcodes::CALL,7,1, // call _testJMPGMethod with arg 1 store 3
      interpreter_opcodes::CALL,7,1, // call _testJMPGMethod with arg 1 store 3
      interpreter_opcodes::CALL,7,1, // call _testJMPGMethod with arg 1 store 3
      interpreter_opcodes::CALL,7,1, // call _testJMPGMethod with arg 1 store 3
      interpreter_opcodes::CALL,7,1, // call _testJMPGMethod with arg 1 store 3
#if 0
      interpreter_opcodes::PUSH_CONSTANT,5, // push 5
      interpreter_opcodes::ADD,             // add 3 + 5 store 8
      interpreter_opcodes::CALL,7,1, // call _testJMPGMethod with arg 8 store 1
#endif
#endif

      interpreter_opcodes::EXIT,-1, // return 1 (call result)
      };

   const int8_t _testCallMethod[9] =
      {
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::PUSH_CONSTANT,7, // push 7
      interpreter_opcodes::CALL,3,2,        // call method 3 (call result 8)
      interpreter_opcodes::RET,1,           // ret 8
      };

   const int8_t _testDivMethod[7] =
      {
      //Expecting 2 args to be pushed!
      interpreter_opcodes::PUSH_ARG,0,      // push arg[0]
      interpreter_opcodes::PUSH_ARG,1,      // push arg[1]
      interpreter_opcodes::DIV,             // arg 1 (14) / arg2 (2) store 7
      interpreter_opcodes::RET,1,           // ret 7
      };

   const int8_t _testAddMethod[7] =
      {
      //Expecting 2 args to be pushed!
      interpreter_opcodes::PUSH_ARG,0,      // push arg[0]
      interpreter_opcodes::PUSH_ARG,1,      // push arg[1]
      interpreter_opcodes::ADD,             // arg 1 (1) + arg2 (7) store 8
      interpreter_opcodes::RET,1,           // ret 8
      };

   const int8_t _testJMPLMethod[14] =
      {
      //Expecting 1 arg
      interpreter_opcodes::PUSH_ARG,0,      //push arg[0]
      interpreter_opcodes::PUSH_CONSTANT,5, // push 5
      interpreter_opcodes::JMPL,10,         // if arg < 5
      interpreter_opcodes::PUSH_CONSTANT,3, // push 3
      interpreter_opcodes::RET,1,           // ret 3
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::RET,1,           // ret 1
      };

   const int8_t _testJMPGMethod[14] =
      {
      //Expecting 1 arg
      interpreter_opcodes::PUSH_ARG,0,      //push arg[0]
      interpreter_opcodes::PUSH_CONSTANT,5, // push 5
      interpreter_opcodes::JMPG,10,         // if arg > 5
      interpreter_opcodes::PUSH_CONSTANT,3, // push 3
      interpreter_opcodes::RET,1,           // ret 3
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::RET,1,           // ret 1
      };

   const int8_t _fib[40] =
      {
      // Expecting 1 arg
      interpreter_opcodes::PUSH_ARG,0,      //push arg[0]
      // save arg in locals[0]
      interpreter_opcodes::DUP,             // dup arg store arg..........stack is arg,arg
      interpreter_opcodes::POP_LOCAL,0,     // pop arg into local 0.......stack is arg
      // if arg < 2 goto push n/ret
      interpreter_opcodes::PUSH_CONSTANT,2, // push 2
      interpreter_opcodes::JMPL,36,         // if arg < 2
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
      // return n
      interpreter_opcodes::PUSH_LOCAL,0,    // push arg
      interpreter_opcodes::RET,1,           // ret arg
      };

    const int8_t _iterFib[52]
      {
      // Expecting 1 arg
      interpreter_opcodes::PUSH_ARG,0,      //push arg[0]
      // save arg in locals[0]
      interpreter_opcodes::DUP,             // dup arg store arg..........stack is arg,arg
      interpreter_opcodes::POP_LOCAL,0,     // pop arg into local 0.......stack is arg
      // if arg < 1 goto push n/ret
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::JMPL,48,          // if arg < 1

      // set up for loop
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::POP_LOCAL,1,     // pop 1 into local 1 (fib)........stack is
      interpreter_opcodes::PUSH_CONSTANT,0, // push 0
      interpreter_opcodes::POP_LOCAL,2,     // pop 0 into local 2 (temp).......stack is
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1
      interpreter_opcodes::POP_LOCAL,3,     // pop 1 into local 2 (counter)....stack is

      //start of loop
      interpreter_opcodes::PUSH_LOCAL,1,    // push local 1 (fib)..............stack is fib
      interpreter_opcodes::PUSH_LOCAL,2,    // push local 2 (temp).............stack is fib,temp
      interpreter_opcodes::PUSH_LOCAL,1,    // push local 1 (fib)..............stack is fib,temp,fib
      interpreter_opcodes::POP_LOCAL,2,     // pop fib into local 2 (temp).....stack is fib,temp
      interpreter_opcodes::ADD,             // add fib + temp store result.....stack is fib+temp
      interpreter_opcodes::POP_LOCAL,1,     // pop result into local 2 (fib)...stack is
      interpreter_opcodes::PUSH_LOCAL,3,    // push local 3 (counter)..........stack is counter
      interpreter_opcodes::PUSH_CONSTANT,1, // push 1..........................stack is counter,1
      interpreter_opcodes::ADD,             // add counter + 1 store result....stack is result
      interpreter_opcodes::DUP,             // dup result store result.........stack is result,result
      interpreter_opcodes::POP_LOCAL,3,     // pop result into local counter...stack is result
      interpreter_opcodes::PUSH_LOCAL,0,    // push local 0 (arg)..............stack is result,arg
      interpreter_opcodes::JMPL,21,          // if result < arg
      //end of loop

      interpreter_opcodes::PUSH_LOCAL,1,    // push local 1 (fib)..............stack is fib
      interpreter_opcodes::RET,1,           // ret fib

      // return n
      interpreter_opcodes::PUSH_LOCAL,0,    // push arg
      interpreter_opcodes::RET,1,           // ret arg
      };

    const char *_newLine = "\n";
    const char *_space = " ";
    const char *_helloWorld = "HelloWorld!\n";
    const char *_timeTaken = "Time taken for Fib ";
    const char *_ms = "ms";

    static const int32_t _methodCount = 8;
    Method _methods[_methodCount];
    static const int32_t _stringCount = 5;
    const char *_strings[_stringCount];
   };

#endif // !defined(INTERPRETER_INCL)
