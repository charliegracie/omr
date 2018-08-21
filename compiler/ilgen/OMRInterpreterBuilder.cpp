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
#include "ilgen/VirtualMachineInterpreterStack.hpp"

#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

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
   _bytecodePtrName(bytecodePtrName),
   _bytecodeElementType(bytecodeElementType),
   _bytecodePtrType(NULL),
   _pcName(pcName),
   _opcodeName(opcodeName),
   _defaultHandler(NULL)
   {
   _bytecodePtrType = d->PointerTo(_bytecodeElementType);

   DefineFunction((char *)"handleBadOpcode", (char *)__FILE__, (char *)HANDLEBADEOPCODE_LINE, (void *)&handleBadOpcode, NoType, 2, Int32, Int32);

   for (int i = 0; i < OPCODES::BC_COUNT; i ++)
      {
      _opcodeBuilders[i] = NULL;
      }
   }

TR::IlValue *
OMR::InterpreterBuilder::GetImmediate(TR::BytecodeBuilder *builder, int32_t pcOffset)
   {
   TR::IlValue *immediate =
   builder->LoadAt(_bytecodePtrType,
   builder->   IndexAt(_bytecodePtrType,
   builder->      Load(_bytecodePtrName),
   builder->      Add(
   builder->         Load(_pcName),
   builder->         ConstInt32(pcOffset))));

   return immediate;
   }

void
OMR::InterpreterBuilder::SetJumpTarget(TR::BytecodeBuilder *builder, TR::IlValue *condition, TR::IlValue *jumpTarget)
   {
   TR::IlBuilder *doJump = NULL;
   builder->IfThen(&doJump, condition);

   doJump->Store("pc", doJump->ConvertTo(Int32, jumpTarget));
   }

void
OMR::InterpreterBuilder::registerBytecodeBuilder(TR::BytecodeBuilder *handler, int32_t opcodeLength)
   {
   TR_ASSERT(handler != NULL, "Can not register a NULL bytecodeBuilder");

   int32_t index = handler->bcIndex();
   TR_ASSERT(index < OPCODES::BC_COUNT, "Can not register an index (%d) larger than %d", index, OPCODES::BC_COUNT);

   _opcodeBuilders[index] = handler;
   _opcodeLengths[index] = opcodeLength;

   handler->propagateVMState(vmState());

   handler->execute();
   }

bool
OMR::InterpreterBuilder::buildIL()
   {
   cout << "InterpreterBuilder::buildIL() running!\n";

   setVMState(createVMState());

   _defaultHandler = OrphanBytecodeBuilder(OPCODES::BC_COUNT + 1, "default handler");

   loadOpcodeArray();

   setPC(this, 0);

   Store("exitLoop",
      EqualTo(
         ConstInt32(1),
         ConstInt32(1)));

   TR::BytecodeBuilder *doWhileBody = NULL;
   TR::BytecodeBuilder *breakBody = NULL;
   DoWhileLoop("exitLoop", &doWhileBody, &breakBody, NULL);

   getNextOpcode(doWhileBody);

   registerBytecodeBuilders();
   completeBytecodeBuilderRegistration();

   Switch("opcode", doWhileBody, _defaultHandler, OPCODES::BC_COUNT,
                   OPCODES::BC_00, &_opcodeBuilders[OPCODES::BC_00], false,
                   OPCODES::BC_01, &_opcodeBuilders[OPCODES::BC_01], false,
                   OPCODES::BC_02, &_opcodeBuilders[OPCODES::BC_02], false,
                   OPCODES::BC_03, &_opcodeBuilders[OPCODES::BC_03], false,
                   OPCODES::BC_04, &_opcodeBuilders[OPCODES::BC_04], false,
                   OPCODES::BC_05, &_opcodeBuilders[OPCODES::BC_05], false,
                   OPCODES::BC_06, &_opcodeBuilders[OPCODES::BC_06], false,
                   OPCODES::BC_07, &_opcodeBuilders[OPCODES::BC_07], false,
                   OPCODES::BC_08, &_opcodeBuilders[OPCODES::BC_08], false,
                   OPCODES::BC_09, &_opcodeBuilders[OPCODES::BC_09], false,
                   OPCODES::BC_10, &_opcodeBuilders[OPCODES::BC_10], false,
                   OPCODES::BC_11, &_opcodeBuilders[OPCODES::BC_11], false,
                   OPCODES::BC_12, &_opcodeBuilders[OPCODES::BC_12], false,
                   OPCODES::BC_13, &_opcodeBuilders[OPCODES::BC_13], false,
                   OPCODES::BC_14, &_opcodeBuilders[OPCODES::BC_14], false,
                   OPCODES::BC_15, &_opcodeBuilders[OPCODES::BC_15], false
                   );

   _defaultHandler->Call("handleBadOpcode", 2, _defaultHandler->Load("opcode"), _defaultHandler->Load("pc"));
   _defaultHandler->Goto(&breakBody);

   incrementPC(doWhileBody, 2);

   handleInterpreterExit(this);

   return true;
   }

