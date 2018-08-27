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

#ifndef OMR_INTERPRETERBUILDER_INCL
#define OMR_INTERPRETERBUILDER_INCL

#include "ilgen/RuntimeBuilder.hpp"

namespace TR { class InterpreterBuilder; }
namespace TR { class BytecodeBuilder;}
namespace TR { class VirtualMachineRegister; }
namespace TR { class VirtualMachineInterpreterStack; }

namespace OMR
{

class InterpreterBuilder : public TR::RuntimeBuilder
   {
public:
   TR_ALLOC(TR_Memory::IlGenerator)

   InterpreterBuilder(TR::TypeDictionary *d, const char *bytecodePtrName, TR::IlType *bytecodeElementType, const char *pcName, const char *opcodeName);

   virtual void DefineFunctions(TR::MethodBuilder *mb) {}

   virtual void Commit(TR::BytecodeBuilder *builder);
   virtual void Reload(TR::BytecodeBuilder *builder);
   virtual TR::IlValue *GetImmediate(TR::BytecodeBuilder *builder);
   virtual void DefaultFallthroughTarget(TR::BytecodeBuilder *builder);
   virtual void SetJumpIfTarget(TR::BytecodeBuilder *builder, TR::IlValue *condition, TR::IlValue *jumpTarget);
   virtual void ReturnTarget(TR::BytecodeBuilder *builder);

   virtual bool buildIL();
   virtual void registerBytecodeBuilders() = 0;
   virtual void handleInterpreterExit(TR::IlBuilder *builder) = 0;
   virtual TR::VirtualMachineState *createVMState() {return NULL;}
   virtual void loadBytecodes(TR::IlBuilder *builder) = 0;
   virtual void loadPC(TR::IlBuilder *builder) = 0;
   virtual void savePC(TR::IlBuilder *builder, TR::IlValue *pc) = 0;

   void registerBytecodeBuilder(TR::BytecodeBuilder *handler);

protected:
   void getNextOpcode(TR::IlBuilder *builder);

   void setBytecodes(TR::IlBuilder *builder, TR::IlValue *value);
   TR::IlValue *getBytecodes(TR::IlBuilder *builder);
   void setPC(TR::IlBuilder *builder, TR::IlValue *value);
   TR::IlValue *getPC(TR::IlBuilder *builder);

   void completeBytecodeBuilderRegistration();

   void DoWhileLoop(const char *whileCondition,
               TR::BytecodeBuilder **body,
               TR::BytecodeBuilder **breakBuilder,
               TR::BytecodeBuilder **continueBuilder);
   void Switch(const char *selectionVar,
               TR::BytecodeBuilder *currentBuilder,
               TR::BytecodeBuilder *defaultBuilder,
               uint32_t numCases,
               int32_t *caseValues,
               TR::BytecodeBuilder **caseBuilders,
               bool *caseFallsThrough);
   void Switch(const char *selectionVar,
               TR::BytecodeBuilder *currentBuilder,
               TR::BytecodeBuilder *defaultBuilder,
               uint32_t numCases,
               ...);

private:
   bool _isRecursiveCall;
   const char *_bytecodePtrName;
   TR::IlType *_bytecodeElementType;
   TR::IlType *_bytecodePtrType;
   const char *_pcName;
   const char *_opcodeName;
   TR::BytecodeBuilder *_defaultHandler;
   TR::BytecodeBuilder *_opcodeBuilders[OPCODES::BC_COUNT];
   };

} // namespace OMR

#endif // !defined(OMR_INTERPRETERBUILDER_INCL)
