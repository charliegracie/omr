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


#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include "Jit.hpp"
#include "ilgen/InterpreterBuilder.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "Interpreter.hpp"

using std::cout;
using std::cerr;

static void printString(int64_t ptr)
   {
   #define PRINTSTRING_LINE LINETOSTR(__LINE__)
   char *string = (char *) ptr;
   printf("%s", string);
   }

static void printInt8(int8_t val)
   {
   #define PRINTINT8_LINE LINETOSTR(__LINE__)
   printf("%d", val);
   }

static void printInt32(int32_t val)
   {
   #define PRINTINT32_LINE LINETOSTR(__LINE__)
   printf("%d", val);
   }

static void printInt64(int64_t val)
   {
   #define PRINTINT64_LINE LINETOSTR(__LINE__)
   printf("%ld", val);
   }

int
main(int argc, char *argv[])
   {

   cout << "Step 1: initialize JIT\n";
   bool initialized = initializeJit();
   if (!initialized)
      {
      cerr << "FAIL: could not initialize JIT\n";
      exit(-1);
      }

   cout << "Step 2: define type dictionary\n";
   TR::TypeDictionary types;

   cout << "Step 3: compile method builder\n";
   InterpreterMethod method(&types);
   uint8_t *entry = 0;
   int32_t rc = compileMethodBuilder(&method, &entry);
   if (rc != 0)
      {
      cerr << "fail: compilation error " << rc << "\n";
      exit(-2);
      }

   cout << "step 4: invoke compiled code and print results\n";
   typedef int64_t (InterpreterMethodFunction)(int64_t*, int8_t *);
   InterpreterMethodFunction *interpreter = (InterpreterMethodFunction *) entry;
   int64_t stack[10] = {};
   int8_t bytecodes[] =
      {
      0,3,  // push 3
      0,5,  // push 5
      1,-1, // add 3 + 5 store 8
      0,2,  // push 2
      2,-1, // sub 8 - 2 store 6
      3,-1, // return 6
      };

   int64_t result = interpreter(stack, bytecodes);

   cout << "interpreter(values) = " << result << "\n";

   cout << "Step 5: shutdown JIT\n";
   shutdownJit();
   }

InterpreterMethod::InterpreterMethod(TR::TypeDictionary *d)
   : InterpreterBuilder(d)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   DefineName("Interpreter");

   pInt64 = d->PointerTo(Int64);
   DefineParameter("stack", pInt64);
   pInt8 = d->PointerTo(Int8);
   DefineParameter("bytecodes", pInt8);

   DefineLocal("pc", Int32);
   DefineLocal("opcode", Int32);
   DefineLocal("exitLoop", Int32);

   DefineReturnType(Int64);

   DefineFunction((char *)"printString", (char *)__FILE__, (char *)PRINTSTRING_LINE, (void *)&printString, NoType, 1, Int64);
   DefineFunction((char *)"printInt8", (char *)__FILE__, (char *)PRINTINT8_LINE, (void *)&printInt8, NoType, 1, Int8);
   DefineFunction((char *)"printInt32", (char *)__FILE__, (char *)PRINTINT32_LINE, (void *)&printInt32, NoType, 1, Int32);
   DefineFunction((char *)"printInt64", (char *)__FILE__, (char *)PRINTINT64_LINE, (void *)&printInt64, NoType, 1, Int64);
   }

TR::IlValue *
InterpreterMethod::add(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right)
   {
   return builder->Add(left, right);
   }

TR::IlValue *
InterpreterMethod::sub(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right)
   {
   return builder->Sub(left, right);
   }

TR::IlValue *
InterpreterMethod::mul(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right)
   {
   return builder->Mul(left, right);
   }

TR::IlValue *
InterpreterMethod::div(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right)
   {
   return builder->Div(left, right);
   }

void
InterpreterMethod::getNextOpcode(TR::IlBuilder *builder)
   {
   builder->Store("opcode",
   builder->   ConvertTo(Int32,
   builder->      LoadAt(pInt8,
   builder->         IndexAt(pInt8,
   builder->            Load("bytecodes"),
                        getPC(builder)))));
   }

void
InterpreterMethod::setPC(TR::IlBuilder *builder, int32_t value)
   {
   setPC(builder, builder->ConstInt32(value));
   }

void
InterpreterMethod::setPC(TR::IlBuilder *builder, TR::IlValue *value)
   {
   builder->Store("pc", value);
   }

