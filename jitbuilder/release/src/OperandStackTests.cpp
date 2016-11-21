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
#include "OperandStackTests.hpp"

using std::cout;
using std::cerr;


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

   cout << "Step 2: define type dictionary\n";
   TR::TypeDictionary types;

   cout << "Step 3: compile method builder\n";
   OperandStackTestMethod method(&types);
   uint8_t *entry = 0;
   int32_t rc = compileMethodBuilder(&method, &entry);
   if (rc != 0)
      {
      cerr << "FAIL: compilation error " << rc << "\n";
      exit(-2);
      }

   cout << "Step 4: invoke compiled code and print results\n";
   typedef void (OperandStackTestMethodFunction)();
   OperandStackTestMethodFunction *test = (OperandStackTestMethodFunction *) entry;

   test();

   cout << "Step 5: shutdown JIT\n";
   shutdownJit();
   }


int32_t *OperandStackTestMethod::_realStack = NULL;
int32_t OperandStackTestMethod::_realStackTop = -1;
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
verifyResult2(int32_t top)
   {
   cout << "Push(2); Push(3); Top()   [ no commit]\n";
   cout << "\tResult 2: top value == 3: ";
   REPORT1(top == 3, "top", top);

   OperandStackTestMethod::verifyStack("2", 0, 1, 1);
   }

void
verifyResult3(int32_t top)
   {
   cout << "Commit(); Top()\n";
   cout << "\tResult 3: top value == 3: ";
   REPORT1(top == 3, "top", top);

   OperandStackTestMethod::verifyStack("3", 2, 3, 3, 2, 1);
   }

void
verifyResult4(int32_t popValue)
   {
   cout << "Pop()    [ no commit]\n";
   cout << "\tResult 4: pop value == 3: ";
   REPORT1(popValue == 3, "popValue", popValue);

   OperandStackTestMethod::verifyStack("4", 2, 3, 3, 2, 1);
   }

void
verifyResult5(int32_t popValue)
   {
   cout << "Pop()    [ no commit]\n";
   cout << "\tResult 5: pop value == 2: ";
   REPORT1(popValue == 2, "popValue", popValue);

   OperandStackTestMethod::verifyStack("5", 2, 3, 3, 2, 1);
   }

void
verifyResult6(int32_t top)
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
verifyResult8(int32_t pick)
   {
   cout << "Push(5); Push(4); Push(3); Push(2); Push(1); Commit(); Pick(3)\n";
   cout << "\tResult 8: pick == 4: ";
   REPORT1(pick == 4, "pick", pick);

   OperandStackTestMethod::verifyStack("8", 2, 0);
   }

void
verifyResult9(int32_t top)
   {
   cout << "Drop(2); Top()\n";
   cout << "\tResult 9: top == 3: ";
   REPORT1(top == 3, "top", top);

   OperandStackTestMethod::verifyStack("9", 2, 0);
   }

void
verifyResult10(int32_t pick)
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

   cout << "\tResult " << step << ": _realStackTop == " << (num-1) << ": ";
   REPORT2(_realStackTop == num-1, "_realStackTop", _realStackTop, "num", num);

   for (int32_t a=0;a < num;a++)
      {
      int32_t val = va_arg(args, int32_t);
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
   _realStack = (int32_t *) malloc (_realStackSize * sizeof(int32_t));
   _realStackTop = -1;
   memset(_realStack, 0, _realStackSize*sizeof(int32_t));

   DefineFunction("verifyResult0", "0", "0", (void *)&verifyResult0, NoType, 0);
   DefineFunction("verifyResult1", "0", "0", (void *)&verifyResult1, NoType, 0);
   DefineFunction("verifyResult2", "0", "0", (void *)&verifyResult2, NoType, 1, Int32);
   DefineFunction("verifyResult3", "0", "0", (void *)&verifyResult3, NoType, 1, Int32);
   DefineFunction("verifyResult4", "0", "0", (void *)&verifyResult4, NoType, 1, Int32);
   DefineFunction("verifyResult5", "0", "0", (void *)&verifyResult5, NoType, 1, Int32);
   DefineFunction("verifyResult6", "0", "0", (void *)&verifyResult6, NoType, 1, Int32);
   DefineFunction("verifyResult7", "0", "0", (void *)&verifyResult7, NoType, 0);
   DefineFunction("verifyResult8", "0", "0", (void *)&verifyResult8, NoType, 1, Int32);
   DefineFunction("verifyResult9", "0", "0", (void *)&verifyResult9, NoType, 1, Int32);
   DefineFunction("verifyResult10", "0", "0", (void *)&verifyResult10, NoType, 1, Int32);
   DefineFunction("verifyResult11", "0", "0", (void *)&verifyResult11, NoType, 0);
   }

