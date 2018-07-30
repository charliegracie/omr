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

#include "ilgen/MethodBuilder.hpp"

enum OPCODES
   {
   BC_00,
   BC_01,
   BC_02,
   BC_03,
   BC_04,
   BC_05,
   BC_06,
   BC_07,
   BC_08,
   BC_09,
   BC_10,
   BC_11,
   BC_12,
   BC_13,
   BC_14,
   BC_15,
   BC_COUNT
   };

enum interpreter_opcodes
   {
   PUSH = BC_00,
   ADD = BC_01,
   SUB = BC_02,
   MUL = BC_03,
   DIV = BC_04,
   RET = BC_05,
   COUNT = BC_06
   };

#define STACKILTYPE Int64
#define STACKTYPE   int64_t

typedef TR::IlValue * (*MathFuncType)(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right);
typedef TR::IlValue * (*BooleanFuncType)(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right);

class InterpreterMethod : public TR::MethodBuilder
   {
   public:
   InterpreterMethod(TR::TypeDictionary *d);
   virtual bool buildIL();

   static TR::IlValue *add(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right);
   static TR::IlValue *sub(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right);
   static TR::IlValue *mul(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right);
   static TR::IlValue *div(TR::IlBuilder *builder, TR::IlValue *left, TR::IlValue *right);

   protected:
   void incrementStack(TR::IlBuilder *builder);
   void decrementStack(TR::IlBuilder *builder);
   void writeToStack(TR::IlBuilder *builder, TR::IlValue *value);
   TR::IlValue *readFromStack(TR::IlBuilder *builder);
   void push(TR::IlBuilder *builder, TR::IlValue *value);
   TR::IlValue *pop(TR::IlBuilder *builder);

   //TR::IlValue *doMath(TR::IlBuilder *builder, MathFuncType mathFunction, TR::IlValue *left, TR::IlValue *right);

   private:
   TR::IlType *pInt8;
   TR::IlType *pInt64;
   TR::VirtualMachineRegister *_stack;
   TR::InterpreterBuilder *_interpreterBuilder;

   void handlePush(TR::IlBuilder *builder);
   void handleAdd(TR::IlBuilder *builder);
   void handleSub(TR::IlBuilder *builder);
   void handleMath(TR::IlBuilder *builder, MathFuncType mathFunction);
   void handleReturn(TR::IlBuilder *builder);
   };

#endif // !defined(INTERPRETER_INCL)
