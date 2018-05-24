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

#include <stdint.h>
#include <iostream>
#include <fstream>

#include "infra/Assert.hpp"
#include "ilgen/JitBuilderReplay.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/JitBuilderRecorder.hpp"


OMR::JitBuilderReplay::JitBuilderReplay(const TR::MethodBuilder *mb)
   : _mb(mb), _nextID(0), _idSize(8)
   {
   }

OMR::JitBuilderReplay::~JitBuilderReplay()
   {
   }

void
OMR::JitBuilderReplay::start()
   {
   VerifyRecordedType();
   }

bool
OMR::JitBuilderReplay::knownID(const void *ptr)
   {
   return true;
   }
   
OMR::JitBuilderReplay::TypeID
OMR::JitBuilderReplay::lookupID(const void *ptr)
   {
   return 0;
   }

void
OMR::JitBuilderReplay::end()
   {

   }
