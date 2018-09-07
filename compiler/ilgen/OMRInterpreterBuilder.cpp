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

#include "compile/Compilation.hpp"
#include "compile/Method.hpp"
#include "env/FrontEnd.hpp"
#include "infra/List.hpp"
#include "il/Block.hpp"
#include "ilgen/BytecodeBuilder.hpp"
#include "ilgen/IlInjector.hpp"
#include "ilgen/IlBuilder.hpp"
#include "ilgen/InterpreterBuilder.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/VirtualMachineState.hpp"
#include "ilgen/VirtualMachineRegister.hpp"

#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <algorithm>

using std::cout;
using std::cerr;

// should really move into IlInjector.hpp
#define TraceEnabled    (comp()->getOption(TR_TraceILGen))
#define TraceIL(m, ...) {if (TraceEnabled) {traceMsg(comp(), m, ##__VA_ARGS__);}}

static void handleBadOpcode(int32_t opcode, int32_t pc)
   {
   #define HANDLEBADEOPCODE_LINE LINETOSTR(__LINE__)
   fprintf(stderr, "Unknown opcode %d in interpreter loop at pc %d\n", opcode, pc);
   fflush(stderr);
   }

OMR::InterpreterBuilder::InterpreterBuilder(TR::TypeDictionary *d, const char *bytecodePtrName, TR::IlType *bytecodeElementType, const char *pcName, const char *opcodeName)
   : TR::RuntimeBuilder(d),
   _isRecursiveCall(false),
   _bytecodePtrName(bytecodePtrName),
   _bytecodeElementType(bytecodeElementType),
   _bytecodePtrType(NULL),
   _pcName(pcName),
   _opcodeName(opcodeName),
   _defaultHandler(NULL),
   _registeredBytecodes()
   {
   _bytecodePtrType = d->PointerTo(_bytecodeElementType);

   DefineFunction((char *)"handleBadOpcode", (char *)__FILE__, (char *)HANDLEBADEOPCODE_LINE, (void *)&handleBadOpcode, NoType, 2, Int32, Int32);

   DefineLocal(_pcName, Int32);
   DefineLocal(_opcodeName, Int32);
   }

void
OMR::InterpreterBuilder::Commit(TR::BytecodeBuilder *builder)
   {
   TR::IlValue *pc = builder->Load(_pcName);
   savePC(builder, pc);
   }

void
OMR::InterpreterBuilder::Reload(TR::BytecodeBuilder *builder)
   {
   loadBytecodes(builder);
   loadPC(builder);
   }

TR::IlValue *
OMR::InterpreterBuilder::GetImmediate(TR::BytecodeBuilder *builder)
   {
   TR::IlValue *pc = builder->Load(_pcName);
   TR::IlValue *increment = builder->ConstInt32(1);
   TR::IlValue *newPC = builder->Add(pc, increment);

   TR::IlValue *immediate =
   builder->LoadAt(_bytecodePtrType,
   builder->   IndexAt(_bytecodePtrType,
   builder->      Load(_bytecodePtrName),
                  pc));

   builder->Store(_pcName, newPC);

   return immediate;
   }

void
OMR::InterpreterBuilder::DefaultFallthroughTarget(TR::BytecodeBuilder *builder)
   {
   }

void
OMR::InterpreterBuilder::SetJumpIfTarget(TR::BytecodeBuilder *builder, TR::IlValue *condition, TR::IlValue *jumpTarget)
   {
   DefaultFallthroughTarget(builder);
   TR::IlBuilder *doJump = NULL;
   builder->IfThen(&doJump, condition);

   doJump->Store(_pcName, doJump->ConvertTo(Int32, jumpTarget));
   }

void
OMR::InterpreterBuilder::ReturnTarget(TR::BytecodeBuilder *builder)
   {
   if (_isRecursiveCall)
      {
      builder->Return(
      builder->   ConstInt64(-1));
      }
   else
      {
      Reload(builder);
      builder->vmState()->Reload(builder);
      }
   }

void
OMR::InterpreterBuilder::registerBytecodeBuilder(TR::BytecodeBuilder *handler)
   {
   TR_ASSERT(handler != NULL, "Can not register a NULL bytecodeBuilder");

   int32_t index = handler->bcIndex();
   TR_ASSERT(index <= 128, "Can not register an index (%d) larger than %d", index, 128);

   _registeredBytecodes.push_back(handler);

   handler->propagateVMState(vmState());

   handler->execute();
   }

