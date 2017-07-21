/*******************************************************************************
 * Copyright (c) 2000, 2017 IBM Corp. and others
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

#ifndef BYTECODE_BUILDER_INCL
#define BYTECODE_BUILDER_INCL

#ifndef TR_BYTECODEBUILDER_DEFINED
#define TR_BYTECODEBUILDER_DEFINED
#define PUT_OMR_BYTECODEBUILDER_INTO_TR
#endif // !defined(TR_BYTECODEBUILDER_DEFINED)

#include "ilgen/BytecodeBuilderRecorder.hpp"

namespace TR { class BytecodeBuilder; }
namespace TR { class MethodBuilder; }
namespace OMR { class VirtualMachineState; }

namespace OMR
{

class BytecodeBuilder : public TR::BytecodeBuilderRecorder
   {
public:
   TR_ALLOC(TR_Memory::IlGenerator)

   BytecodeBuilder(TR::MethodBuilder *methodBuilder, int32_t bcIndex, char *name=NULL);
   TR::BytecodeBuilder *self();

   /* @brief after calling this, all IL nodes created will have this BytecodeBuilder's _bcIndex */
   void SetCurrentIlGenerator();

   void AddFallThroughBuilder(TR::BytecodeBuilder *ftb);
   void AddSuccessorBuilders(uint32_t numBuilders, ...);
   void AddSuccessorBuilder(TR::BytecodeBuilder **b) { AddSuccessorBuilders(1, b); }

   void initialize(TR::IlGeneratorMethodDetails * details,
                   TR::ResolvedMethodSymbol     * methodSymbol,
                   TR::FrontEnd                 * fe,
                   TR::SymbolReferenceTable     * symRefTab); 
   virtual uint32_t countBlocks();

protected:
   TR::BytecodeBuilder       * _fallThroughBuilder;
   List<TR::BytecodeBuilder> * _successorBuilders;

   virtual void appendBlock(TR::Block *block = 0, bool addEdge=true);
   void addAllSuccessorBuildersToWorklist();
   bool connectTrees();
   virtual void setHandlerInfo(uint32_t catchType);
   void transferVMState(TR::BytecodeBuilder **b);
   };

} // namespace OMR


#if defined(PUT_OMR_BYTECODEBUILDER_INTO_TR)

namespace TR
{
   class BytecodeBuilder : public OMR::BytecodeBuilder
      {
      public:
         BytecodeBuilder(TR::MethodBuilder *methodBuilder, int32_t bcIndex, char *name=NULL)
            : OMR::BytecodeBuilder(methodBuilder, bcIndex, name)
            { }
      };

} // namespace TR

#endif // defined(PUT_OMR_BYTECODEBUILDER_INTO_TR)

#endif // !defined(OMR_ILBUILDER_INCL)
