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


#ifndef OMR_ILBUILDERRECORDER_INCL
#define OMR_ILBUILDERRECORDER_INCL

#ifndef TR_ILBUILDERRECORDER_DEFINED
#define TR_ILBUILDERRECORDER_DEFINED
#define PUT_OMR_ILBUILDERRECORDER_INTO_TR
#endif // !defined(TR_ILBUILDERRECORDER_DEFINED)

#include <fstream>
#include <stdarg.h>
#include <string.h>
#include "ilgen/IlInjector.hpp"
#include "il/ILHelpers.hpp"

#include "ilgen/IlValue.hpp" // must go after IlInjector.hpp or TR_ALLOC isn't cleaned up

namespace TR { class JitBuilderRecorder; }
namespace TR { class MethodBuilder; }
namespace TR { class IlBuilder; }
namespace TR { class IlBuilderRecorder; }
namespace TR { class IlType; }
namespace TR { class TypeDictionary; }

namespace OMR
{

class IlBuilderRecorder : public TR::IlInjector
   {

public:
   TR_ALLOC(TR_Memory::IlGenerator)

   IlBuilderRecorder(TR::MethodBuilder *methodBuilder, TR::TypeDictionary *types);
   IlBuilderRecorder(TR::IlBuilder *source);
   virtual ~IlBuilderRecorder() { }

   // needed to make IlBuilderRecorder a concrete class
   virtual bool injectIL()      { return true; }

   void NewIlBuilder();

   // create a new local value (temporary variable)
   TR::IlValue *NewValue(TR::IlType *dt);

   // constants
   TR::IlValue *NullAddress();
   TR::IlValue *ConstInt8(int8_t value);
   TR::IlValue *ConstInt16(int16_t value);
   TR::IlValue *ConstInt32(int32_t value);
   TR::IlValue *ConstInt64(int64_t value);
   TR::IlValue *ConstFloat(float value);
   TR::IlValue *ConstDouble(double value);
   TR::IlValue *ConstAddress(const void * const value);
   TR::IlValue *ConstString(const char * const value);
   TR::IlValue *ConstzeroValueForValue(TR::IlValue *v);

   TR::IlValue *Const(int8_t value)             { return ConstInt8(value); }
   TR::IlValue *Const(int16_t value)            { return ConstInt16(value); }
   TR::IlValue *Const(int32_t value)            { return ConstInt32(value); }
   TR::IlValue *Const(int64_t value)            { return ConstInt64(value); }
   TR::IlValue *Const(float value)              { return ConstFloat(value); }
   TR::IlValue *Const(double value)             { return ConstDouble(value); }
   TR::IlValue *Const(const void * const value) { return ConstAddress(value); }

   TR::IlValue *ConstInteger(TR::IlType *intType, int64_t value);

