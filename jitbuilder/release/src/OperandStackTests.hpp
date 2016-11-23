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


#ifndef OPERANDSTACKTESTS_INCL
#define OPERANDSTACKTESTS_IMPL

#include "ilgen/MethodBuilder.hpp"

#define STACKVALUEILTYPE	Int64
#define	STACKVALUETYPE		int64_t


class OperandStackTestMethod : public TR::MethodBuilder
   {
   public:
   OperandStackTestMethod(TR::TypeDictionary *);
   virtual bool buildIL();

   static void verifyStack(const char *step, int32_t max, int32_t num, ...);
   static bool verifyUntouched(int32_t maxTouched);

   private:
   TR::IlType                      * _valueType;

   OMR::VirtualMachineOperandStack * _stack;
   OMR::VirtualMachineRegister     * _stackTop;

   static STACKVALUETYPE           * _realStack;
   static STACKVALUETYPE           * _realStackTop;
   static int32_t                    _realStackSize;
   };

#endif // !defined(OPERANDSTACKTESTS_INCL)
