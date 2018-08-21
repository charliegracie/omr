/*******************************************************************************
 * Copyright (c) 2000, 2018 IBM Corp. and others
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

#ifndef OMR_RUNTIMEBUILDER_INCL
#define OMR_RUNTIMEBUILDER_INCL

#include "ilgen/MethodBuilder.hpp"

namespace TR { class RuntimeBuilder; }
namespace TR { class BytecodeBuilder;}
namespace TR { class VirtualMachineRegister; }
namespace TR { class VirtualMachineInterpreterStack; }

namespace OMR
{

class RuntimeBuilder : public TR::MethodBuilder
   {
public:
   TR_ALLOC(TR_Memory::IlGenerator)

   RuntimeBuilder(TR::TypeDictionary *d)
   : TR::MethodBuilder(d)
   {
   }

   virtual TR::IlValue *GetImmediate(TR::BytecodeBuilder *builder, int32_t pcOffset) = 0;
   virtual void DefaultFallthroughTarget(TR::BytecodeBuilder *builder) = 0;
   virtual void SetJumpIfTarget(TR::BytecodeBuilder *builder, TR::IlValue *condition, TR::IlValue *jumpTarget) = 0;
   virtual void ReturnTarget(TR::BytecodeBuilder *builder) = 0;

protected:


private:
   };

} // namespace OMR

#endif // !defined(OMR_RUNTIMEBUILDER_INCL)
