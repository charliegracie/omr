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
#include <fstream>

#include "ilgen/JitBuilderRecorderBinaryFile.hpp"
#include "infra/Assert.hpp"


OMR::JitBuilderRecorderBinaryFile::JitBuilderRecorderBinaryFile(const TR::MethodBuilderRecorder *mb, const char *fileName)
   : TR::JitBuilderRecorderBinaryBuffer(mb), _file(fileName, std::fstream::out | std::fstream::app)
   {
   }

void
OMR::JitBuilderRecorderBinaryFile::Close()
   {
   end();
   EndStatement();

   _file.write(reinterpret_cast<const char *>(&_buf[0]), _buf.size());

   _file.close();
   }
