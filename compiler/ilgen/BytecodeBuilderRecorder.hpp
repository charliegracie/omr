/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2017, 2017
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


#ifndef BYTECODE_BUILDERRECORDER_INCL
#define BYTECODE_BUILDERRECORDER_INCL

#ifndef TR_BYTECODEBUILDERRECORDER_DEFINED
#define TR_BYTECODEBUILDERRECORDER_DEFINED
#define PUT_OMR_BYTECODEBUILDERRECORDER_INTO_TR
#endif // !defined(TR_BYTECODEBUILDERRECORDER_DEFINED)

#include "ilgen/IlBuilder.hpp"

namespace TR { class BytecodeBuilder; }
namespace TR { class BytecodeBuilderRecorder; }
namespace TR { class MethodBuilderRecorder; }
namespace OMR { class VirtualMachineState; }

namespace OMR
{

class BytecodeBuilderRecorder : public TR::IlBuilder
   {
public:
   TR_ALLOC(TR_Memory::IlGenerator)

   BytecodeBuilderRecorder(TR::MethodBuilder *methodBuilder, int32_t bcIndex, char *name=NULL);

   virtual bool isBytecodeBuilder() { return true; }

   /**
    * @brief bytecode index for this builder object
    */
   int32_t bcIndex() { return _bcIndex; }
   virtual int32_t currentByteCodeIndex() { return _bcIndex; } // override from IlGenerator

   /**
    * @brief name for this BytecodeBuilderRecorder, helpful for debug output
    */
   char *name() { return _name; }

   void AddFallThroughBuilder(TR::BytecodeBuilder *ftb);

   void AddSuccessorBuilders(uint32_t numBuilders, ...);
   void AddSuccessorBuilder(TR::BytecodeBuilder **b) { AddSuccessorBuilders(1, b); }

   OMR::VirtualMachineState *initialVMState()                { return _initialVMState; }
   OMR::VirtualMachineState *vmState()                       { return _vmState; }
   void setVMState(OMR::VirtualMachineState *vmState)        { _vmState = vmState; }

   void propagateVMState(OMR::VirtualMachineState *fromVMState);

   // The following control flow services are meant to hide the similarly named services
   // provided by the IlBuilder class. The reason these implementations exist is to
   // automatically manage the propagation of virtual machine states between bytecode
   // builders. By using these services, and AddFallthroughBuilder(), users do not have
   // to do anything to propagate VM states; it's all just taken care of under the covers.
   void Goto(TR::BytecodeBuilder **dest);
   void Goto(TR::BytecodeBuilder *dest);
   void IfCmpEqual(TR::BytecodeBuilder **dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpEqual(TR::BytecodeBuilder *dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpEqualZero(TR::BytecodeBuilder **dest, TR::IlValue *c);
   void IfCmpEqualZero(TR::BytecodeBuilder *dest, TR::IlValue *c);
   void IfCmpNotEqual(TR::BytecodeBuilder **dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpNotEqual(TR::BytecodeBuilder *dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpNotEqualZero(TR::BytecodeBuilder **dest, TR::IlValue *c);
   void IfCmpNotEqualZero(TR::BytecodeBuilder *dest, TR::IlValue *c);
   void IfCmpLessThan(TR::BytecodeBuilder **dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpLessThan(TR::BytecodeBuilder *dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpUnsignedLessThan(TR::BytecodeBuilder **dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpUnsignedLessThan(TR::BytecodeBuilder *dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpLessOrEqual(TR::BytecodeBuilder **dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpLessOrEqual(TR::BytecodeBuilder *dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpUnsignedLessOrEqual(TR::BytecodeBuilder **dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpUnsignedLessOrEqual(TR::BytecodeBuilder *dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpGreaterThan(TR::BytecodeBuilder **dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpGreaterThan(TR::BytecodeBuilder *dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpUnsignedGreaterThan(TR::BytecodeBuilder **dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpUnsignedGreaterThan(TR::BytecodeBuilder *dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpGreaterOrEqual(TR::BytecodeBuilder **dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpGreaterOrEqual(TR::BytecodeBuilder *dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpUnsignedGreaterOrEqual(TR::BytecodeBuilder **dest, TR::IlValue *v1, TR::IlValue *v2);
   void IfCmpUnsignedGreaterOrEqual(TR::BytecodeBuilder *dest, TR::IlValue *v1, TR::IlValue *v2);

protected:
   int32_t                             _bcIndex;
   char                              * _name;
   OMR::VirtualMachineState          * _initialVMState;
   OMR::VirtualMachineState          * _vmState;

   void transferVMState(TR::BytecodeBuilder **b);
   void addSuccessorBuilder(TR::BytecodeBuilder **b);
   TR::BytecodeBuilder *asBytecodeBuilder();
   };

} // namespace OMR


#if defined(PUT_OMR_BYTECODEBUILDERRECORDER_INTO_TR)

namespace TR
{
   class BytecodeBuilderRecorder : public OMR::BytecodeBuilderRecorder
      {
      public:
         BytecodeBuilderRecorder(TR::MethodBuilder *methodBuilder, int32_t bcIndex, char *name=NULL)
            : OMR::BytecodeBuilderRecorder(methodBuilder, bcIndex, name)
            { }
      };

} // namespace TR

#endif // defined(PUT_OMR_BYTECODEBUILDERRECORDER_INTO_TR)

#endif // !defined(OMR_ILBUILDERRECORDER_INCL)
