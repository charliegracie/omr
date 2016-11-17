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

#ifndef OMR_VIRTUALMACHINEREGISTER_INCL
#define OMR_VIRTUALMACHINEREGISTER_INCL


#include "ilgen/VirtualMachineState.hpp"
#include "ilgen/IlBuilder.hpp"

namespace TR { class IlBuilder; }
namespace TR { class IlType; }

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

class VirtualMachineRegister : public OMR::VirtualMachineState
   {
   public:
   // "type" must be pointer to the type of the register
   VirtualMachineRegister(TR::IlBuilder *b,
                          const char * const localName,
                          const TR::IlType * const type,
                          const void * const address) :
      ::OMR::VirtualMachineState(),
      _localName(localName),
      _type(type),
      _address(address)
      {
      Reload(b);
      }

   // Commit() writes the simulated register value to the actual virtual machine
   // register, typically done in preparation for transition TO the interpreter
   virtual void Commit(TR::IlBuilder *b)
      {
      b->StoreAt(
      b->   ConstAddress(_address),
      b->   Load(_localName));
      }

   // Reload() writes the actual register value to the simulated virtual machine
   // register, typically done in preparation for transition FROM the interpreter
   virtual void Reload(TR::IlBuilder *b)
      {
      b->Store(_localName,
      b->   LoadAt((TR::IlType *)_type,
      b->      ConstAddress(_address)));
      }

   // Load() returns the current value of the simulated register
   TR::IlValue *Load(TR::IlBuilder *b)
      {
      return b->Load(_localName);
      }

   // Store() writes a new value into the simulated register; the previous value is lost
   void Store(TR::IlBuilder *b, TR::IlValue *value)
      {
      b->Store(_localName, value);
      }

   private:

   const char * const _localName;
   const TR::IlType * const _type;
   const void * const _address;
   };
}

#endif // !defined(OMR_VIRTUALMACHINEREGISTER_INCL
