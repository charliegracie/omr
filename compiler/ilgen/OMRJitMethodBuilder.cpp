/*******************************************************************************
 * Copyright (c) 2017, 2018 IBM Corp. and others
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

#include "infra/Assert.hpp"
#include "ilgen/BytecodeBuilder.hpp"
#include "ilgen/IlBuilder.hpp"
#include "ilgen/JitMethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/VirtualMachineState.hpp"
#include "ilgen/VirtualMachineRegister.hpp"

OMR::JitMethodBuilder::JitMethodBuilder(TR::TypeDictionary *d)
   : TR::RuntimeBuilder(d),
   _builders()
   {
   }

void
OMR::JitMethodBuilder::Commit(TR::BytecodeBuilder *builder)
   {
   }

void
OMR::JitMethodBuilder::Reload(TR::BytecodeBuilder *builder)
   {
   }

TR::IlValue *
OMR::JitMethodBuilder::GetImmediate(TR::BytecodeBuilder *builder)
   {
   TR::IlValue *immediate = getBytecodeAtIndex(builder, _pc);
   _pc += 1;
   return immediate;
   }

void
OMR::JitMethodBuilder::DefaultFallthroughTarget(TR::BytecodeBuilder *builder)
   {
   TR_ASSERT(_pc == (builder->bcIndex() + builder->bcLength()), "DefaultFallthroughTarget _pc %d != calculated pc %d\n", _pc, (builder->bcIndex() + builder->bcLength()));
   builder->AddFallThroughBuilder(getBuilder(_pc));
   }

void
OMR::JitMethodBuilder::SetJumpIfTarget(TR::BytecodeBuilder *builder, TR::IlValue *condition, TR::IlValue *jumpTarget)
   {
   TR::BytecodeBuilder *target = getBuilder(jumpTarget->get32bitConstValue());
   builder->IfCmpNotEqualZero(target, condition);
   DefaultFallthroughTarget(builder);
   }

void
OMR::JitMethodBuilder::ReturnTarget(TR::BytecodeBuilder *builder)
   {
   builder->Return(builder->ConstInt64(-1));
   }

bool
OMR::JitMethodBuilder::buildIL()
   {
   DefineFunctions(this);

   int32_t i = 0;
   bool canGenerate = true;
   while (hasMoreBytecodes(i))
      {
      TR::BytecodeBuilder *builder = createBuilder(i);
      if (NULL == builder)
         {
         canGenerate = false;
         break;
         }
      setBuilder(i, builder);
      i += builder->bcLength();
      }

   if (!canGenerate)
      {
      return false;
      }

   TR::VirtualMachineState *state = createVMState();
   setVMState(state);

   AppendBuilder(getBuilder(0));

   int32_t bytecodeIndex = GetNextBytecodeFromWorklist();
   while (-1 != bytecodeIndex)
      {
      TR::BytecodeBuilder *builder = getBuilder(bytecodeIndex);
      TR_ASSERT(bytecodeIndex == builder->bcIndex(), "bytecodeIndex %d must == builder->bcIndex() %d", bytecodeIndex, builder->bcIndex());

      _pc = bytecodeIndex + 1;

      builder->execute();

      bytecodeIndex = GetNextBytecodeFromWorklist();
      }

   return true;
   }

TR::BytecodeBuilder *
OMR::JitMethodBuilder::getBuilder(int32_t bcIndex)
   {
   TR::BytecodeBuilder *builder = NULL;
   auto search = _builders.find(bcIndex);
   if (search != _builders.end())
      {
      builder = search->second;
      }
   TR_ASSERT(NULL != builder, "Attempt to get a BytecodeBuilder for index %ld but it does not exist", bcIndex);
   return builder;
   }

void
OMR::JitMethodBuilder::setBuilder(int32_t bcIndex, TR::BytecodeBuilder *builder)
   {
   auto search = _builders.find(bcIndex);
   TR_ASSERT(search == _builders.end(), "Attempt to add a BytecodeBuilder at index %ld but one already exists", bcIndex);

   _builders.insert(std::make_pair(bcIndex, builder));
   }
