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
#include "ilgen/VirtualMachineInterpreterStack.hpp"
#include "ilgen/VirtualMachineState.hpp"
#include "ilgen/VirtualMachineRegister.hpp"

#include "Interpreter.hpp"

enum interpreter_opcodes
   {
   PUSH = OMR::InterpreterBuilder::OPCODES::BC_00,
   ADD = OMR::InterpreterBuilder::OPCODES::BC_01,
   SUB = OMR::InterpreterBuilder::OPCODES::BC_02,
   MUL = OMR::InterpreterBuilder::OPCODES::BC_03,
   DIV = OMR::InterpreterBuilder::OPCODES::BC_04,
   RET = OMR::InterpreterBuilder::OPCODES::BC_05,
   FAIL = OMR::InterpreterBuilder::OPCODES::BC_06
   };

using std::cout;
using std::cerr;

#define PRINTSTRING(builder, value) ((builder)->Call("printString", 1, (builder)->ConstInt64((int64_t)value)))
#define PRINTINT64(builder, value) ((builder)->Call("printInt64", 1, (builder)->ConstInt64((int64_t)value))
#define PRINTINT64VALUE(builder, value) ((builder)->Call("printInt64", 1, value))

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
   typedef int64_t (InterpreterMethodFunction)(int64_t **, int8_t *);
   InterpreterMethodFunction *interpreter = (InterpreterMethodFunction *) entry;
   int64_t *stack = (int64_t *)malloc(sizeof(int64_t) * 10);
   if (NULL == stack)
      {
      cerr << "fail: failed to allocated stack\n";
      exit(-3);
      }
   int8_t bytecodes[] =
      {
      interpreter_opcodes::PUSH,3, // push 3
      interpreter_opcodes::PUSH,5, // push 5
      interpreter_opcodes::ADD,-1, // add 3 + 5 store 8
      interpreter_opcodes::PUSH,2, // push 2
      interpreter_opcodes::SUB,-1, // sub 8 - 2 store 6
      interpreter_opcodes::PUSH,4, // push 4
      interpreter_opcodes::MUL,-1, // mul 6 * 4 store 24
      interpreter_opcodes::PUSH,8, // push 8
      interpreter_opcodes::DIV,-1, // div 24 / 8 store 3

      //interpreter_opcodes::FAIL,-1,

      interpreter_opcodes::RET,-1, // return 3
      };

   int64_t result = interpreter(&stack, bytecodes);

   free(stack);

   cout << "interpreter(values) = " << result << "\n";

   cout << "Step 5: shutdown JIT\n";
   shutdownJit();
   }

InterpreterMethod::InterpreterMethod(TR::TypeDictionary *d)
   : MethodBuilder(d)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   pInt8 = d->PointerTo(Int8);

   DefineName("Interpreter");

   DefineParameter("stackPtrPtr", d->PointerTo(d->PointerTo(Int64)));
   DefineParameter("bytecodes", pInt8);

   DefineLocal("pc", Int32);
   DefineLocal("opcode", Int32);

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

bool
InterpreterMethod::buildIL()
   {
   cout << "InterpreterMethod::buildIL() running!\n";

   TR::VirtualMachineRegister *stackRegister = new TR::VirtualMachineRegister(this, "_STACK_", _types->PointerTo(_types->PointerTo(Int64)), sizeof(Int64), Load("stackPtrPtr"));
   TR::VirtualMachineInterpreterStack *stack = new TR::VirtualMachineInterpreterStack(this, stackRegister, Int64);

   _interpreterBuilder = new TR::InterpreterBuilder(this, _types, stack, "bytecodes", Int8, "pc", "opcode");

   handlePush(_interpreterBuilder->registerOpcodeHandler(interpreter_opcodes::PUSH));
   handleMath(_interpreterBuilder->registerOpcodeHandler(interpreter_opcodes::ADD), &InterpreterMethod::add);
   handleMath(_interpreterBuilder->registerOpcodeHandler(interpreter_opcodes::SUB), &InterpreterMethod::sub);
   handleMath(_interpreterBuilder->registerOpcodeHandler(interpreter_opcodes::MUL), &InterpreterMethod::mul);
   handleMath(_interpreterBuilder->registerOpcodeHandler(interpreter_opcodes::DIV), &InterpreterMethod::div);
   handleReturn(_interpreterBuilder->registerOpcodeHandler(interpreter_opcodes::RET));

   _interpreterBuilder->execute(this);

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

   _interpreterBuilder->getState()->Push(builder, value);
   }

void
InterpreterMethod::handleMath(TR::IlBuilder *builder, MathFuncType mathFunction)
   {
   TR::IlValue *right = _interpreterBuilder->getState()->Pop(builder);
   TR::IlValue *left = _interpreterBuilder->getState()->Pop(builder);

   TR::IlValue *value = (*mathFunction)(builder, left, right);

   _interpreterBuilder->getState()->Push(builder, value);
   }

void
InterpreterMethod::handleReturn(TR::IlBuilder *builder)
   {
   TR::IlValue *ret = _interpreterBuilder->getState()->Pop(builder);
   builder->Return(ret);
   }
