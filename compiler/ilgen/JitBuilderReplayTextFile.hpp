/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2016, 2016
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
 ******************************************************************************/


#ifndef OMR_JITBUILDERREPLAY_TEXTFILE_INCL
#define OMR_JITBUILDERREPLAY_TEXTFILE_INCL


#ifndef TR_JITBUILDERREPLAY_TEXTFILE_DEFINED
#define TR_JITBUILDERREPLAY_TEXTFILE_DEFINED
#define PUT_OMR_JITBUILDERREPLAY_TEXTFILE_INTO_TR
#endif


#include "ilgen/JitBuilderReplay.hpp"

#include <iostream>
#include <fstream>
#include <map>

namespace TR { class IlBuilderReplay; }
namespace TR { class IlBuilder; }
namespace TR { class MethodBuilder; }
namespace TR { class IlType; }
namespace TR { class IlValue; }

namespace OMR
{

class JitBuilderReplayTextFile : public TR::JitBuilderReplay
   {
   public:
   JitBuilderReplayTextFile(const TR::MethodBuilder *mb, const char *fileName);
   virtual ~JitBuilderReplayTextFile() { }

   virtual void Close();
   virtual void VerifyRecordedType();
   virtual void HandleMethodBuilderConstruction();
   virtual void HandleMethodBuilderBuildIL();

   private:
   OMR::JitBuilderReplay::TypeID GetIDFromToken(std::string token);
   const char * GetStringFromToken(std::string token);
   const char * getStatementFromString(std::string statementString);
   void mapTypeIDToPointer(OMR::JitBuilderReplay::TypeID id, const void * pointer);
   TR::IlType *getTypeFromTypeID(OMR::JitBuilderReplay::TypeID id);
   TR::IlValue *getValueFromTypeID(OMR::JitBuilderReplay::TypeID id);
   TR::IlBuilder *getBuilderFromTypeID(OMR::JitBuilderReplay::TypeID id);
   const char *getCharsFromTypeID(OMR::JitBuilderReplay::TypeID id);

   std::fstream _file;
   };

} // namespace OMR


#if defined(PUT_OMR_JITBUILDERREPLAY_TEXTFILE_INTO_TR)

namespace TR
{
   class JitBuilderReplayTextFile : public OMR::JitBuilderReplayTextFile
      {
      public:
         JitBuilderReplayTextFile(const TR::MethodBuilder *mb, const char *fileName)
            : OMR::JitBuilderReplayTextFile(mb, fileName)
            { }
         virtual ~JitBuilderReplayTextFile()
            { }
      };

} // namespace TR

#endif // defined(PUT_OMR_JITBUILDERREPLAY_TEXTFILE_INTO_TR)

#endif // !defined(OMR_JITBUILDERREPLAY_TEXTFILE_INCL)
