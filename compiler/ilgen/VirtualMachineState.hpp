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


#ifndef OMR_VIRTUALMACHINESTATE_INCL
#define OMR_VIRTUALMACHINESTATE_INCL


namespace TR { class IlBuilder; }

namespace OMR
{

// VirtualMachineState is really an interface that language compilers will implement
// to define all the aspects of the virtual machine state that the JIT compiler will
// simulate (operand stacks, virtual registers like "pc", "sp", etc.). The interface
// has four functions: 1) Commit() to cause the simulated state to be written to the
// actual virtual machine state, 2) Reload() to set the simulated machine state based
// on the current virtual machine state, 3) MakeCopy() to make an identical copy of
// the curent virtual machine state, and 4) MergeInto() to perform the actions needed
// for the current state to be merged with a second virtual machine state.
//
// Commit() is typically called when transitioning from compiled code to the interpreter.
//
// Reload() is typically called on the transition back from the interpreter to compiled
// code.
//
// MakeCopy is typically called when control flow edges are created: the current vm state
// must also become the initial vm state at the target of the control flow edge and be
// able to evolve independently from the current vm state.
//
// Merge() is typically called when one compiled code path needs to merge into a second
// compiled code path (think the bottom of an if-then-else control flow diamond). At the
// merge point, the locations of all aspects of the simulated machine state must be
// identical so that the code below the merge point will compute correct results.
// Merge() typically causes the current simulated machine state to be written to the
// locations that were used by an existing simulated machine state.
//
// Language compilers should extend this base class, add instance variables for all
// necessary virtual machine state variables (using classes like VirtualMachineRegister
// and VirtualMachineOperandStack) and then implement Commit(), Reload(), MakeCopy(),
// and Merge() to call Commit(), Reload(), MakeCopy(), and Merge(), respectively, on
// each instance variable.

class VirtualMachineState
   {
   public:

   // Commit() should cause all simulated aspects of the virtual machine
   // state to become real. Operations to commit the state should be added to
   // the provided IlBuilder "b", which is assumed to be transitioning to the
   // interpeter. Empty base implementation does nothing.
   virtual void Commit(TR::IlBuilder *b) { }

   // Reload() should cause all simulated aspects of the virtual machine
   // state to match the real virtual machine state. Operations to reload the
   // current machine state should be added to the provided IlBuilder "b", which
   // is assumed to be transitioning back to compiled code from the interpreter.
   // Empty base implementation does nothing.
   virtual void Reload(TR::IlBuilder *b) { }

   // MakeCopy() creates an identical copy of the current object, typically
   // used to provide the input state at the destination of a control flow edge
   // Any subclasses *must* implement this function since the default is just
   // to return the current object (since it has no state).
   virtual VirtualMachineState *MakeCopy() { return this; }

   // MergeInto() should cause all simulated aspects of the current virtual machine
   // state to be stored into the same locations holding the respective simulated
   // aspects of the "other" virtual machine state. Operations to accomplish this
   // merging should be added to the "b" builder, which is assumed to be a path
   // leading to the target virtual machine state. Empty base implementation does
   // nothing.
   virtual void MergeInto(OMR::VirtualMachineState *other, TR::IlBuilder *b) { }
   };
}

#endif // !defined(OMR_VIRTUALMACHINESTATE_INCL
