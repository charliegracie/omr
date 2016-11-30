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


#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdarg.h>

#include "Jit.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/VirtualMachineOperandStack.hpp"
#include "ilgen/VirtualMachineRegister.hpp"
#include "ilgen/VirtualMachineRegisterInStruct.hpp"
#include "OperandStackTests.hpp"

using std::cout;
using std::cerr;

typedef struct Thread
   {
   int pad;
   STACKVALUETYPE *sp;
   } Thread;


static STACKVALUETYPE **verifySP = NULL;

int
main(int argc, char *argv[])
   {
   cout << "Step 1: initialize JIT\n";
   bool initialized = initializeJit();
   if (!initialized)
      {
      cerr << "FAIL: could not initialize JIT\n";
      exit(-1);
      }

   cout << "Step 2: compile operand stack tests using a straight pointer\n";
   TR::TypeDictionary types2;
   OperandStackTestMethod pointerMethod(&types2);
   uint8_t *entry2 = 0;
   int32_t rc2 = compileMethodBuilder(&pointerMethod, &entry2);
   if (rc2 != 0)
      {
      cerr << "FAIL: compilation error " << rc2 << "\n";
      exit(-2);
      }

   cout << "Step 3: invoke compiled code and print results\n";
   typedef void (OperandStackTestMethodFunction)();
   OperandStackTestMethodFunction *ptrTest = (OperandStackTestMethodFunction *) entry2;
   verifySP = pointerMethod.getSPPtr();
   ptrTest();

   cout << "Step 4: compile operand stack tests using a Thread structure\n";
   TR::TypeDictionary types4;
   OperandStackTestUsingStructMethod threadMethod(&types4);
   uint8_t *entry4 = 0;
   int32_t rc4 = compileMethodBuilder(&threadMethod, &entry4);
   if (rc4 != 0)
      {
      cerr << "FAIL: compilation error " << rc4 << "\n";
      exit(-2);
      }

   cout << "Step 5: invoke compiled code and print results\n";
   typedef void (OperandStackTestUsingStructMethodFunction)(Thread *thread);
   OperandStackTestUsingStructMethodFunction *threadTest = (OperandStackTestUsingStructMethodFunction *) entry4;

   Thread thread;
   thread.sp = threadMethod.getSP();

   verifySP = &thread.sp;
   threadTest(&thread);

   cout << "Step 6: shutdown JIT\n";
   shutdownJit();
   }


STACKVALUETYPE *OperandStackTestMethod::_realStack = NULL;
STACKVALUETYPE *OperandStackTestMethod::_realStackTop = _realStack;
int32_t OperandStackTestMethod::_realStackSize = -1;

#define REPORT1(c,n,v)         { if (c) cout << "Pass\n"; else cout << "Fail: " << (n) << " is " << (v) << "\n"; }
#define REPORT2(c,n1,v1,n2,v2) { if (c) cout << "Pass\n"; else cout << "Fail: " << (n1) << " is " << (v1) << ", " << (n2) << " is " << (v2) << "\n"; }

// Result 0: empty stack even though Push has happened
void
verifyResult0()
   {
   cout << "Push(1)  [ no commit ]\n";
   OperandStackTestMethod::verifyStack("0", -1, 0);
   }

void
verifyResult1()
   {
   cout << "Commit(); Top()\n";
   OperandStackTestMethod::verifyStack("1", 0, 1, 1);
   }

void
verifyResult2(STACKVALUETYPE top)
   {
   cout << "Push(2); Push(3); Top()   [ no commit]\n";
   cout << "\tResult 2: top value == 3: ";
   REPORT1(top == 3, "top", top);

   OperandStackTestMethod::verifyStack("2", 0, 1, 1);
   }

void
verifyResult3(STACKVALUETYPE top)
   {
   cout << "Commit(); Top()\n";
   cout << "\tResult 3: top value == 3: ";
   REPORT1(top == 3, "top", top);

   OperandStackTestMethod::verifyStack("3", 2, 3, 3, 2, 1);
   }

