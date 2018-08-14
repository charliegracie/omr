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

#define STACKVALUEILTYPE Int64
#define STACKVALUETYPE int64_t

typedef int64_t (InterpreterMethodFunction)(struct Interpreter *, struct Frame *);
typedef int64_t (JitMethodFunction)(struct Frame *);

typedef struct Method
   {
   int32_t callsUntilJit;
   int8_t const *bytecodes;
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
   Method *methods;
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

#endif // !defined(INTERPRETERTYPES_INCL)