bool bytecodeBuilderSort (TR::BytecodeBuilder *i,TR::BytecodeBuilder *j) { return (i->bcIndex()<j->bcIndex()); }

bool
OMR::InterpreterBuilder::buildIL()
   {
   cout << "InterpreterBuilder::buildIL() running!\n";

   DefineFunctions(this);

   setVMState(createVMState());

   _defaultHandler = OrphanBytecodeBuilder(129, "default handler");

   //TODO can this be replaced with Reload();
   loadBytecodes(this);
   loadPC(this);

   TR::BytecodeBuilder *doWhileBody = NULL;
   TR::BytecodeBuilder *breakBody = NULL;
   DoWhileLoop("exitLoop", &doWhileBody, &breakBody, NULL);

   getNextOpcode(doWhileBody);

   registerBytecodeBuilders();

   TR::IlValue *pc = getPC(doWhileBody);
   TR::IlValue *increment = doWhileBody->ConstInt32(1);
   TR::IlValue *newPC = doWhileBody->Add(pc, increment);
   setPC(doWhileBody, newPC);

   size_t numberOfRegisteredBytecodes = _registeredBytecodes.size();
   int32_t *caseValues = (int32_t *) _comp->trMemory()->allocateHeapMemory(numberOfRegisteredBytecodes * sizeof(int32_t));
   TR_ASSERT(0 != caseValues, "out of memory");

   TR::BytecodeBuilder **caseBuilders = (TR::BytecodeBuilder **) _comp->trMemory()->allocateHeapMemory(numberOfRegisteredBytecodes * sizeof(TR::BytecodeBuilder *));
   TR_ASSERT(0 != caseBuilders, "out of memory");

   bool *caseFallsThrough = (bool *) _comp->trMemory()->allocateHeapMemory(numberOfRegisteredBytecodes * sizeof(bool));
   TR_ASSERT(0 != caseFallsThrough, "out of memory");

   std::sort(_registeredBytecodes.begin(), _registeredBytecodes.end(), bytecodeBuilderSort);

   for (int i = 0; i < numberOfRegisteredBytecodes; i++)
      {
      TR::BytecodeBuilder *builder = _registeredBytecodes.at(i);
      caseBuilders[i] = builder;
      caseValues[i] = builder->bcIndex();
      caseFallsThrough[i] = false;
      }

   Switch("opcode", doWhileBody, _defaultHandler, numberOfRegisteredBytecodes, caseValues, caseBuilders, caseFallsThrough);

   _defaultHandler->Call("handleBadOpcode", 2, _defaultHandler->Load(_opcodeName), _defaultHandler->Load(_pcName));
   _defaultHandler->Goto(&breakBody);

   handleInterpreterExit(this);

   return true;
   }

void
OMR::InterpreterBuilder::getNextOpcode(TR::IlBuilder *builder)
   {
   builder->Store(_opcodeName,
   builder->   ConvertTo(Int32,
   builder->      LoadAt(_bytecodePtrType,
   builder->         IndexAt(_bytecodePtrType,
   builder->      Load(_bytecodePtrName),
                        getPC(builder)))));
   }

void
OMR::InterpreterBuilder::setBytecodes(TR::IlBuilder *builder, TR::IlValue *value)
   {
   builder->Store(_bytecodePtrName, value);
   }

void
OMR::InterpreterBuilder::setPC(TR::IlBuilder *builder, TR::IlValue *value)
   {
   builder->Store(_pcName, value);
   }

TR::IlValue *
OMR::InterpreterBuilder::getPC(TR::IlBuilder *builder)
   {
   return builder->Load(_pcName);
   }