   // arithmetic
   TR::IlValue *Add(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *AddWithOverflow(TR::IlBuilder **handler, TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *AddWithUnsignedOverflow(TR::IlBuilder **handler, TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *Sub(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *SubWithOverflow(TR::IlBuilder **handler, TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *SubWithUnsignedOverflow(TR::IlBuilder **handler, TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *Mul(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *MulWithOverflow(TR::IlBuilder **handler, TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *Div(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *And(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *Or(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *Xor(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *ShiftL(TR::IlValue *v, TR::IlValue *amount);
   TR::IlValue *ShiftL(TR::IlValue *v, int8_t amount)                { return ShiftL(v, ConstInt8(amount)); }
   TR::IlValue *ShiftR(TR::IlValue *v, TR::IlValue *amount);
   TR::IlValue *ShiftR(TR::IlValue *v, int8_t amount)                { return ShiftR(v, ConstInt8(amount)); }
   TR::IlValue *UnsignedShiftR(TR::IlValue *v, TR::IlValue *amount);
   TR::IlValue *UnsignedShiftR(TR::IlValue *v, int8_t amount)        { return UnsignedShiftR(v, ConstInt8(amount)); }
   TR::IlValue *NotEqualTo(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *EqualTo(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *LessThan(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *UnsignedLessThan(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *LessOrEqualTo(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *UnsignedLessOrEqualTo(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *GreaterThan(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *UnsignedGreaterThan(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *GreaterOrEqualTo(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *UnsignedGreaterOrEqualTo(TR::IlValue *left, TR::IlValue *right);
   TR::IlValue *ConvertTo(TR::IlType *t, TR::IlValue *v);
   TR::IlValue *UnsignedConvertTo(TR::IlType *t, TR::IlValue *v);

   // memory
   void Store(const char *name, TR::IlValue *value);
   void StoreOver(TR::IlValue *dest, TR::IlValue *value);
   void StoreAt(TR::IlValue *address, TR::IlValue *value);
   void StoreIndirect(const char *type, const char *field, TR::IlValue *object, TR::IlValue *value);
   TR::IlValue *Load(const char *name);
   TR::IlValue *LoadIndirect(const char *type, const char *field, TR::IlValue *object);
   TR::IlValue *LoadAt(TR::IlType *dt, TR::IlValue *address);
   TR::IlValue *IndexAt(TR::IlType *dt, TR::IlValue *base, TR::IlValue *index);
   TR::IlValue *CreateLocalArray(int32_t numElements, TR::IlType *elementType);
   TR::IlValue *CreateLocalStruct(TR::IlType *structType);
   TR::IlValue *AtomicAddWithOffset(TR::IlValue *baseAddress, TR::IlValue *offset, TR::IlValue *value);
   TR::IlValue *AtomicAdd(TR::IlValue *baseAddress, TR::IlValue * value);

   /**
    * `StructFieldAddress` and `UnionFieldAddress` are two functions that
    * provide a workaround for a limitation in JitBuilder. Becuase `TR::IlValue`
    * cannot represent an object type (only pointers to objects), there is no
    * way to generate a load for a field that is itself an object. The workaround
    * is to use the field's address instead. This is not an elegent solution and
    * should be revisited.
    */
   TR::IlValue *StructFieldInstanceAddress(const char* structName, const char* fieldName, TR::IlValue* obj);
   TR::IlValue *UnionFieldInstanceAddress(const char* unionName, const char* fieldName, TR::IlValue* obj);

   // vector memory
   TR::IlValue *VectorLoad(const char *name);
   TR::IlValue *VectorLoadAt(TR::IlType *dt, TR::IlValue *address);
   void VectorStore(const char *name, TR::IlValue *value);
   void VectorStoreAt(TR::IlValue *address, TR::IlValue *value);

   // control
   void Transaction(TR::IlBuilder **persistentFailureBuilder, TR::IlBuilder **transientFailureBuilder, TR::IlBuilder **fallThroughBuilder);
   void TransactionAbort();
   void AppendBuilder(TR::IlBuilder *builder);
   TR::IlValue *Call(const char *name, int32_t numArgs, ...);
   TR::IlValue *Call(const char *name, int32_t numArgs, TR::IlValue **argValues);
   TR::IlValue *ComputedCall(const char *name, int32_t numArgs, ...);
   TR::IlValue *ComputedCall(const char *name, int32_t numArgs, TR::IlValue **args);
   void Goto(TR::IlBuilder **dest);
   void Goto(TR::IlBuilder *dest);
   void Return();
   void Return(TR::IlValue *value);
   virtual void ForLoop(bool countsUp,
                const char *indVar,
                TR::IlBuilder *body,
                TR::IlBuilder *breakBuilder,
                TR::IlBuilder *continueBuilder,
                TR::IlValue *initial,
                TR::IlValue *iterateWhile,
                TR::IlValue *increment);
   virtual void ForLoop(bool countsUp,
                const char *indVar,
                TR::IlBuilder **body,
                TR::IlBuilder **breakBuilder,
                TR::IlBuilder **continueBuilder,
                TR::IlValue *initial,
                TR::IlValue *iterateWhile,
                TR::IlValue *increment);

   void ForLoopUp(const char *indVar,
                  TR::IlBuilder **body,
                  TR::IlValue *initial,
                  TR::IlValue *iterateWhile,
                  TR::IlValue *increment)
      {
      ForLoop(true, indVar, body, NULL, NULL, initial, iterateWhile, increment);
      }

   void ForLoopDown(const char *indVar,
                    TR::IlBuilder **body,
                    TR::IlValue *initial,
                    TR::IlValue *iterateWhile,
                    TR::IlValue *increment)
      {
      ForLoop(false, indVar, body, NULL, NULL, initial, iterateWhile, increment);
      }

   void ForLoopWithBreak(bool countsUp,
                         const char *indVar,
                         TR::IlBuilder **body,
                         TR::IlBuilder **breakBody,
                         TR::IlValue *initial,
                         TR::IlValue *iterateWhile,
                         TR::IlValue *increment)
      {
      ForLoop(countsUp, indVar, body, breakBody, NULL, initial, iterateWhile, increment);
      }

   void ForLoopWithContinue(bool countsUp,
                            const char *indVar,
                            TR::IlBuilder **body,
                            TR::IlBuilder **continueBody,
                            TR::IlValue *initial,
                            TR::IlValue *iterateWhile,
                            TR::IlValue *increment)
      {
      ForLoop(countsUp, indVar, body, NULL, continueBody, initial, iterateWhile, increment);
      }

#if 0
   virtual void WhileDoLoop(const char *exitCondition, TR::IlBuilder **body, TR::IlBuilder **breakBuilder = NULL, TR::IlBuilder **continueBuilder = NULL);
   void WhileDoLoopWithBreak(const char *exitCondition, TR::IlBuilder **body, TR::IlBuilder **breakBuilder)
      {
      WhileDoLoop(exitCondition, body, breakBuilder);
      }

   void WhileDoLoopWithContinue(const char *exitCondition, TR::IlBuilder **body, TR::IlBuilder **continueBuilder)
      {
      WhileDoLoop(exitCondition, body, NULL, continueBuilder);
      }

   virtual void DoWhileLoop(const char *exitCondition, TR::IlBuilder **body, TR::IlBuilder **breakBuilder = NULL, TR::IlBuilder **continueBuilder = NULL);
   void DoWhileLoopWithBreak(const char *exitCondition, TR::IlBuilder **body, TR::IlBuilder **breakBuilder)
      {
      DoWhileLoop(exitCondition, body, breakBuilder);
      }
   void DoWhileLoopWithContinue(const char *exitCondition, TR::IlBuilder **body, TR::IlBuilder **continueBuilder)
      {
      DoWhileLoop(exitCondition, body, NULL, continueBuilder);
      }
#endif

   /* @brief creates an AND nest of short-circuited conditions, for each term pass an IlBuilder containing the condition and the IlValue that computes the condition */
   void IfAnd(TR::IlBuilder **allTrueBuilder, TR::IlBuilder **anyFalseBuilder, int32_t numTerms, ... );
   /* @brief creates an OR nest of short-circuited conditions, for each term pass an IlBuilder containing the condition and the IlValue that computes the condition */
   void IfOr(TR::IlBuilder **anyTrueBuilder, TR::IlBuilder **allFalseBuilder, int32_t numTerms, ... );

   void IfCmpNotEqualZero(TR::IlBuilder **target, TR::IlValue *condition);
   void IfCmpNotEqualZero(TR::IlBuilder *target, TR::IlValue *condition);
   void IfCmpNotEqual(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpNotEqual(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpEqualZero(TR::IlBuilder **target, TR::IlValue *condition);
   void IfCmpEqualZero(TR::IlBuilder *target, TR::IlValue *condition);
   void IfCmpEqual(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpEqual(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpLessThan(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpLessThan(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpUnsignedLessThan(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpUnsignedLessThan(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpLessOrEqual(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpLessOrEqual(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpUnsignedLessOrEqual(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpUnsignedLessOrEqual(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpGreaterThan(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpGreaterThan(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpUnsignedGreaterThan(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpUnsignedGreaterThan(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpGreaterOrEqual(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpGreaterOrEqual(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpUnsignedGreaterOrEqual(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right);
   void IfCmpUnsignedGreaterOrEqual(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right);

   void IfThenElse(TR::IlBuilder *thenPath, TR::IlBuilder *elsePath, TR::IlValue *condition);
   void IfThenElse(TR::IlBuilder **thenPath, TR::IlBuilder **elsePath, TR::IlValue *condition);
   virtual void IfThen(TR::IlBuilder **thenPath, TR::IlValue *condition)
      {
      IfThenElse(thenPath, NULL, condition);
      }
#if 0
   void Switch(const char *selectionVar,
               TR::IlBuilder **defaultBuilder,
               uint32_t numCases,
               int32_t *caseValues,
               TR::IlBuilder **caseBuilders,
               bool *caseFallsThrough);
   void Switch(const char *selectionVar,
               TR::IlBuilder **defaultBuilder,
               uint32_t numCases,
               ...);
#endif

protected:

   /**
    * @brief MethodBuilder parent for this IlBuilder object
    */
   TR::MethodBuilder      * _methodBuilder;

   TR::IlBuilder *asIlBuilder();
   TR::IlValue *newValue();
   TR::IlBuilder *createBuilderIfNeeded(TR::IlBuilder *builder);

   void convertTo(TR::IlValue *returnValue, TR::IlType *dt, TR::IlValue *v, const char *s);
   void binaryOp(const TR::IlValue *returnValue, const TR::IlValue *left, const TR::IlValue *right, const char *s);
   void unaryOp(TR::IlValue *returnValue, TR::IlValue *v, const char *s);

   TR::JitBuilderRecorder *recorder() const;
   TR::JitBuilderRecorder *clearRecorder();
   void restoreRecorder(TR::JitBuilderRecorder *recorder);
   };

} // namespace OMR



#if defined(PUT_OMR_ILBUILDERRECORDER_INTO_TR)

namespace TR
{
   class IlBuilderRecorder : public OMR::IlBuilderRecorder
      {
      public:
         IlBuilderRecorder(TR::MethodBuilder *methodBuilder, TR::TypeDictionary *types)
            : OMR::IlBuilderRecorder(methodBuilder, types)
            { }

         IlBuilderRecorder(TR::IlBuilder *source)
            : OMR::IlBuilderRecorder(source)
            { }
      };

} // namespace TR

#endif // defined(PUT_OMR_ILBUILDERRECORDER_INTO_TR)

#endif // !defined(OMR_ILBUILDERRECORDER_INCL)
