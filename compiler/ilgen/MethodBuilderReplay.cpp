/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2016, 2017
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 *******************************************************************************/

#include "ilgen/MethodBuilderReplay.hpp"

#include <iostream>
#include <fstream>
#include <string>

#include <stdint.h>
#include "infra/BitVector.hpp"
#include "compile/Compilation.hpp"
#include "ilgen/IlGeneratorMethodDetails_inlines.hpp"
#include "ilgen/IlInjector.hpp"
#include "ilgen/IlBuilder.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/BytecodeBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/VirtualMachineState.hpp"
//#include "ilgen/JitBuilderRecorderBinaryFile.hpp"
#include "ilgen/JitBuilderReplayTextFile.hpp"

// MethodBuilderReplay introduces recording support for the MethodBuilder
// class. It is designed as a superclass for MethodBuilder because
// MethodBuilder should be able to call out to its Recorder to enable
// diagnostic scenarios (e.g. record how a method was compiled), but the
// Recorder class should not be able to directly refer to MethodBuilder
// to enable the separation of JitBuilder API calls (via
// MethodBuilderRecorder) from the process of generating IL (via
// MethodBuilder).

OMR::MethodBuilderReplay::MethodBuilderReplay(TR::TypeDictionary *types, TR::JitBuilderReplay *replay)
   : TR::MethodBuilder(types, NULL, NULL),
   _replay(replay)
   {
   if (_replay == NULL)
      {
      // TODO handle getting the file via environment variable????
      }
   else
	   _replay->setMethodBuilderReplay(this);

   _replay->HandleMethodBuilderConstruction();
   }

bool
OMR::MethodBuilderReplay::buildIL()
   {
   _replay->HandleMethodBuilderBuildIL();
   return true;
   }
