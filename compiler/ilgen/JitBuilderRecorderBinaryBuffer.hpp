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


#ifndef OMR_JITBUILDERRECORDER_BINARYBUFFER_INCL
#define OMR_JITBUILDERRECORDER_BINARYBUFFER_INCL


#ifndef TR_JITBUILDERRECORDER_BINARYBUFFER_DEFINED
#define TR_JITBUILDERRECORDER_BINARYBUFFER_DEFINED
#define PUT_OMR_JITBUILDERRECORDER_BINARYBUFFER_INTO_TR
#endif


#include "ilgen/JitBuilderRecorder.hpp"
#include <vector>

namespace TR { class IlBuilderRecorder; }
namespace TR { class MethodBuilderRecorder; }
namespace TR { class IlType; }
namespace TR { class IlValue; }

namespace OMR
{

class JitBuilderRecorderBinaryBuffer : public TR::JitBuilderRecorder
   {
   public:
   JitBuilderRecorderBinaryBuffer(const TR::MethodBuilderRecorder *mb);
   virtual ~JitBuilderRecorderBinaryBuffer() { }

   virtual void Close();
   virtual void String(const char * const string);
   virtual void Number(int8_t num);
   virtual void Number(int16_t num);
   virtual void Number(int32_t num);
   virtual void Number(int64_t num);
   virtual void Number(float num);
   virtual void Number(double num);
   virtual void ID(TypeID id);
   virtual void Statement(const char *s);
   virtual void Type(const TR::IlType *type);
   virtual void Value(const TR::IlValue *v);
   virtual void Builder(const TR::IlBuilderRecorder *b);
   virtual void Location(const void * location);
   virtual void EndStatement();

   std::vector<uint8_t> & buffer() { return _buf; }

   protected:
   std::vector<uint8_t> _buf;
   };

} // namespace OMR


#if defined(PUT_OMR_JITBUILDERRECORDER_BINARYBUFFER_INTO_TR)

namespace TR
{
   class JitBuilderRecorderBinaryBuffer : public OMR::JitBuilderRecorderBinaryBuffer
      {
      public:
         JitBuilderRecorderBinaryBuffer(const TR::MethodBuilderRecorder *mb)
            : OMR::JitBuilderRecorderBinaryBuffer(mb)
            { }
         virtual ~JitBuilderRecorderBinaryBuffer()
            { }
      };

} // namespace TR

#endif // defined(PUT_OMR_JITBUILDERRECORDER_BINARYBUFFER_INTO_TR)

#endif // !defined(OMR_JITBUILDERRECORDER_BINARYBUFFER_INCL)
