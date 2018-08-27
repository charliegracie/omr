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
#include "ilgen/VirtualMachineInterpreterStack.hpp"

#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

using std::cout;
using std::cerr;

OMR::JitMethodBuilder::JitMethodBuilder(TR::TypeDictionary *d)
   : TR::RuntimeBuilder(d)
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
   int8_t immediate = getBytecodes()[_pc];
   _pc += 1;
   //TODO store _pc as _pcName if there JitMethod should be implementing Commit/Reload

   return builder->ConstInt8(immediate);
   }

void
OMR::JitMethodBuilder::DefaultFallthroughTarget(TR::BytecodeBuilder *builder)
   {
   if (_pc != (builder->bcIndex() + builder->bcLength()))
      {
      int *x = 0;
      fprintf(stderr, "DefaultFallthroughTarget _pc %d != calculated pc %d\n", _pc, (builder->bcIndex() + builder->bcLength()));
      *x = 0;
      }
   builder->AddFallThroughBuilder(_builders[builder->bcIndex() + builder->bcLength()]);
   }

void
OMR::JitMethodBuilder::SetJumpIfTarget(TR::BytecodeBuilder *builder, TR::IlValue *condition, TR::IlValue *jumpTarget)
   {
   TR::BytecodeBuilder *target = _builders[jumpTarget->get32bitConstValue()];
   builder->IfCmpNotEqualZero(target, condition);
   builder->AddFallThroughBuilder(_builders[builder->bcIndex() + builder->bcLength()]);
   }

void
OMR::JitMethodBuilder::ReturnTarget(TR::BytecodeBuilder *builder)
   {
   builder->Return(builder->ConstInt64(-1));
   }

bool
OMR::JitMethodBuilder::buildIL()
   {
   cout << "JitMethodBuilder::buildIL() running!\n";

   DefineFunctions(this);

   int32_t bytecodeLength = getNumberBytecodes();
   _builders = (TR::BytecodeBuilder **)malloc(sizeof(TR::BytecodeBuilder *) * bytecodeLength);
   if (NULL == _builders)
      {
      return false;
      }

   int32_t i = 0;
   bool canGenerate = true;
   auto bytecodes = getBytecodes();
   while (canGenerate && (i < bytecodeLength))
      {
      OPCODES opcode = (OPCODES)bytecodes[i];
      _builders[i] = createBuilder(opcode, i);
      if (NULL == _builders[i])
         {
         canGenerate = false;
         break;
         }
      i += _builders[i]->bcLength();
      }

   if (!canGenerate)
      {
      free(_builders);
      return false;
      }

   TR::VirtualMachineState *state = createVMState();
   setVMState(state);

   AppendBuilder(_builders[0]);

   int32_t bytecodeIndex = GetNextBytecodeFromWorklist();
   while (-1 != bytecodeIndex)
      {
      TR::BytecodeBuilder *builder = _builders[bytecodeIndex];

      TR_ASSERT(NULL != builder, "Builder at bytecodeIndex %d can not be NULL", bytecodeIndex);
      TR_ASSERT(bytecodeIndex == builder->bcIndex(), "bytecodeIndex %d must == builder->bcIndex() %d", bytecodeIndex, builder->bcIndex());
      _pc = bytecodeIndex + 1;
      //TODO see above about _pcName

      builder->execute();

      bytecodeIndex = GetNextBytecodeFromWorklist();
      }

   free(_builders);
   return true;
   }