void
InterpreterMethod::incrementPC(TR::IlBuilder *builder, int32_t increment)
   {
   TR::IlValue *pc = getPC(builder);
   TR::IlValue *incrementValue = builder->ConstInt32(increment);

   pc = builder->Add(pc, incrementValue);

   setPC(builder, pc);
   }

TR::IlValue *
InterpreterMethod::getPC(TR::IlBuilder *builder)
   {
   return builder->Load("pc");
   }

//TR::IlValue *
//InterpreterMethod::doMath(TR::IlBuilder *builder, MathFuncType mathFunction, TR::IlValue *left, TR::IlValue *right)
//   {
//   return (*mathFunction)(builder, left, right);
//   }

void
InterpreterMethod::incrementStack(TR::IlBuilder *builder)
   {
   TR::IlValue *stackPtr = builder->Load("stack");
   stackPtr = builder->Add(stackPtr, builder->ConstInt32(8));
   builder->Store("stack", stackPtr);
   }

void
InterpreterMethod::decrementStack(TR::IlBuilder *builder)
   {
   TR::IlValue *stackPtr = builder->Load("stack");
   stackPtr = builder->Sub(stackPtr, builder->ConstInt32(8));
   builder->Store("stack", stackPtr);
   }

void
InterpreterMethod::writeToStack(TR::IlBuilder *builder, TR::IlValue *value)
   {
   builder->StoreAt(
   builder->   Load("stack"),
   builder->   ConvertTo(Int64, value));
   }

TR::IlValue *
InterpreterMethod::readFromStack(TR::IlBuilder *builder)
   {
   return builder->LoadAt(pInt64,
          builder->   Load("stack"));
   }

void
InterpreterMethod::push(TR::IlBuilder *builder, TR::IlValue *value)
   {
   writeToStack(builder, value);
   incrementStack(builder);
   }

TR::IlValue *
InterpreterMethod::pop(TR::IlBuilder *builder)
   {
   decrementStack(builder);
   return readFromStack(builder);
   }

bool
InterpreterMethod::buildIL()
   {
   cout << "InterpreterMethod::buildIL() running!\n";

   TR::IlBuilder *doWhileBody = NULL;
   TR::IlBuilder *breakBody = NULL;

   setPC(this, 0);

   DoWhileLoopWithBreak("exitLoop", &doWhileBody, &breakBody);

   getNextOpcode(doWhileBody);

   TR::IlBuilder *opcodeBuilders[interpreter_opcodes::COUNT] = {NULL};
   TR::IlBuilder *defaultBldr = NULL;
   TR::IlBuilder *case0Bldr = NULL;

   doWhileBody->Switch("opcode", &defaultBldr, interpreter_opcodes::COUNT,
                   interpreter_opcodes::PUSH, &opcodeBuilders[interpreter_opcodes::PUSH], false,
                   interpreter_opcodes::ADD, &opcodeBuilders[interpreter_opcodes::ADD], false,
                   interpreter_opcodes::SUB, &opcodeBuilders[interpreter_opcodes::SUB], false,
                   interpreter_opcodes::RET, &opcodeBuilders[interpreter_opcodes::RET], false);

   handlePush(opcodeBuilders[interpreter_opcodes::PUSH]);
   handleAdd(opcodeBuilders[interpreter_opcodes::ADD]);
   handleSub(opcodeBuilders[interpreter_opcodes::SUB]);
   handleReturn(opcodeBuilders[interpreter_opcodes::RET]);

   defaultBldr->Goto(&breakBody);

   incrementPC(doWhileBody, 2);

   Return(
      ConstInt64(-1));

   return true;
   }

void
InterpreterMethod::handlePush(TR::IlBuilder *builder)
   {
   TR::IlValue *value =
   builder->LoadAt(pInt8,
   builder->   IndexAt(pInt8,
   builder->      Load("bytecodes"),
   builder->      Add(
   builder->         Load("pc"),
   builder->         ConstInt32(1))));

   push(builder, value);
   }

void
InterpreterMethod::handleAdd(TR::IlBuilder *builder)
   {
   TR::IlValue *right = pop(builder);
   TR::IlValue *left = pop(builder);

   TR::IlValue *value = builder->Add(left, right);

   push(builder, value);
   }

void
InterpreterMethod::handleSub(TR::IlBuilder *builder)
   {
   TR::IlValue *right = pop(builder);
   TR::IlValue *left = pop(builder);

   TR::IlValue *value = builder->Sub(left, right);

   push(builder, value);
   }

void
InterpreterMethod::handleReturn(TR::IlBuilder *builder)
   {
   TR::IlValue *ret = pop(builder);
   builder->Return(ret);
   }
