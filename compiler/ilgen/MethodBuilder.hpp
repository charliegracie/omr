/*******************************************************************************
 * Copyright (c) 2016, 2016 IBM Corp. and others
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

#ifndef OMR_METHODBUILDER_INCL
#define OMR_METHODBUILDER_INCL


#ifndef TR_METHODBUILDER_DEFINED
#define TR_METHODBUILDER_DEFINED
#define PUT_OMR_METHODBUILDER_INTO_TR
#endif


#include <map>
#include <set>
#include <fstream>
#include "env/TypedAllocator.hpp"
#include "ilgen/MethodBuilderRecorder.hpp"

// Maximum length of _definingLine string (including null terminator)
#define MAX_LINE_NUM_LEN 7

class TR_BitVector;
namespace TR { class IlBuilder; }
namespace TR { class BytecodeBuilder; }
namespace TR { class ResolvedMethod; }
namespace TR { class SymbolReference; }
namespace TR { class JitBuilderRecorder; }
namespace OMR { class VirtualMachineState; }

namespace TR { class SegmentProvider; }
namespace TR { class Region; }
class TR_Memory;

namespace OMR
{

class MethodBuilder : public TR::MethodBuilderRecorder
   {
   public:
   TR_ALLOC(TR_Memory::IlGenerator)

   MethodBuilder(TR::TypeDictionary *types, TR::JitBuilderRecorder *recorder, OMR::VirtualMachineState *vmState);
   MethodBuilder(const MethodBuilder &src);
   virtual ~MethodBuilder();

   virtual void setupForBuildIL();

   virtual bool injectIL();

   void addToTreeConnectingWorklist(TR::BytecodeBuilder *builder);
   void addToBlockCountingWorklist(TR::BytecodeBuilder *builder);

   virtual bool isMethodBuilder()                            { return true; }

   const char *getDefiningFile()                             { return _definingFile; }
   const char *getDefiningLine()                             { return _definingLine; }

   const char *getMethodName()                               { return _methodName; }
   void AllLocalsHaveBeenDefined();

   TR::IlType *getReturnType()                               { return _returnType; }
   int32_t getNumParameters()                                { return _numParameters; }
   const char *getSymbolName(int32_t slot);

   TR::IlType **getParameterTypes();
   char *getSignature(int32_t numParams, TR::IlType **paramTypeArray);
   char *getSignature(TR::IlType **paramTypeArray)
      {
      return getSignature(_numParameters, paramTypeArray);
      }

   TR::SymbolReference *lookupSymbol(const char *name);
   void defineSymbol(const char *name, TR::SymbolReference *v);
   bool symbolDefined(const char *name);
   bool isSymbolAnArray(const char * name);

   TR::ResolvedMethod *lookupFunction(const char *name);

   TR::BytecodeBuilder *OrphanBytecodeBuilder(int32_t bcIndex=0, char *name=NULL);

   // we can't use "using" on all platforms yet, so help out the compiler overloading explicitly
   void AppendBuilder(TR::IlBuilder *b)                     { TR::MethodBuilderRecorder::AppendBuilder(b); }
   void AppendBuilder(TR::BytecodeBuilder *bcb)             { TR::MethodBuilderRecorder::AppendBuilder(bcb); }

   void DefineFile(const char *file);

   void DefineLine(const char *line);
   void DefineLine(int32_t line);

   void DefineName(const char *name);
   void DefineParameter(const char *name, TR::IlType *type);
   void DefineArrayParameter(const char *name, TR::IlType *dt);
   void DefineReturnType(TR::IlType *dt);
   void DefineLocal(const char *name, TR::IlType *dt);
   void DefineMemory(const char *name, TR::IlType *dt, void *location);
   void DefineFunction(const char* const name,
                       const char* const fileName,
                       const char* const lineNumber,
                       void           * entryPoint,
                       TR::IlType     * returnType,
                       int32_t          numParms,
                       ...);
   void DefineFunction(const char* const name,
                       const char* const fileName,
                       const char* const lineNumber,
                       void           * entryPoint,
                       TR::IlType     * returnType,
                       int32_t          numParms,
                       TR::IlType     ** parmTypes);

   /**
    * @brief will be called if a Call is issued to a function that has not yet been defined, provides a
    *        mechanism for MethodBuilder subclasses to provide method lookup on demand rather than all up
    *        front via the constructor.
    * @returns true if the function was found and DefineFunction has been called for it, otherwise false
    */
   virtual bool RequestFunction(const char *name) { return false; }

   void addToAllBytecodeBuildersList(TR::BytecodeBuilder* bcBuilder);

   protected:
   virtual uint32_t countBlocks();
   virtual bool connectTrees();
   TR::MethodBuilder *self();

   private:
   TR::SegmentProvider *_segmentProvider;
   TR::Region *_memoryRegion;
   TR_Memory *_trMemory;

   // These values are typically defined outside of a compilation
   const char                * _methodName;
   TR::IlType                * _returnType;
   int32_t                     _numParameters;

   typedef bool (*StrComparator)(const char *, const char*);

   typedef TR::typed_allocator<std::pair<const char * const, TR::SymbolReference *>, TR::Region &> SymbolMapAllocator;
   typedef std::map<const char *, TR::SymbolReference *, StrComparator, SymbolMapAllocator> SymbolMap;

   // This map should only be accessed inside a compilation via lookupSymbol
   SymbolMap                   _symbols;

   typedef TR::typed_allocator<std::pair<const char * const, int32_t>, TR::Region &> ParameterMapAllocator;
   typedef std::map<const char *, int32_t, StrComparator, ParameterMapAllocator> ParameterMap;
   ParameterMap                _parameterSlot;

   typedef TR::typed_allocator<std::pair<const char * const, TR::IlType *>, TR::Region &> SymbolTypeMapAllocator;
   typedef std::map<const char *, TR::IlType *, StrComparator, SymbolTypeMapAllocator> SymbolTypeMap;
   SymbolTypeMap               _symbolTypes;

   typedef TR::typed_allocator<std::pair<int32_t const, const char *>, TR::Region &> SlotToSymNameMapAllocator;
   typedef std::map<int32_t, const char *, std::less<int32_t>, SlotToSymNameMapAllocator> SlotToSymNameMap;
   SlotToSymNameMap            _symbolNameFromSlot;
   
   typedef TR::typed_allocator<const char *, TR::Region &> StringSetAllocator;
   typedef std::set<const char *, StrComparator, StringSetAllocator> ArrayIdentifierSet;

   // This set acts as an identifier for symbols which correspond to arrays
   ArrayIdentifierSet          _symbolIsArray;

   typedef TR::typed_allocator<std::pair<const char * const, void *>, TR::Region &> MemoryLocationMapAllocator;
   typedef std::map<const char *, void *, StrComparator, MemoryLocationMapAllocator> MemoryLocationMap;
   MemoryLocationMap           _memoryLocations;

   typedef TR::typed_allocator<std::pair<const char * const, TR::ResolvedMethod *>, TR::Region &> FunctionMapAllocator;
   typedef std::map<const char *, TR::ResolvedMethod *, StrComparator, FunctionMapAllocator> FunctionMap;
   FunctionMap                 _functions;

   TR::IlType                ** _cachedParameterTypes;
   const char                * _definingFile;
   char                        _definingLine[MAX_LINE_NUM_LEN];
   TR::IlType                * _cachedParameterTypesArray[10];

   bool                        _newSymbolsAreTemps;

   List<TR::BytecodeBuilder> * _allBytecodeBuilders;

   uint32_t                    _numBlocksBeforeWorklist;
   List<TR::BytecodeBuilder> * _countBlocksWorklist;
   List<TR::BytecodeBuilder> * _connectTreesWorklist;
   };

} // namespace OMR


#if defined(PUT_OMR_METHODBUILDER_INTO_TR)

namespace TR
{
   class MethodBuilder : public OMR::MethodBuilder
      {
      public:
         MethodBuilder(TR::TypeDictionary *types, TR::JitBuilderRecorder *recorder=NULL, OMR::VirtualMachineState *vmState=NULL)
            : OMR::MethodBuilder(types, recorder, vmState)
            { }
      };

} // namespace TR

#endif // defined(PUT_OMR_METHODBUILDER_INTO_TR)

#endif // !defined(OMR_METHODBUILDER_INCL)