void
verifyResult4(STACKVALUETYPE popValue)
   {
   cout << "Pop()    [ no commit]\n";
   cout << "\tResult 4: pop value == 3: ";
   REPORT1(popValue == 3, "popValue", popValue);

   OperandStackTestMethod::verifyStack("4", 2, 3, 3, 2, 1);
   }

void
verifyResult5(STACKVALUETYPE popValue)
   {
   cout << "Pop()    [ no commit]\n";
   cout << "\tResult 5: pop value == 2: ";
   REPORT1(popValue == 2, "popValue", popValue);

   OperandStackTestMethod::verifyStack("5", 2, 3, 3, 2, 1);
   }

void
verifyResult6(STACKVALUETYPE top)
   {
   cout << "Push(Add(popValue1, popValue2)); Commit(); Top()\n";
   cout << "\tResult 6: top == 5: ";
   REPORT1(top == 5, "top", top);

   OperandStackTestMethod::verifyStack("6", 2, 2, 5, 1);
   }

void
verifyResult7()
   {
   cout << "Drop(2); Commit(); [ empty stack ]\n";
   OperandStackTestMethod::verifyStack("7", 2, 0);
   }

void
verifyResult8(STACKVALUETYPE pick)
   {
   cout << "Push(5); Push(4); Push(3); Push(2); Push(1); Commit(); Pick(3)\n";
   cout << "\tResult 8: pick == 4: ";
   REPORT1(pick == 4, "pick", pick);

   OperandStackTestMethod::verifyStack("8", 2, 0);
   }

void
verifyResult9(STACKVALUETYPE top)
   {
   cout << "Drop(2); Top()\n";
   cout << "\tResult 9: top == 3: ";
   REPORT1(top == 3, "top", top);

   OperandStackTestMethod::verifyStack("9", 2, 0);
   }

void
verifyResult10(STACKVALUETYPE pick)
   {
   cout << "Dup(); Pick(2)\n";
   cout << "\tResult 10: pick == 4: ";
   REPORT1(pick == 4, "pick", pick);

   OperandStackTestMethod::verifyStack("10", 2, 0);
   }

void
verifyResult11()
   {
   cout << "Commit();\n";
   OperandStackTestMethod::verifyStack("11", 3, 4, 3, 3, 4, 5);
   }

bool
OperandStackTestMethod::verifyUntouched(int32_t maxTouched)
   {
   for (int32_t i=maxTouched+1;i < _realStackSize;i++) 
      if (_realStack[i] != 0)
         return false;
   return true;
   }

void
OperandStackTestMethod::verifyStack(const char *step, int32_t max, int32_t num, ...)
   {
   va_list args;

   va_start(args, num);

   STACKVALUETYPE *realSP = *verifySP;

   cout << "\tResult " << step << ": realSP-_realStack == " << num << ": ";
   REPORT2((realSP-_realStack) == num, "_realStackTop-_realStack", (realSP-_realStack), "num", num);

   for (int32_t a=0;a < num;a++)
      {
      STACKVALUETYPE val = va_arg(args, STACKVALUETYPE);
      cout << "\tResult " << step << ": _realStack[" << a << "] == " << val << ": ";
      REPORT2(_realStack[a] == val, "_realStack[a]", _realStack[a], "val", val);
      }

   cout << "\tResult " << step << ": upper stack untouched: ";
   REPORT1(verifyUntouched(max), "max", max);
   }


