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
 ******************************************************************************/


#ifndef OMR_METHODBUILDERREPLAY_INCL
#define OMR_METHODBUILDERREPLAY_INCL

#ifndef TR_METHODBUILDERREPLAY_DEFINED
#define TR_METHODBUILDERREPLAY_DEFINED
#define PUT_OMR_METHODBUILDERREPLAY_INTO_TR
#endif


#include "ilgen/MethodBuilder.hpp"

// Maximum length of _definingLine string (including null terminator)
#define MAX_LINE_NUM_LEN 7

class TR_BitVector;
namespace TR { class BytecodeBuilder; }
namespace TR {class JitBuilderReplay; }
namespace OMR { class VirtualMachineState; }

namespace OMR
{

class MethodBuilderReplay : public TR::MethodBuilder
   {
   public:
   TR_ALLOC(TR_Memory::IlGenerator)

   MethodBuilderReplay(TR::TypeDictionary *types, TR::JitBuilderReplay *replay);
   virtual ~MethodBuilderReplay()  { }

   virtual bool buildIL();

   protected:

   TR::JitBuilderReplay* _replay;
   };

} // namespace OMR


#if defined(PUT_OMR_METHODBUILDERREPLAY_INTO_TR)

namespace TR
{
   class MethodBuilderReplay : public OMR::MethodBuilderReplay
      {
      public:
         MethodBuilderReplay(TR::TypeDictionary *types, TR::JitBuilderReplay *replay)
            : OMR::MethodBuilderReplay(types, replay)
            { }
      };

} // namespace TR

#endif // defined(PUT_OMR_METHODBUILDERREPLAY_INTO_TR)

#endif // !defined(OMR_METHODBUILDERREPLAY_INCL)
