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
#include "ilgen/OpcodeBuilder.hpp"
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
   : TR::MethodBuilder(d),
   _stack(NULL),
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

TR::OpcodeBuilder *
OMR::InterpreterBuilder::OrphanOpcodeBuilder(int32_t bcIndex, char *name)
   {
   TR::OpcodeBuilder *orphan = new (comp()->trHeapMemory()) TR::OpcodeBuilder(_methodBuilder, bcIndex, name);
   orphan->initialize(_details, _methodSymbol, _fe, _symRefTab);
   orphan->setupForBuildIL();
   return orphan;
   }

void
OMR::InterpreterBuilder::registerOpcodeBuilder(TR::OpcodeBuilder *handler)
   {
   int32_t index = handler->bcIndex();
   _opcodeBuilders[index] = handler;

   handler->execute();
   }

bool
OMR::InterpreterBuilder::buildIL()
   {
   cout << "InterpreterBuilder::buildIL() running!\n";

   _stack = createStack();

   _defaultHandler = OrphanOpcodeBuilder(OPCODES::BC_COUNT + 1, "default handler");

   TR::IlBuilder *doWhileBody = NULL;
   TR::IlBuilder *breakBody = NULL;

   setPC(this, 0);

   Store("exitLoop", EqualTo(ConstInt32(1), ConstInt32(1)));
   DoWhileLoopWithBreak("exitLoop", &doWhileBody, &breakBody);

   getNextOpcode(doWhileBody);

   handleOpcodes();
   handleUnusedOpcodes();

   doWhileBody->Switch("opcode", &_defaultHandler, OPCODES::BC_COUNT,
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

   handleReturn();

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
   TR::IlValue *incrementValue = builder->ConstInt32(increment);

   pc = builder->Add(pc, incrementValue);

   setPC(builder, pc);
   }

TR::IlValue *
OMR::InterpreterBuilder::getPC(TR::IlBuilder *builder)
   {
   return builder->Load("pc");
   }

void
OMR::InterpreterBuilder::handleUnusedOpcodes()
   {
   for (int i = 0; i < OPCODES::BC_COUNT; i ++)
      {
      if (NULL == _opcodeBuilders[i])
         {
         _opcodeBuilders[i] = OrphanOpcodeBuilder(i, "Unknown opcode");
         _opcodeBuilders[i]->Goto(_defaultHandler);
         }
      }
   }
