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

#ifndef OMR_JITMETHODBUILDER_INCL
#define OMR_JITMETHODBUILDER_INCL

#include "ilgen/RuntimeBuilder.hpp"

namespace TR { class BytecodeBuilder;}
namespace TR { class VirtualMachineRegister; }
namespace TR { class VirtualMachineInterpreterStack; }

namespace OMR
{

class JitMethodBuilder : public TR::RuntimeBuilder
   {
public:
   TR_ALLOC(TR_Memory::IlGenerator)

   JitMethodBuilder(TR::TypeDictionary *d);

   virtual void DefineFunctions(TR::MethodBuilder *mb) {}

   virtual void Commit(TR::BytecodeBuilder *builder);
   virtual void Reload(TR::BytecodeBuilder *builder);
   virtual TR::IlValue *GetImmediate(TR::BytecodeBuilder *builder);
   virtual void DefaultFallthroughTarget(TR::BytecodeBuilder *builder);
   virtual void SetJumpIfTarget(TR::BytecodeBuilder *builder, TR::IlValue *condition, TR::IlValue *jumpTarget);
   virtual void ReturnTarget(TR::BytecodeBuilder *builder);

   virtual bool buildIL();
   virtual TR::BytecodeBuilder *createBuilder(OPCODES opcode, int32_t bytcodeIndex) = 0;
   virtual TR::VirtualMachineState *createVMState() = 0;
   virtual const int8_t *getBytecodes() = 0;
   virtual int32_t getNumberBytecodes() = 0;

protected:

private:
   TR::BytecodeBuilder **_builders;
   int32_t _pc;
   };

} // namespace OMR

#endif // !defined(OMR_JITMETHODBUILDER_INCL)
