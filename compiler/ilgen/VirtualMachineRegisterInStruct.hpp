/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2000, 2016
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

#ifndef OMR_VIRTUALMACHINEREGISTERINSTRUCT_INCL
#define OMR_VIRTUALMACHINEREGISTERINSTRUCT_INCL

#include "ilgen/VirtualMachineRegister.hpp"
#include "ilgen/IlBuilder.hpp"

namespace OMR
{

// VirtualMachineRegisterInStruct can be used to represent values in the virtual
// machine that are stored in a readily accessible struct (one whose address
// can be loaded via a local variable. The value does not need to be a virtual
// machine register, but often it is the registers of the virtual machine that
// are candidates for VirtualMachineRegisterInStruct. An alternative is
// VirtualMachineRegister, which can be more convenient if the virtual machine
// value is stored in a more arbitrary place or in a structure that isn't readily
// accessible inside the compiled method. 
// VirtualMachineRegisterInStruct is a subclass of VirtualMachineRegister
// The simulated register value is simply stored in a single local variable, which
// gives the compiler visibility to all changes to the register (and enables
// optimization / simplification). Because there is just a single local variable,
// the Merge() function does not need to do anything (the value is accessible from
// the same location at all locations). The Commit() and Reload() functions simply
// move the value back and forth between the local variable and the structure that
// holds the actual virtual machine state.

class VirtualMachineRegisterInStruct : public ::OMR::VirtualMachineRegister
   {
   public:
   VirtualMachineRegisterInStruct(TR::IlBuilder *b,
                          const char * const structName,
                          const char * const localNameHoldingStructAddress,
                          const char * const fieldName,
                          const char * const localNameToUse) :
      ::OMR::VirtualMachineRegister(),
      _structName(structName),
      _fieldName(fieldName),
      _localNameHoldingStructAddress(localNameHoldingStructAddress),
      _localName(localNameToUse),
      {
      _type = b->typeDictionary()->GetFieldType(structName, fieldName);
      _adjustByStep = _type->getSize();
      ReloadState(b);
      }

   // CommitState() writes the simulated register value to the actual virtual machine
   // register, typically done in preparation for transition TO the interpreter
   virtual void Commit(TR::IlBuilder *b)
      {
      b->StoreIndirect(_structName, _fieldName,
      b->   Load(_localNameHoldingStructAddress),
      b->   Load(_localName));
      }

   // ReloadState() writes the actual register value to the simulated virtual machine
   // register, typically done in preparation for transition FROM the interpreter
   virtual void Reload(TR::IlBuilder *b)
      {
      b->Store(_localNameToUse,
      b->   LoadIndirect(_structName, _fieldName,
      b->      Load(_localNameHoldingStructAddress)));
      }

   private:

   const char * const _structName;
   const char * const _fieldName;
   const char * const _localNameHoldingStructAddress;
   const char * const _name;
   };
}

#endif // !defined(OMR_VIRTUALMACHINEREGISTERINSTRUCT_INCL)
