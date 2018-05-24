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
#include "ilgen/JitBuilderRecorderTextFile.hpp"


OMR::JitBuilderRecorderTextFile::JitBuilderRecorderTextFile(const TR::MethodBuilderRecorder *mb, const char *fileName)
   : TR::JitBuilderRecorder(mb), _file(fileName, std::fstream::out | std::fstream::trunc)
   {
   start();
   }

void
OMR::JitBuilderRecorderTextFile::Close()
   {
   end();
   EndStatement();
   _file.close();
   }

void
OMR::JitBuilderRecorderTextFile::String(const char * const string)
   {
   _file << "\"" << strlen(string) << " [" << string << "]\" ";
   }

void
OMR::JitBuilderRecorderTextFile::Number(int8_t num)
   {
   _file << num << " ";
   }

void
OMR::JitBuilderRecorderTextFile::Number(int16_t num)
   {
   _file << num << " ";
   }

void
OMR::JitBuilderRecorderTextFile::Number(int32_t num)
   {
   _file << num << " ";
   }

void
OMR::JitBuilderRecorderTextFile::Number(int64_t num)
   {
   _file << num << " ";
   }

void
OMR::JitBuilderRecorderTextFile::Number(float num)
   {
   _file << num << " ";
   }

void
OMR::JitBuilderRecorderTextFile::Number(double num)
   {
   _file << num << " ";
   }

void
OMR::JitBuilderRecorderTextFile::ID(TypeID id)
   {
   _file << "ID" << id << " ";
   }

void
OMR::JitBuilderRecorderTextFile::Statement(const char *s)
   {
   _file << "S" << lookupID(s) << " ";
   }

void
OMR::JitBuilderRecorderTextFile::Type(const TR::IlType *type)
   {
   _file << "T" << lookupID(type) << " ";
   }

void
OMR::JitBuilderRecorderTextFile::Value(const TR::IlValue *v)
   {
   _file << "V" << lookupID(v) << " ";
   }

void
OMR::JitBuilderRecorderTextFile::Builder(const TR::IlBuilderRecorder *b)
   {
   if (b)
      _file << "B" << lookupID(b) << " ";
   else
      _file << "Def ";
   }

void
OMR::JitBuilderRecorderTextFile::Location(const void *location)
   {
   _file << "{" << location << "} ";
   }

void
OMR::JitBuilderRecorderTextFile::EndStatement()
   {
   _file << "\n";
   }