void
OMR::InterpreterBuilder::getNextOpcode(TR::IlBuilder *builder)
   {
   builder->Store("opcode",
   builder->   ConvertTo(Int32,
   builder->      LoadAt(_bytecodePtrType,
   builder->         IndexAt(_bytecodePtrType,
   builder->            Load("bytecodes"),
                        getPC(builder)))));
   }

void
OMR::InterpreterBuilder::setPC(TR::IlBuilder *builder, int32_t value)
   {
   setPC(builder, builder->ConstInt32(value));
   }

void
OMR::InterpreterBuilder::setPC(TR::IlBuilder *builder, TR::IlValue *value)
   {
   builder->Store("pc", value);
   }

void
OMR::InterpreterBuilder::incrementPC(TR::IlBuilder *builder, int32_t increment)
   {
   TR::IlValue *pc = getPC(builder);
   TR::IlValue *incrementValue = builder->Load("_pcIncrementAmount_");

   pc = builder->Add(pc, incrementValue);

   setPC(builder, pc);
   }

TR::IlValue *
OMR::InterpreterBuilder::getPC(TR::IlBuilder *builder)
   {
   return builder->Load("pc");
   }

void
OMR::InterpreterBuilder::completeBytecodeBuilderRegistration()
   {
   for (int i = 0; i < OPCODES::BC_COUNT; i ++)
      {
      if (NULL == _opcodeBuilders[i])
         {
         _opcodeBuilders[i] = OrphanBytecodeBuilder(i, "Unknown opcode");
         _opcodeBuilders[i]->propagateVMState(vmState());
         _opcodeBuilders[i]->Goto(_defaultHandler);
         }
      else
         {
         _opcodeBuilders[i]->Store("_pcIncrementAmount_", _opcodeBuilders[i]->ConstInt32(_opcodeLengths[i]));
         }
      }
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
                  ...)
   {
   int32_t *caseValues = (int32_t *) _comp->trMemory()->allocateHeapMemory(numCases * sizeof(int32_t));
   TR_ASSERT(0 != caseValues, "out of memory");

   TR::BytecodeBuilder **caseBuilders = (TR::BytecodeBuilder **) _comp->trMemory()->allocateHeapMemory(numCases * sizeof(TR::BytecodeBuilder *));
   TR_ASSERT(0 != caseBuilders, "out of memory");

   bool *caseFallsThrough = (bool *) _comp->trMemory()->allocateHeapMemory(numCases * sizeof(bool));
   TR_ASSERT(0 != caseFallsThrough, "out of memory");

   va_list cases;
   va_start(cases, numCases);
   for (int32_t c=0;c < numCases;c++)
      {
      caseValues[c] = (int32_t) va_arg(cases, int);
      caseBuilders[c] = *(TR::BytecodeBuilder **) va_arg(cases, TR::BytecodeBuilder **);
      caseFallsThrough[c] = (bool) va_arg(cases, int);
      }
   va_end(cases);

   Switch(selectionVar, currentBuilder, defaultBuilder, numCases, caseValues, caseBuilders, caseFallsThrough);

   // if Switch created any new builders, we need to put those back into the arguments passed into this Switch call
   va_start(cases, numCases);
   for (int32_t c=0;c < numCases;c++)
      {
      int throwawayValue = va_arg(cases, int);
      TR::BytecodeBuilder **caseBuilder = va_arg(cases, TR::BytecodeBuilder **);
      (*caseBuilder) = caseBuilders[c];
      int throwAwayFallsThrough = va_arg(cases, int);
      }
   va_end(cases);
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
   //   back via caseBuilders, and second a builder that branches to the breakBuilder (unless this case falls through)
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