void
OMR::InterpreterBuilder::DoWhileLoop(const char *whileCondition, TR::BytecodeBuilder **body, TR::BytecodeBuilder **breakBuilder, TR::BytecodeBuilder **continueBuilder)
   {
   methodSymbol()->setMayHaveLoops(true);
   TR_ASSERT(body != NULL, "doWhileLoop needs to have a body");

   if (!symbolDefined(whileCondition))
      defineValue(whileCondition, Int32);

   if (*body == NULL)
      {
      *body = OrphanBytecodeBuilder(-1, "DoWhileLoopBody");
      }
   TraceIL("InterpreterBuilder[ %p ]::DoWhileLoop do body B%d while %s\n", this, (*body)->getEntry()->getNumber(), whileCondition);

   AppendBuilder(*body);
   (*body)->propagateVMState(vmState());

   TR::BytecodeBuilder *loopContinue = NULL;

   if (continueBuilder)
      {
      TR_ASSERT(*continueBuilder == NULL, "DoWhileLoop returns continueBuilder, cannot provide continueBuilder as input");
      loopContinue = *continueBuilder = OrphanBytecodeBuilder(-1, "DoWhileLoopContinue");
      }
   else
      loopContinue = OrphanBytecodeBuilder(-1, "DoWhileLoopContinue");

   AppendBuilder(loopContinue);
   loopContinue->propagateVMState(vmState());

   loopContinue->IfCmpNotEqualZero(*body,
   loopContinue->   Load(whileCondition));

   if (breakBuilder)
      {
      *breakBuilder = OrphanBytecodeBuilder(-1, "DoWhileLoopBreakBuilder");
      AppendBuilder(*breakBuilder);
      (*breakBuilder)->propagateVMState(vmState());

      }

   // make sure any subsequent operations go into their own block *after* the loop
   appendBlock();
   }

void
OMR::InterpreterBuilder::Switch(const char *selectionVar,
                  TR::BytecodeBuilder *currentBuilder,
                  TR::BytecodeBuilder *defaultBuilder,
                  uint32_t numCases,
                  int32_t *caseValues,
                  TR::BytecodeBuilder **caseBuilders,
                  bool *caseFallsThrough)
   {
   TR::IlValue *selectorValue = currentBuilder->Load(selectionVar);
   TR_ASSERT(selectorValue->getDataType() == TR::Int32, "Switch only supports selector having type Int32");

   TR::Node *defaultNode = TR::Node::createCase(0, defaultBuilder->getEntry()->getEntry());
   TR::Node *lookupNode = TR::Node::create(TR::lookup, numCases + 2, currentBuilder->loadValue(selectorValue), defaultNode);

   // get the lookup tree into this builder, even though we haven't completely filled it in yet
   currentBuilder->genTreeTop(lookupNode);
   TR::Block *switchBlock = currentBuilder->getCurrentBlock();

   // make sure no fall through edge created from the lookup
   currentBuilder->appendNoFallThroughBlock();

   TR::BytecodeBuilder *breakBuilder = OrphanBytecodeBuilder(-1, "SwitchBreakBuilder");

   // each case handler is a sequence of two builder objects: first the one passed in via caseBuilder (or will be passed
   // back via caseBuilders, and second a builder that branches to the breakBuilder (unless this case falls through)
   for (int32_t c=0;c < numCases;c++)
      {
      int32_t value = caseValues[c];
      TR::BytecodeBuilder *handler = NULL;
      TR_ASSERT(caseBuilders[c] != NULL, "Switch does not support NULL case builders");
      if (!caseFallsThrough[c])
         {
         handler = OrphanBytecodeBuilder(-1, "SwitchCaseHandler");
         handler->propagateVMState(vmState());

         handler->AppendBuilder(caseBuilders[c]);
         caseBuilders[c]->propagateVMState(vmState());

         // handle "break" with a separate builder so user can add whatever they want into caseBuilders[c]
         TR::BytecodeBuilder *branchToBreak = OrphanBytecodeBuilder(-1, "SwitchBranchToBreak");
         branchToBreak->propagateVMState(vmState());

         branchToBreak->Goto(&breakBuilder);
         handler->AppendBuilder(branchToBreak);
         branchToBreak->propagateVMState(vmState());
         }
      else
         {
         handler = caseBuilders[c];
         handler->propagateVMState(vmState());
         }

      TR::Block *caseBlock = handler->getEntry();
      cfg()->addEdge(switchBlock, caseBlock);
      currentBuilder->AppendBuilder(handler);

      TR::Node *caseNode = TR::Node::createCase(0, caseBlock->getEntry(), value);
      lookupNode->setAndIncChild(c+2, caseNode);
      }

   cfg()->addEdge(switchBlock, defaultBuilder->getEntry());
   currentBuilder->AppendBuilder(defaultBuilder);
   defaultBuilder->propagateVMState(vmState());

   currentBuilder->AppendBuilder(breakBuilder);
   breakBuilder->propagateVMState(vmState());
   }
