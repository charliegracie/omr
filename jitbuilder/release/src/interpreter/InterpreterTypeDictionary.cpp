/*******************************************************************************
 * Copyright (c) 2016, 2018 IBM Corp. and others
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

#include "InterpreterTypeDictionary.hpp"
#include "InterpreterTypes.h"

InterpreterTypeDictionary::InterpreterTypeDictionary()
   : TypeDictionary()
   {
   _interpreterTypes.bytecodes = Int8;
   _interpreterTypes.pBytecodes = PointerTo(_interpreterTypes.bytecodes);

   _interpreterTypes.value = STACKVALUEILTYPE;
   _interpreterTypes.pValue = PointerTo(STACKVALUEILTYPE);

   _interpreterTypes.method = DefineStruct("Method");
   _interpreterTypes.pMethod = PointerTo(_interpreterTypes.method);
   DefineField("Method", "callsUntilJit", Int32, offsetof(Method, callsUntilJit));
   DefineField("Method", "bytecodes", _interpreterTypes.pBytecodes, offsetof(Method, bytecodes));
   DefineField("Method", "compiledMethod", Address, offsetof(Method, compiledMethod));
   CloseStruct("Method");

   _interpreterTypes.frame = DefineStruct("Frame");
   _interpreterTypes.pFrame = PointerTo(_interpreterTypes.frame);
   DefineField("Frame", "previous", _interpreterTypes.pFrame, offsetof(Frame, previous));
   DefineField("Frame", "savedPC", Int32, offsetof(Frame, savedPC));
   DefineField("Frame", "bytecodes", _interpreterTypes.pBytecodes, offsetof(Frame, bytecodes));
   DefineField("Frame", "frameType", Int32, offsetof(Frame, frameType));
   DefineField("Frame", "args", _interpreterTypes.pValue, offsetof(Frame, args));
   DefineField("Frame", "locals", _interpreterTypes.pValue, offsetof(Frame, locals));
   DefineField("Frame", "sp", _interpreterTypes.pValue, offsetof(Frame, sp));
   CloseStruct("Frame");

   _interpreterTypes.interpreter = DefineStruct("Interpreter");
   _interpreterTypes.pInterpreter = PointerTo(_interpreterTypes.interpreter);
   DefineField("Interpreter", "currentFrame", _interpreterTypes.pFrame, offsetof(Interpreter, currentFrame));
   DefineField("Interpreter", "methods", _interpreterTypes.pMethod, offsetof(Interpreter, methods));
   CloseStruct("Interpreter");
   }
