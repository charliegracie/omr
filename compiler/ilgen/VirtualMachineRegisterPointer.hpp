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

#ifndef OMR_VIRTUALMACHINEREGISTERPOINTER_INCL
#define OMR_VIRTUALMACHINEREGISTERPOINTER_INCL


#include "ilgen/VirtualMachineRegister.hpp"

namespace OMR
{

// VirtualMachineRegister can be used to represent values in the virtual machine
// at any address. The value does not need to be a virtual machine register, but
// often it is the registers of the virtual machine that are candidates for
// VirtualMachineRegister. An alternative is VirtualMachineRegisterInStruct,
// which may be more convenient if the virtual machine value is stored in a
// structure that the compiled method has easy access to (for example, if the
// base address of the struct is a parameter of every compiled method).
// VirtualMachineRegister must implement the same API as VirtualMachineState.
// The simulated register value is simply stored in a single local variable, which
// gives the compiler visibility to all changes to the register (and enables
// optimization / simplification). Because there is just a single local variable,
// the CopyState() and MergeInto() functions do not need to do anything (the
// value is accessible from the same variable at all locations). The Commit()
// and Reload() functions simply move the value back and forth between the
// local variable and the structure that holds the actual virtual machine state.
// VirtualMachineRegister provides two additional functions:
// Load() will load the *simulated* value of the register for use in the builder "b"
// Store() will store the provided "value" into the *simulated* register by
// appending to the builder "b"

class VirtualMachineRegisterPointer : public OMR::VirtualMachineRegister
   {
   public:
   // "type" must be pointer to the type of the register
   VirtualMachineRegisterPointer(TR::IlBuilder *b,
                                 const char * const localName,
                                 TR::IlType * type,
                                 const void * const address) :
      ::OMR::VirtualMachineRegister(b, localName, type, address)
      { }

   // Adjust() with a TR::IlValue amount adds to the simulated register multiplied by the
   // size of the type
   virtual void Adjust(TR::IlBuilder *b, TR::IlValue *amount)
      {
      TR::IlType *elementType = _type->baseType();
      b->Store(_localName,
      b->   Add(
      b->      Load(_localName),
      b->      Mul(
      b->         ConstInteger(elementType, elementType->getSize()),
                  amount)));
      }
   // Adjust() with a constant amount adds the constant to the simulated register multiplied
   // by the size of the type
   virtual void Adjust(TR::IlBuilder *b, int64_t amount)
      {
      TR::IlType *elementType = _type->baseType();
      b->Store(_localName,
      b->   Add(
      b->      Load(_localName),
      b->      ConstInteger(elementType, amount * elementType->getSize())));
      }
   };
}

#endif // !defined(OMR_VIRTUALMACHINEREGISTERPOINTER_INCL)