class TestOperandStack : public OMR::VirtualMachineOperandStack
   {
   public:
   TestOperandStack(TR::MethodBuilder *mb, TR::IlType *Int32, OMR::VirtualMachineRegister *stackBase, OMR::VirtualMachineRegister *stackTop)
      : OMR::VirtualMachineOperandStack(mb, 32, Int32, stackBase),
      _stackTop(stackTop)
      {
      }

   virtual void Commit(TR::IlBuilder *b)
      {
      this->OMR::VirtualMachineOperandStack::Commit(b);
      _stackTop->Commit(b);
      }

   virtual void Push(TR::IlBuilder *b, TR::IlValue *v)
      {
      this->OMR::VirtualMachineOperandStack::Push(v);
      adjustTop(b, +1);
      }

   virtual TR::IlValue *Pop(TR::IlBuilder *b)
      {
      adjustTop(b, -1);
      return this->OMR::VirtualMachineOperandStack::Pop();
      }

   virtual TR::IlValue *Top(TR::IlBuilder *b)
      {
      return this->OMR::VirtualMachineOperandStack::Top();
      }

   virtual TR::IlValue *Pick(TR::IlBuilder *b, int32_t depth)
      {
      return this->OMR::VirtualMachineOperandStack::Pick(depth);
      }

   virtual void Drop(TR::IlBuilder *b, int32_t depth)
      {
      adjustTop(b, -depth);
      this->OMR::VirtualMachineOperandStack::Drop(depth);
      }

   void Dup(TR::IlBuilder *b)
      {
      this->OMR::VirtualMachineOperandStack::Dup();
      adjustTop(b, +1);
      }


   private:
   void adjustTop(TR::IlBuilder *b, int32_t amount)
      {
      _stackTop->Store(b, b->Add(_stackTop->Load(b), b->ConstInt32(amount)));
      }

   OMR::VirtualMachineRegister *_stackTop;
   };

bool
OperandStackTestMethod::buildIL()
   {
   TR::IlType *pInt32 = _types->PointerTo(Int32);
   _stackTop = new OMR::VirtualMachineRegister(this, "stackTop", pInt32, &_realStackTop);

   TR::IlType *pWord = _types->PointerTo(Word);
   OMR::VirtualMachineRegister stackBase(this, "stackBase", pWord, &_realStack);
   _stack = new TestOperandStack(this, Int32, &stackBase, _stackTop);

   TR::IlBuilder *b = this;

   _stack->Push(b, ConstInt32(1));
   Call("verifyResult0", 0);

   _stack->Commit(b);
   Call("verifyResult1", 0);

   _stack->Push(b, ConstInt32(2));
   _stack->Push(b, ConstInt32(3));
   Call("verifyResult2", 1, _stack->Top(b));

   _stack->Commit(b);
   Call("verifyResult3", 1, _stack->Top(b));

   TR::IlValue *val1 = _stack->Pop(b);
   Call("verifyResult4", 1, val1);

   TR::IlValue *val2 = _stack->Pop(b);
   Call("verifyResult5", 1, val2);

   TR::IlValue *sum = Add(val1, val2);
   _stack->Push(b, sum);
   _stack->Commit(b);
   Call("verifyResult6", 1, _stack->Top(b));

   _stack->Drop(b, 2);
   _stack->Commit(b);
   Call("verifyResult7", 0);

   _stack->Push(b, ConstInt32(5));
   _stack->Push(b, ConstInt32(4));
   _stack->Push(b, ConstInt32(3));
   _stack->Push(b, ConstInt32(2));
   _stack->Push(b, ConstInt32(1));
   Call("verifyResult8", 1, _stack->Pick(b, 3));

   _stack->Drop(b, 2);
   Call("verifyResult9", 1, _stack->Top(b));

   _stack->Dup(b);
   Call("verifyResult10", 1, _stack->Pick(b, 2));

   _stack->Commit(b);
   Call("verifyResult11", 0);

   Return();

   return true;
   }
