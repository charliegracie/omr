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

#ifndef JITMETHOD_INCL
#define JITMETHOD_INCL

#include "ilgen/JitMethodBuilder.hpp"

#include "InterpreterTypes.h"
#include "InterpreterTypeDictionary.hpp"

#include "PushConstantBuilder.hpp"
#include "DupBuilder.hpp"
#include "MathBuilder.hpp"
#include "RetBuilder.hpp"
#include "ExitBuilder.hpp"
#include "CallBuilder.hpp"
#include "JumpIfBuilder.hpp"
#include "PopLocalBuilder.hpp"
#include "PushLocalBuilder.hpp"

namespace TR { class BytecodeBuilder;}

class JitMethod : public TR::JitMethodBuilder
   {
   public:
   JitMethod(InterpreterTypeDictionary *d, Method *method);

   virtual void DefineFunctions(TR::MethodBuilder *mb);

   virtual TR::BytecodeBuilder *createBuilder(int32_t bytcodeIndex);
   virtual TR::VirtualMachineState *createVMState();
   virtual bool hasMoreBytecodes(int32_t currentBytecodeIndex);
   virtual TR::IlValue *getBytecodeAtIndex(TR::IlBuilder *builder, int32_t index);

   protected:

   private:
   Method *_method;
   InterpreterTypeDictionary *_interpTypes;

   };

#endif // !defined(JITMETHOD_INCL)
