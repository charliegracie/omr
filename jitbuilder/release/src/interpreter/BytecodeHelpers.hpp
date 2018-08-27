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


#ifndef BYTECODEHELPERS_INCL
#define BYTECODEHELPERS_INCL

namespace TR { class IlBuilder; }

class BytecodeHelpers
   {
   public:
   static void DefineFunctions(TR::MethodBuilder *mb);

   static Frame *allocateFrame();
   static void freeFrame(Frame *frame);
   static void setupArgs(Frame *newFrame, Frame *frame, int8_t argCount);
   static void pushReturn(Frame *frame, int64_t retVal);
   static Frame *i2jTransition(Interpreter *interp, JitMethodFunction *func);
   static Frame *j2jTransition(Interpreter *interp, JitMethodFunction *func);
   static Frame *j2iTransition(Interpreter *interp);;
   static Frame *i2iTransition(Interpreter *interp, int8_t argCount);
   static void compileMethod(Interpreter *interp, int8_t methodIndex);

   protected:
   private:
   static Frame *transitionToJIT(Interpreter *interp, JitMethodFunction *func);
   };

#endif // !defined(BYTECODEHELPERS_INCL)
