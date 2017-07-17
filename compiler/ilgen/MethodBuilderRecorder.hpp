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


#ifndef OMR_METHODBUILDERRECORDER_INCL
#define OMR_METHODBUILDERRECORDER_INCL

#ifndef TR_METHODBUILDERRECORDER_DEFINED
#define TR_METHODBUILDERRECORDER_DEFINED
#define PUT_OMR_METHODBUILDERRECORDER_INTO_TR
#endif


#include "ilgen/IlBuilder.hpp"

// Maximum length of _definingLine string (including null terminator)
#define MAX_LINE_NUM_LEN 7

class TR_BitVector;
namespace TR { class BytecodeBuilder; }
namespace OMR { class VirtualMachineState; }

namespace OMR
{

class MethodBuilderRecorder : public TR::IlBuilder
   {
   public:
   TR_ALLOC(TR_Memory::IlGenerator)

   MethodBuilderRecorder(TR::TypeDictionary *types, TR::JitBuilderRecorder *recorder, OMR::VirtualMachineState *vmState);
   virtual ~MethodBuilderRecorder()                          { }

   int32_t getNextValueID()                                  { return _nextValueID++; }

   bool usesBytecodeBuilders()                               { return _useBytecodeBuilders; }
   void setUseBytecodeBuilders()                             { _useBytecodeBuilders = true; }

   OMR::VirtualMachineState *vmState()                       { return _vmState; }
   void setVMState(OMR::VirtualMachineState *vmState)        { _vmState = vmState; }

   void AllLocalsHaveBeenDefined();

   void AppendBuilder(TR::IlBuilder *b)                     { TR::IlBuilder::AppendBuilder(b); }

   /**
    * @brief append a bytecode builder object to this method
    * @param builder the bytecode builder object to add, usually for bytecode index 0
    * A side effect of this call is that the builder is added to the worklist so that
    * all other bytecodes can be processed by asking for GetNextBytecodeFromWorklist()
    */
   void AppendBuilder(TR::BytecodeBuilder *bb);

   void DefineFile(const char *file);
   void DefineLine(const char *line);
   void DefineLine(int32_t line);
   void DefineName(const char *name);
   void DefineLocal(const char *name, TR::IlType *dt);
   void DefineMemory(const char *name, TR::IlType *dt, void *location);
   void DefineParameter(const char *name, TR::IlType *type);
   void DefineArrayParameter(const char *name, TR::IlType *dt);
   void DefineReturnType(TR::IlType *dt);
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

   /**
    * @brief add a bytecode builder to the worklist
    * @param bcBuilder is the bytecode builder whose bytecode index will be added to the worklist
    */
   void addBytecodeBuilderToWorklist(TR::BytecodeBuilder *bcBuilder);

   /**
    * @brief get lowest index bytecode from the worklist
    * @returns lowest bytecode index or -1 if worklist is empty
    * It is important to use the worklist because it guarantees no bytecode will be
    * processed before at least one predecessor bytecode has been processed, which
    * means there should be a non-NULL VirtualMachineState object on the associated
    * BytecodeBuilder object.
    */
   int32_t GetNextBytecodeFromWorklist();
   
   void addToAllBytecodeBuildersList(TR::BytecodeBuilder *bcBuilder);

   TR::JitBuilderRecorder *recorder() const { return _recorder; }
   void setRecorder(TR::JitBuilderRecorder *recorder) { _recorder = recorder; }

   protected:
   int32_t                          _nextValueID;

   bool                             _useBytecodeBuilders;
   OMR::VirtualMachineState       * _vmState;

   TR_BitVector                   * _bytecodeWorklist;
   TR_BitVector                   * _bytecodeHasBeenInWorklist;

   TR::JitBuilderRecorder         * _recorder;

   TR::MethodBuilder *asMethodBuilder();
   };

} // namespace OMR


#if defined(PUT_OMR_METHODBUILDERRECORDER_INTO_TR)

namespace TR
{
   class MethodBuilderRecorder : public OMR::MethodBuilderRecorder
      {
      public:
         MethodBuilderRecorder(TR::TypeDictionary *types, TR::JitBuilderRecorder *recorder)
            : OMR::MethodBuilderRecorder(types, recorder, NULL)
            { }
         MethodBuilderRecorder(TR::TypeDictionary *types, TR::JitBuilderRecorder *recorder=NULL, OMR::VirtualMachineState *vmState=NULL)
            : OMR::MethodBuilderRecorder(types, recorder, vmState)
            { }
      };

} // namespace TR

#endif // defined(PUT_OMR_METHODBUILDERRECORDER_INTO_TR)

#endif // !defined(OMR_METHODBUILDERRECORDER_INCL)