OperandStackTestMethod::OperandStackTestMethod(TR::TypeDictionary *d)
   : MethodBuilder(d)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   DefineName("test");
   DefineReturnType(NoType);

   _realStackSize = 32;
   _realStack = (STACKVALUETYPE *) malloc (_realStackSize * sizeof(STACKVALUETYPE));
   _realStackTop = _realStack;
   memset(_realStack, 0, _realStackSize*sizeof(STACKVALUETYPE));

   _valueType = STACKVALUEILTYPE;

   DefineFunction("verifyResult0", "0", "0", (void *)&verifyResult0, NoType, 0);
   DefineFunction("verifyResult1", "0", "0", (void *)&verifyResult1, NoType, 0);
   DefineFunction("verifyResult2", "0", "0", (void *)&verifyResult2, NoType, 1, _valueType);
   DefineFunction("verifyResult3", "0", "0", (void *)&verifyResult3, NoType, 1, _valueType);
   DefineFunction("verifyResult4", "0", "0", (void *)&verifyResult4, NoType, 1, _valueType);
   DefineFunction("verifyResult5", "0", "0", (void *)&verifyResult5, NoType, 1, _valueType);
   DefineFunction("verifyResult6", "0", "0", (void *)&verifyResult6, NoType, 1, _valueType);
   DefineFunction("verifyResult7", "0", "0", (void *)&verifyResult7, NoType, 0);
   DefineFunction("verifyResult8", "0", "0", (void *)&verifyResult8, NoType, 1, _valueType);
   DefineFunction("verifyResult9", "0", "0", (void *)&verifyResult9, NoType, 1, _valueType);
   DefineFunction("verifyResult10", "0", "0", (void *)&verifyResult10, NoType, 1, _valueType);
   DefineFunction("verifyResult11", "0", "0", (void *)&verifyResult11, NoType, 0);
   }


bool
OperandStackTestMethod::testStack()
   {
   TR::IlBuilder *b = this;

   _stack->Push(b, ConstInteger(_valueType, 1));
   Call("verifyResult0", 0);

   _stack->Commit(b);
   Call("verifyResult1", 0);

   _stack->Push(b, ConstInteger(_valueType, 2));
   _stack->Push(b, ConstInteger(_valueType, 3));
   Call("verifyResult2", 1, _stack->Top());

   _stack->Commit(b);
   Call("verifyResult3", 1, _stack->Top());

   TR::IlValue *val1 = _stack->Pop(b);
   Call("verifyResult4", 1, val1);

   TR::IlValue *val2 = _stack->Pop(b);
   Call("verifyResult5", 1, val2);

   TR::IlValue *sum = Add(val1, val2);
   _stack->Push(b, sum);
   _stack->Commit(b);
   Call("verifyResult6", 1, _stack->Top());

   _stack->Drop(b, 2);
   _stack->Commit(b);
   Call("verifyResult7", 0);

   _stack->Push(b, ConstInteger(_valueType, 5));
   _stack->Push(b, ConstInteger(_valueType, 4));
   _stack->Push(b, ConstInteger(_valueType, 3));
   _stack->Push(b, ConstInteger(_valueType, 2));
   _stack->Push(b, ConstInteger(_valueType, 1));
   Call("verifyResult8", 1, _stack->Pick(3));

   _stack->Drop(b, 2);
   Call("verifyResult9", 1, _stack->Top());

   _stack->Dup(b);
   Call("verifyResult10", 1, _stack->Pick(2));

   _stack->Commit(b);
   Call("verifyResult11", 0);

   Return();

   return true;
   }

bool
OperandStackTestMethod::buildIL()
   {
   TR::IlType *pValueType = _types->PointerTo(_valueType);
   TR::IlValue *realStackTopAddress = ConstAddress(&_realStackTop);
   _stackTop = new OMR::VirtualMachineRegister(this, "SP", pValueType, sizeof(STACKVALUETYPE), realStackTopAddress);
   _stack = new OMR::VirtualMachineOperandStack(this, 32, _valueType, _stackTop);

   return testStack();
   }




OperandStackTestUsingStructMethod::OperandStackTestUsingStructMethod(TR::TypeDictionary *d)
   : OperandStackTestMethod(d)
   {
   d->DefineStruct("Thread");
   d->DefineField("Thread", "sp", d->PointerTo(STACKVALUEILTYPE), offsetof(Thread, sp));
   d->CloseStruct("Thread");

   DefineParameter("thread", d->PointerTo("Thread"));
   }

bool
OperandStackTestUsingStructMethod::buildIL()
   {
   _stackTop = new OMR::VirtualMachineRegisterInStruct(this, "Thread", "thread", "sp", "SP");
   _stack = new OMR::VirtualMachineOperandStack(this, 32, _valueType, _stackTop);

   return testStack();
   }
