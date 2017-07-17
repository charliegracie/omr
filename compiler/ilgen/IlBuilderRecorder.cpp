/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2000, 2017
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

#include "ilgen/IlBuilderRecorder.hpp"
#include "ilgen/IlBuilder.hpp"

#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "infra/Assert.hpp"
#include "compile/Compilation.hpp"
#include "ilgen/JitBuilderRecorder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/IlValue.hpp"

// IlBuilder is a class designed to help build Testarossa IL quickly without
// a lot of knowledge of the intricacies of commoned references, symbols,
// symbol references, or blocks. You can add operations to an IlBuilder via
// a set of services, for example: Add, Sub, Load, Store. These services
// operate on TR::IlValues, which currently correspond to SymbolReferences.
// However, IlBuilders also incorporate a notion of "symbols" that are
// identified by strings (i.e. arbitrary symbolic names).
//
// So Load("a") returns a TR::IlValue. Load("b") returns a different
// TR::IlValue. The value of a+b can be computed by Add(Load("a"),Load("b")),
// and that value can be stored into c by Store("c", Add(Load("a"),Load("b"))).
//
// More complicated services exist to construct control flow by linking
// together sets of IlBuilder objects. A simple if-then construct, for
// example, can be built with the following code:
//    TR::IlValue *condition = NotEqualToZero(Load("a"));
//    TR::IlBuilder *then = NULL;
//    IfThen(&then, condition);
//    then->Return(then->ConstInt32(0));
//
//
// An IlBuilder is really a sequence of Blocks and other IlBuilders, but an
// IlBuilder is also a sequence of TreeTops connecting together a set of
// Blocks that are arranged in the CFG. Another way to think of an IlBuilder
// is as representing the code needed to execute a particular code path
// (which may itasIlBuilder have sub control flow handled by embedded IlBuilders).
// All IlBuilders exist within the single control flow graph, but the full
// control flow graph will not be connected together until all IlBuilder
// objects have had injectIL() called.


OMR::IlBuilderRecorder::IlBuilderRecorder(TR::MethodBuilder *methodBuilder, TR::TypeDictionary *types)
      : TR::IlInjector(types),
      _methodBuilder(methodBuilder)
   {
   }

OMR::IlBuilderRecorder::IlBuilderRecorder(TR::IlBuilder *source)
      : TR::IlInjector(source),
      _methodBuilder(source->_methodBuilder)
   {
   }

TR::IlBuilder *
OMR::IlBuilderRecorder::asIlBuilder()
   {
   return static_cast<TR::IlBuilder *>(this);
   }

TR::JitBuilderRecorder *
OMR::IlBuilderRecorder::recorder() const
   {
   return _methodBuilder->recorder();
   }

TR::JitBuilderRecorder *
OMR::IlBuilderRecorder::clearRecorder()
   {
   TR::JitBuilderRecorder *recorder = _methodBuilder->recorder();
   _methodBuilder->setRecorder(NULL);
   return recorder;
   }

void
OMR::IlBuilderRecorder::restoreRecorder(TR::JitBuilderRecorder *recorder)
   {
   _methodBuilder->setRecorder(recorder);
   }

void
OMR::IlBuilderRecorder::NewIlBuilder()
   {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(asIlBuilder());
      rec->BeginStatement(rec->STATEMENT_NEWILBUILDER);
      rec->Builder(asIlBuilder());
      rec->EndStatement();
      }
   }

TR::IlBuilder *
OMR::IlBuilderRecorder::createBuilderIfNeeded(TR::IlBuilder *builder)
   {
   if (builder == NULL)
      builder = asIlBuilder()->OrphanBuilder();
   return builder;
   }

TR::IlValue *
OMR::IlBuilderRecorder::newValue()
   {
   TR::IlValue *value = new (_comp->trHeapMemory()) TR::IlValue(_methodBuilder);
   return value;
   }

void
OMR::IlBuilderRecorder::binaryOp(const TR::IlValue *returnValue, const TR::IlValue *left, const TR::IlValue *right, const char *s)
   {
   TR::JitBuilderRecorder *rec = recorder();
   TR_ASSERT(rec, "cannot call IlBuilderRecorder::binaryOp if recorder is NULL");

   rec->StoreID(returnValue);
   rec->BeginStatement(asIlBuilder(), s);
   rec->Value(returnValue);
   rec->Value(left);
   rec->Value(right);
   rec->EndStatement();
   }

void
OMR::IlBuilderRecorder::AppendBuilder(TR::IlBuilder *builder)
   {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_APPENDBUILDER);
      rec->Builder(builder);
      rec->EndStatement();
      }
   }

/**
 * @brief Store an IlValue into a named local variable
 * @param varName the name of the local variable to be stored into. If the name has not been used before, this local variable will
 *                take the same data type as the value being written to it.
 * @param value IlValue that should be written to the local variable, which should be the same data type
 */
void
OMR::IlBuilderRecorder::Store(const char *name, TR::IlValue *value)
   {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_STORE);
      rec->String(name);
      rec->Value(value);
      rec->EndStatement();
      }
   }

/**
 * @brief Store an IlValue into the same local value as another IlValue
 * @param dest IlValue that should now hold the same value as "value"
 * @param value IlValue that should overwrite "dest"
 */
void
OMR::IlBuilderRecorder::StoreOver(TR::IlValue *dest, TR::IlValue *value)
   {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_STOREOVER);
      rec->Value(dest);
      rec->Value(value);
      rec->EndStatement();
      }
   }

/**
 * @brief Store a vector IlValue into a named local variable
 * @param varName the name of the local variable to be vector stored into. if the name has not been used before, this local variable will
 *                take the same data type as the value being written to it. The width of this data will be determined by the vector
 *                data type of IlValue.
 * @param value IlValue with the vector data that should be written to the local variable, and should have the same data type
 */
void
OMR::IlBuilderRecorder::VectorStore(const char *name, TR::IlValue *value)
   {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_VECTORSTORE);
      rec->String(name);
      rec->Value(value);
      rec->EndStatement();
      }
   }

/**
 * @brief Store an IlValue through a pointer
 * @param address the pointer address through which the value will be written
 * @param value IlValue that should be written at "address"
 */
void
OMR::IlBuilderRecorder::StoreAt(TR::IlValue *address, TR::IlValue *value)
   {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_STOREAT);
      rec->Value(address);
      rec->Value(value);
      rec->EndStatement();
      }
   }

/**
 * @brief Store a vector IlValue through a pointer
 * @param address the pointer address through which the vector value will be written. The width of the store will be determined
 *                by the vector data type of IlValue.
 * @param value IlValue with the vector data that should be written  at "address"
 */
void
OMR::IlBuilderRecorder::VectorStoreAt(TR::IlValue *address, TR::IlValue *value)
   {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_VECTORSTOREAT);
      rec->Value(address);
      rec->Value(value);
      rec->EndStatement();
      }
   }

TR::IlValue *
OMR::IlBuilderRecorder::CreateLocalArray(int32_t numElements, TR::IlType *elementType)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_CREATELOCALARRAY);
      rec->Value(returnValue);
      rec->Number(numElements);
      rec->Type(elementType);
      rec->EndStatement();
      }
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::CreateLocalStruct(TR::IlType *structType)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_CREATELOCALSTRUCT);
      rec->Value(returnValue);
      rec->Type(structType);
      rec->EndStatement();
      }
   return returnValue;
   }

void
OMR::IlBuilderRecorder::StoreIndirect(const char *type, const char *field, TR::IlValue *object, TR::IlValue *value)
  {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_STOREINDIRECT);
      rec->String(type);
      rec->String(field);
      rec->Value(object);
      rec->Value(value);
      rec->EndStatement();
      }
  }

TR::IlValue *
OMR::IlBuilderRecorder::Load(const char *name)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_LOAD);
      rec->Value(returnValue);
      rec->String(name);
      rec->EndStatement();
      }

   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::VectorLoad(const char *name)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_VECTORLOAD);
      rec->Value(returnValue);
      rec->String(name);
      rec->EndStatement();
      }

   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::LoadIndirect(const char *type, const char *field, TR::IlValue *object)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_LOADINDIRECT);
      rec->Value(returnValue);
      rec->String(type);
      rec->String(field);
      rec->Value(object);
      rec->EndStatement();
      }

   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::LoadAt(TR::IlType *dt, TR::IlValue *address)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_LOADAT);
      rec->Value(returnValue);
      rec->Type(dt);
      rec->Value(address);
      rec->EndStatement();
      }

   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::VectorLoadAt(TR::IlType *dt, TR::IlValue *address)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_VECTORLOADAT);
      rec->Value(returnValue);
      rec->Type(dt);
      rec->Value(address);
      rec->EndStatement();
      }

   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::IndexAt(TR::IlType *dt, TR::IlValue *base, TR::IlValue *index)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_INDEXAT);
      rec->Value(returnValue);
      rec->Type(dt);
      rec->Value(base);
      rec->Value(index);
      rec->EndStatement();
      }

   return returnValue;
   }

/**
 * @brief Generate IL to load the address of a struct field.
 *
 * The address of the field is calculated by adding the field's offset to the
 * base address of the struct. The address is also converted (type casted) to
 * a pointer to the type of the field.
 */
TR::IlValue *
OMR::IlBuilderRecorder::StructFieldInstanceAddress(const char* structName, const char* fieldName, TR::IlValue* obj)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_STRUCTFIELDINSTANCEADDRESS);
      rec->Value(returnValue);
      rec->String(structName);
      rec->String(fieldName);
      rec->Value(obj);
      rec->EndStatement();
      }

   return returnValue;
   }

/**
 * @brief Generate IL to load the address of a union field.
 *
 * The address of the field is simply the base address of the union, since the
 * offset of all union fields is zero.
 */
TR::IlValue *
OMR::IlBuilderRecorder::UnionFieldInstanceAddress(const char* unionName, const char* fieldName, TR::IlValue* obj)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_UNIONFIELDINSTANCEADDRESS);
      rec->Value(returnValue);
      rec->String(unionName);
      rec->String(fieldName);
      rec->Value(obj);
      rec->EndStatement();
      }

   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::NullAddress()
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_NULLADDRESS);
      rec->Value(returnValue);
      rec->EndStatement();
      }

   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::ConstInt8(int8_t value)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);
      Int8->RecordFirstTime(rec);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_CONSTINT8);
      rec->Value(returnValue);
      rec->Number(value);
      rec->EndStatement();
      }
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::ConstInt16(int16_t value)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);
      Int16->RecordFirstTime(rec);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_CONSTINT16);
      rec->Value(returnValue);
      rec->Number(value);
      rec->EndStatement();
      }
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::ConstInt32(int32_t value)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);
      Int32->RecordFirstTime(rec);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_CONSTINT32);
      rec->Value(returnValue);
      rec->Number(value);
      rec->EndStatement();
      }
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::ConstInt64(int64_t value)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);
      Int64->RecordFirstTime(rec);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_CONSTINT64);
      rec->Value(returnValue);
      rec->Number(value);
      rec->EndStatement();
      }
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::ConstFloat(float value)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);
      Float->RecordFirstTime(rec);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_CONSTFLOAT);
      rec->Value(returnValue);
      rec->Number(value);
      rec->EndStatement();
      }
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::ConstDouble(double value)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);
      Double->RecordFirstTime(rec);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_CONSTDOUBLE);
      rec->Value(returnValue);
      rec->Number(value);
      rec->EndStatement();
      }
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::Add(TR::IlValue *left, TR::IlValue *right)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      binaryOp(returnValue, left, right, rec->STATEMENT_ADD);
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::LessThan(TR::IlValue *left, TR::IlValue *right)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      binaryOp(returnValue, left, right, rec->STATEMENT_LESSTHAN);
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::ConstString(const char * const value)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);
      Address->RecordFirstTime(rec);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_CONSTSTRING);
      rec->Value(returnValue);
      rec->String(value);
      rec->EndStatement();
      }
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::ConstAddress(const void * const value)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);
      Address->RecordFirstTime(rec);

      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_CONSTADDRESS);
      rec->Value(returnValue);
      rec->Location(value);
      rec->EndStatement();
      }
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::ConstInteger(TR::IlType *intType, int64_t value)
   {
   if      (intType == Int8)  return ConstInt8 ((int8_t)  value);
   else if (intType == Int16) return ConstInt16((int16_t) value);
   else if (intType == Int32) return ConstInt32((int32_t) value);
   else if (intType == Int64) return ConstInt64(          value);

   TR_ASSERT(0, "unknown integer type");
   return NULL;
   }

TR::IlValue *
OMR::IlBuilderRecorder::ConvertTo(TR::IlType *dt, TR::IlValue *v)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      convertTo(returnValue, dt, v, rec->STATEMENT_CONVERTTO);
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::UnsignedConvertTo(TR::IlType *dt, TR::IlValue *v)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      convertTo(returnValue, dt, v, rec->STATEMENT_UNSIGNEDCONVERTTO);
   return returnValue;
   }

void
OMR::IlBuilderRecorder::convertTo(TR::IlValue *returnValue, TR::IlType *dt, TR::IlValue *v, const char *s)
   {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);
      dt->RecordFirstTime(rec);

      rec->BeginStatement(asIlBuilder(), s);
      rec->Value(returnValue);
      rec->Type(dt);
      rec->Value(v);
      rec->EndStatement();
      }
   }

void
OMR::IlBuilderRecorder::unaryOp(TR::IlValue *returnValue, TR::IlValue *v, const char *s)
   {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->StoreID(returnValue);

      rec->BeginStatement(s);
      rec->Value(returnValue);
      rec->Value(v);
      rec->EndStatement();
      }
   }

TR::IlValue *
OMR::IlBuilderRecorder::NotEqualTo(TR::IlValue *left, TR::IlValue *right)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      binaryOp(returnValue, left, right, rec->STATEMENT_NOTEQUALTO);
   return returnValue;
   }

void
OMR::IlBuilderRecorder::Goto(TR::IlBuilder **dest)
   {
   *dest = createBuilderIfNeeded(*dest);
   Goto(*dest);
   }

void
OMR::IlBuilderRecorder::Goto(TR::IlBuilder *dest)
   {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_GOTO);
      rec->Builder(dest);
      rec->EndStatement();
      }
   }

void
OMR::IlBuilderRecorder::Return()
   {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_RETURN);
      rec->EndStatement();
      }
   }

void
OMR::IlBuilderRecorder::Return(TR::IlValue *value)
   {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_RETURNVALUE);
      rec->Value(value);
      rec->EndStatement();
      }
   }

TR::IlValue *
OMR::IlBuilderRecorder::Sub(TR::IlValue *left, TR::IlValue *right)
   {
   TR::IlValue *returnValue = newValue();
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      binaryOp(returnValue, left, right, rec->STATEMENT_SUB);
   return returnValue;
   }

#if 0
/*
 * blockThrowsException:
 * ....
 * goto blockAfterExceptionHandler 
 * Handler:
 * ....
 * goto blockAfterExceptionHandler 
 * blockAfterExceptionHandler:
 */
void
OMR::IlBuilderRecorder::appendExceptionHandler(TR::Block *blockThrowsException, TR::IlBuilder **handler, uint32_t catchType)
   {
   //split block after overflowCHK, and add an goto to blockAfterExceptionHandler
   appendBlock();
   TR::Block *blockWithGoto = _currentBlock;
   TR::Node *gotoNode = TR::Node::create(NULL, TR::Goto);
   genTreeTop(gotoNode);
   _currentBlock = NULL;
   _currentBlockNumber = -1;
 
   //append handler, add exception edge and merge edge
   *handler = createBuilderIfNeeded(*handler);
   TR_ASSERT(*handler != NULL, "exception handler cannot be NULL\n");
   (*handler)->_isHandler = true;
   cfg()->addExceptionEdge(blockThrowsException, (*handler)->getEntry());
   AppendBuilder(*handler);
   (*handler)->setHandlerInfo(catchType);

   TR::Block *blockAfterExceptionHandler = _currentBlock;
   TraceIL("blockAfterExceptionHandler block_%d, blockWithGoto block_%d \n", blockAfterExceptionHandler->getNumber(), blockWithGoto->getNumber());
   gotoNode->setBranchDestination(blockAfterExceptionHandler->getEntry());
   cfg()->addEdge(blockWithGoto, blockAfterExceptionHandler);
   }

void
OMR::IlBuilderRecorder::setHandlerInfo(uint32_t catchType)
   {
   TR::Block *catchBlock = getEntry();
   catchBlock->setIsCold();
   catchBlock->setHandlerInfoWithOutBCInfo(catchType, comp()->getInlineDepth(), -1, _methodSymbol->getResolvedMethod(), comp());
   }

TR::Node*
OMR::IlBuilderRecorder::genOverflowCHKTreeTop(TR::Node *operationNode, TR::ILOpCodes overflow)
   {
   TR::Node *overflowChkNode = TR::Node::createWithRoomForOneMore(overflow, 3, symRefTab()->findOrCreateOverflowCheckSymbolRef(_methodSymbol), operationNode, operationNode->getFirstChild(), operationNode->getSecondChild());
   overflowChkNode->setOverflowCheckOperation(operationNode->getOpCodeValue());
   genTreeTop(overflowChkNode);
   return overflowChkNode;
   }

TR::IlValue *
OMR::IlBuilderRecorder::genOperationWithOverflowCHK(TR::ILOpCodes op, TR::Node *leftNode, TR::Node *rightNode, TR::IlBuilder **handler, TR::ILOpCodes overflow)
   {
   /*
    * BB1:
    *    overflowCHK
    *       operation(add/sub/mul)
    *          child1
    *          child2
    *       =>child1
    *       =>child2
    *    store
    *       => operation
    * BB2:
    *    goto BB3 
    * Handler:
    *    ...
    * BB3:
    *    continue
    */
   TR::Node *operationNode = binaryOpNodeFromNodes(op, leftNode, rightNode);
   TR::Node *overflowChkNode = genOverflowCHKTreeTop(operationNode, overflow);

   TR::Block *blockWithOverflowCHK = _currentBlock;
   TR::IlValue *resultValue = newValue(operationNode->getDataType(), operationNode);
   genTreeTop(TR::Node::createStore(resultValue->getSymbolReference(), operationNode));

   appendExceptionHandler(blockWithOverflowCHK, handler, TR::Block::CanCatchOverflowCheck);
   return resultValue;
   }

// This function takes 4 arguments and generate the addValue.
// This function is called by AddWithOverflow and AddWithUnsignedOverflow.
TR::ILOpCodes 
OMR::IlBuilderRecorder::getOpCode(TR::IlValue *leftValue, TR::IlValue *rightValue)
   {
   TR::ILOpCodes op;
   if (leftValue->getDataType() == TR::Address)
      {
      if (rightValue->getDataType() == TR::Int32)
         op = TR::aiadd;
      else if (rightValue->getDataType() == TR::Int64)
         op = TR::aladd;
      else 
         TR_ASSERT(0, "the right child type must be either TR::Int32 or TR::Int64 when the left child of Add is TR::Address\n");
      }    
   else 
      {
      op = addOpCode(leftValue->getDataType());
      }
   return op; 
   }

TR::IlValue *
OMR::IlBuilderRecorder::AddWithOverflow(TR::IlBuilder **handler, TR::IlValue *left, TR::IlValue *right)
   {
   TR::Node *leftNode = loadValue(left);
   TR::Node *rightNode = loadValue(right);
   TR::ILOpCodes opcode = getOpCode(left, right);
   TR::IlValue *addValue = genOperationWithOverflowCHK(opcode, leftNode, rightNode, handler, TR::OverflowCHK);
   TraceIL("IlBuilder[ %p ]::%d is AddWithOverflow %d + %d\n", this, addValue->getID(), left->getID(), right->getID());
   //ILB_REPLAY("%s = %s->AddWithOverflow(%s, %s);", REPLAY_VALUE(addValue), REPLAY_BUILDER(this), REPLAY_BUILDER(*handler), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return addValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::AddWithUnsignedOverflow(TR::IlBuilder **handler, TR::IlValue *left, TR::IlValue *right)
   {
   TR::Node *leftNode = loadValue(left);
   TR::Node *rightNode = loadValue(right);
   TR::ILOpCodes opcode = getOpCode(left, right);
   TR::IlValue *addValue = genOperationWithOverflowCHK(opcode, leftNode, rightNode, handler, TR::UnsignedOverflowCHK);
   TraceIL("IlBuilder[ %p ]::%d is AddWithUnsignedOverflow %d + %d\n", this, addValue->getID(), left->getID(), right->getID());
   //ILB_REPLAY("%s = %s->AddWithUnsignedOverflow(%s, %s, %s);", REPLAY_VALUE(addValue), REPLAY_BUILDER(this), REPLAY_PTRTOBUILDER(handler), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return addValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::SubWithOverflow(TR::IlBuilder **handler, TR::IlValue *left, TR::IlValue *right)
   {
   TR::Node *leftNode = loadValue(left);
   TR::Node *rightNode = loadValue(right);
   TR::IlValue *subValue = genOperationWithOverflowCHK(TR::ILOpCode::subtractOpCode(leftNode->getDataType()), leftNode, rightNode, handler, TR::OverflowCHK);
   TraceIL("IlBuilder[ %p ]::%d is SubWithOverflow %d + %d\n", this, subValue->getID(), left->getID(), right->getID());
   return subValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::SubWithUnsignedOverflow(TR::IlBuilder **handler, TR::IlValue *left, TR::IlValue *right)
   {
   TR::Node *leftNode = loadValue(left);
   TR::Node *rightNode = loadValue(right);
   TR::IlValue *unsignedSubValue = genOperationWithOverflowCHK(TR::ILOpCode::subtractOpCode(leftNode->getDataType()), leftNode, rightNode, handler, TR::UnsignedOverflowCHK);
   TraceIL("IlBuilder[ %p ]::%d is UnsignedSubWithOverflow %d + %d\n", this, unsignedSubValue->getID(), left->getID(), right->getID());
   //ILB_REPLAY("%s = %s->UnsignedSubWithOverflow(%s, %s, %s);", REPLAY_VALUE(unsignedSubValue), REPLAY_BUILDER(this), REPLAY_PTRTOBUILDER(handler), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return unsignedSubValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::MulWithOverflow(TR::IlBuilder **handler, TR::IlValue *left, TR::IlValue *right)
   {
   TR::Node *leftNode = loadValue(left);
   TR::Node *rightNode = loadValue(right);
   TR::IlValue *mulValue = genOperationWithOverflowCHK(TR::ILOpCode::multiplyOpCode(leftNode->getDataType()), leftNode, rightNode, handler, TR::OverflowCHK);
   TraceIL("IlBuilder[ %p ]::%d is MulWithOverflow %d + %d\n", this, mulValue->getID(), left->getID(), right->getID());
   return mulValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::Mul(TR::IlValue *left, TR::IlValue *right)
   {
   TR::IlValue *returnValue=binaryOpFromOpMap(TR::ILOpCode::multiplyOpCode, left, right);
   TraceIL("IlBuilder[ %p ]::%d is Mul %d * %d\n", this, returnValue->getID(), left->getID(), right->getID());
   ILB_REPLAY("%s = %s->Mul(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::Div(TR::IlValue *left, TR::IlValue *right)
   {
   TR::IlValue *returnValue=binaryOpFromOpMap(TR::ILOpCode::divideOpCode, left, right);
   TraceIL("IlBuilder[ %p ]::%d is Div %d / %d\n", this, returnValue->getID(), left->getID(), right->getID());
   ILB_REPLAY("%s = %s->Div(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::And(TR::IlValue *left, TR::IlValue *right)
   {
   TR::IlValue *returnValue=binaryOpFromOpMap(TR::ILOpCode::andOpCode, left, right);
   TraceIL("IlBuilder[ %p ]::%d is And %d & %d\n", this, returnValue->getID(), left->getID(), right->getID());
   ILB_REPLAY("%s = %s->And(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::Or(TR::IlValue *left, TR::IlValue *right)
   {
   TR::IlValue *returnValue=binaryOpFromOpMap(TR::ILOpCode::orOpCode, left, right);
   TraceIL("IlBuilder[ %p ]::%d is Or %d | %d\n", this, returnValue->getID(), left->getID(), right->getID());
   ILB_REPLAY("%s = %s->Or(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::Xor(TR::IlValue *left, TR::IlValue *right)
   {
   TR::IlValue *returnValue=binaryOpFromOpMap(TR::ILOpCode::xorOpCode, left, right);
   TraceIL("IlBuilder[ %p ]::%d is Xor %d ^ %d\n", this, returnValue->getID(), left->getID(), right->getID());
   ILB_REPLAY("%s = %s->Xor(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::ShiftL(TR::IlValue *v, TR::IlValue *amount)
   {
   TR::IlValue *returnValue=binaryOpFromOpMap(TR::ILOpCode::shiftLeftOpCode, v, amount);
   TraceIL("IlBuilder[ %p ]::%d is shr %d << %d\n", this, returnValue->getID(), v->getID(), amount->getID());
   ILB_REPLAY("%s = %s->ShiftL(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(v), REPLAY_VALUE(amount));
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::ShiftR(TR::IlValue *v, TR::IlValue *amount)
   {
   TR::IlValue *returnValue=binaryOpFromOpMap(TR::ILOpCode::shiftRightOpCode, v, amount);
   TraceIL("IlBuilder[ %p ]::%d is shr %d >> %d\n", this, returnValue->getID(), v->getID(), amount->getID());
   ILB_REPLAY("%s = %s->ShiftR(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(v), REPLAY_VALUE(amount));
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::UnsignedShiftR(TR::IlValue *v, TR::IlValue *amount)
   {
   TR::IlValue *returnValue=binaryOpFromOpMap(TR::ILOpCode::unsignedShiftRightOpCode, v, amount);
   TraceIL("IlBuilder[ %p ]::%d is unsigned shr %d >> %d\n", this, returnValue->getID(), v->getID(), amount->getID());
   ILB_REPLAY("%s = %s->UnsignedShiftR(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(v), REPLAY_VALUE(amount));
   return returnValue;
   }

/*
 * @brief IfAnd service for constructing short circuit AND conditional nests (like the && operator)
 * @param allTrueBuilder builder containing operations to execute if all conditional tests evaluate to true
 * @param anyFalseBuilder builder containing operations to execute if any conditional test is false
 * @param numTerms the number of conditional terms
 * @param ... for each term, provide a TR::IlBuilder object and a TR::IlValue object that evaluates a condition (builder is where all the operations to evaluate the condition go, the value is the final result of the condition)
 *
 * Example:
 * TR::IlBuilder *cond1Builder = OrphanBuilder();
 * TR::IlValue *cond1 = cond1Builder->GreaterOrEqual(
 *                      cond1Builder->   Load("x"),
 *                      cond1Builder->   Load("lower"));
 * TR::IlBuilder *cond2Builder = OrphanBuilder();
 * TR::IlValue *cond2 = cond2Builder->LessThan(
 *                      cond2Builder->   Load("x"),
 *                      cond2Builder->   Load("upper"));
 * TR::IlBuilder *inRange = NULL, *outOfRange = NULL;
 * IfAnd(&inRange, &outOfRange, 2, cond1Builder, cond1, cond2Builder, cond2);
 */
void
OMR::IlBuilderRecorder::IfAnd(TR::IlBuilder **allTrueBuilder, TR::IlBuilder **anyFalseBuilder, int32_t numTerms, ...)
   {
   TR::IlBuilder *mergePoint = asIlBuilder()->OrphanBuilder();
   *allTrueBuilder = createBuilderIfNeeded(*allTrueBuilder);
   *anyFalseBuilder = createBuilderIfNeeded(*anyFalseBuilder);

   va_list terms;
   va_start(terms, numTerms);
   for (int32_t t=0;t < numTerms;t++)
      {
      TR::IlBuilder *condBuilder = va_arg(terms, TR::IlBuilder*);
      TR::IlValue *condValue = va_arg(terms, TR::IlValue*);
      AppendBuilder(condBuilder);
      condBuilder->IfCmpEqualZero(anyFalseBuilder, condValue);
      // otherwise fall through to test next term
      }
   va_end(terms);

   // if control gets here, all the provided terms were true
   AppendBuilder(*allTrueBuilder);
   Goto(mergePoint);

   // also need to handle the false case
   AppendBuilder(*anyFalseBuilder);
   Goto(mergePoint);

   AppendBuilder(mergePoint);

   // return state for "this" can get confused by the Goto's in this service
   setComesBack();
   }

/*
 * @brief IfOr service for constructing short circuit OR conditional nests (like the || operator)
 * @param anyTrueBuilder builder containing operations to execute if any conditional test evaluates to true
 * @param allFalseBuilder builder containing operations to execute if all conditional tests are false
 * @param numTerms the number of conditional terms
 * @param ... for each term, provide a TR::IlBuilder object and a TR::IlValue object that evaluates a condition (builder is where all the operations to evaluate the condition go, the value is the final result of the condition)
 *
 * Example:
 * TR::IlBuilder *cond1Builder = asIlBuilder()->OrphanBuilder();
 * TR::IlValue *cond1 = cond1Builder->LessThan(
 *                      cond1Builder->   Load("x"),
 *                      cond1Builder->   Load("lower"));
 * TR::IlBuilder *cond2Builder = asIlBuilder()->OrphanBuilder();
 * TR::IlValue *cond2 = cond2Builder->GreaterOrEqual(
 *                      cond2Builder->   Load("x"),
 *                      cond2Builder->   Load("upper"));
 * TR::IlBuilder *inRange = NULL, *outOfRange = NULL;
 * IfOr(&outOfRange, &inRange, 2, cond1Builder, cond1, cond2Builder, cond2);
 */
void
OMR::IlBuilderRecorder::IfOr(TR::IlBuilder **anyTrueBuilder, TR::IlBuilder **allFalseBuilder, int32_t numTerms, ...)
   {
   TR::IlBuilder *mergePoint = asIlBuilder()->OrphanBuilder();
   *anyTrueBuilder = createBuilderIfNeeded(*anyTrueBuilder);
   *allFalseBuilder = createBuilderIfNeeded(*allFalseBuilder);

   va_list terms;
   va_start(terms, numTerms);
   for (int32_t t=0;t < numTerms-1;t++)
      {
      TR::IlBuilder *condBuilder = va_arg(terms, TR::IlBuilder*);
      TR::IlValue *condValue = va_arg(terms, TR::IlValue*);
      AppendBuilder(condBuilder);
      condBuilder->IfCmpNotEqualZero(anyTrueBuilder, condValue);
      // otherwise fall through to test next term
      }

   // reverse condition on last term so that it can fall through to anyTrueBuilder
   TR::IlBuilder *condBuilder = va_arg(terms, TR::IlBuilder*);
   TR::IlValue *condValue = va_arg(terms, TR::IlValue*);
   AppendBuilder(condBuilder);
   condBuilder->IfCmpEqualZero(allFalseBuilder, condValue);
   va_end(terms);

   // any true term will end up here
   AppendBuilder(*anyTrueBuilder);
   Goto(mergePoint);

   // if control gets here, all the provided terms were false
   AppendBuilder(*allFalseBuilder);
   Goto(mergePoint);

   AppendBuilder(mergePoint);

   // return state for "this" can get confused by the Goto's in this service
   setComesBack();
   }

TR::IlValue *
OMR::IlBuilderRecorder::EqualTo(TR::IlValue *left, TR::IlValue *right)
   {
   TR::IlValue *returnValue=compareOp(TR_cmpEQ, false, left, right);
   TraceIL("IlBuilder[ %p ]::%d is EqualTo %d == %d?\n", this, returnValue->getID(), left->getID(), right->getID());
   ILB_REPLAY("%s = %s->EqualTo(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return returnValue;
   }

void
OMR::IlBuilderRecorder::integerizeAddresses(TR::IlValue **leftPtr, TR::IlValue **rightPtr)
   {
   TR::IlValue *left = *leftPtr;
   if (left->getDataType() == TR::Address)
      *leftPtr = ConvertTo(Word, left);

   TR::IlValue *right = *rightPtr;
   if (right->getDataType() == TR::Address)
      *rightPtr = ConvertTo(Word, right);
   }

TR::IlValue *
OMR::IlBuilderRecorder::UnsignedLessThan(TR::IlValue *left, TR::IlValue *right)
   {
   integerizeAddresses(&left, &right);
   TR::IlValue *returnValue=compareOp(TR_cmpLT, true, left, right);
   TraceIL("IlBuilder[ %p ]::%d is UnsignedLessThan %d < %d?\n", this, returnValue->getID(), left->getID(), right->getID());
   ILB_REPLAY("%s = %s->UnsignedLessThan(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::LessOrEqualTo(TR::IlValue *left, TR::IlValue *right)
   {
   integerizeAddresses(&left, &right);
   TR::IlValue *returnValue=compareOp(TR_cmpLE, false, left, right);
   TraceIL("IlBuilder[ %p ]::%d is LessOrEqualTo %d <= %d?\n", this, returnValue->getID(), left->getID(), right->getID());
   ILB_REPLAY("%s = %s->LessOrEqualTo(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::UnsignedLessOrEqualTo(TR::IlValue *left, TR::IlValue *right)
   {
   integerizeAddresses(&left, &right);
   TR::IlValue *returnValue=compareOp(TR_cmpLE, true, left, right);
   TraceIL("IlBuilder[ %p ]::%d is UnsignedLessOrEqualTo %d <= %d?\n", this, returnValue->getID(), left->getID(), right->getID());
   ILB_REPLAY("%s = %s->UnsignedLessOrEqualTo(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::GreaterThan(TR::IlValue *left, TR::IlValue *right)
   {
   integerizeAddresses(&left, &right);
   TR::IlValue *returnValue=compareOp(TR_cmpGT, false, left, right);
   TraceIL("IlBuilder[ %p ]::%d is GreaterThan %d > %d?\n", this, returnValue->getID(), left->getID(), right->getID());
   ILB_REPLAY("%s = %s->GreaterThan(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::UnsignedGreaterThan(TR::IlValue *left, TR::IlValue *right)
   {
   integerizeAddresses(&left, &right);
   TR::IlValue *returnValue=compareOp(TR_cmpGT, true, left, right);
   TraceIL("IlBuilder[ %p ]::%d is UnsignedGreaterThan %d > %d?\n", this, returnValue->getID(), left->getID(), right->getID());
   ILB_REPLAY("%s = %s->UnsignedGreaterThan(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::GreaterOrEqualTo(TR::IlValue *left, TR::IlValue *right)
   {
   integerizeAddresses(&left, &right);
   TR::IlValue *returnValue=compareOp(TR_cmpGE, false, left, right);
   TraceIL("IlBuilder[ %p ]::%d is GreaterOrEqualTo %d >= %d?\n", this, returnValue->getID(), left->getID(), right->getID());
   ILB_REPLAY("%s = %s->GreaterOrEqualTo(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return returnValue;
   }

TR::IlValue *
OMR::IlBuilderRecorder::UnsignedGreaterOrEqualTo(TR::IlValue *left, TR::IlValue *right)
   {
   integerizeAddresses(&left, &right);
   TR::IlValue *returnValue=compareOp(TR_cmpGE, true, left, right);
   TraceIL("IlBuilder[ %p ]::%d is UnsignedGreaterOrEqualTo %d >= %d?\n", this, returnValue->getID(), left->getID(), right->getID());
   ILB_REPLAY("%s = %s->UnsignedGreaterOrEqualTo(%s, %s);", REPLAY_VALUE(returnValue), REPLAY_BUILDER(this), REPLAY_VALUE(left), REPLAY_VALUE(right));
   return returnValue;
   }

TR::IlValue** 
OMR::IlBuilderRecorder::processCallArgs(TR::Compilation *comp, int numArgs, va_list args)
   {
   TR::IlValue ** argValues = (TR::IlValue **) comp->trMemory()->allocateHeapMemory(numArgs * sizeof(TR::IlValue *));
   for (int32_t a=0;a < numArgs;a++)
      {
      argValues[a] = va_arg(args, TR::IlValue*);
      }
   return argValues;
   }

/*
 * \param numArgs 
 *    Number of actual arguments for the method  plus 1 
 * \param ... 
 *    The list is a computed address followed by the actual arguments
 */
TR::IlValue *
OMR::IlBuilderRecorder::ComputedCall(const char *functionName, int32_t numArgs, ...)
   {
   // TODO: figure out Call REPLAY
   TraceIL("IlBuilder[ %p ]::ComputedCall %s\n", this, functionName);
   va_list args;
   va_start(args, numArgs);
   TR::IlValue **argValues = processCallArgs(_comp, numArgs, args);
   va_end(args);
   TR::ResolvedMethod *resolvedMethod = _methodBuilder->lookupFunction(functionName);
   if (resolvedMethod == NULL && _methodBuilder->RequestFunction(functionName))
      resolvedMethod = _methodBuilder->lookupFunction(functionName);
   TR_ASSERT(resolvedMethod, "Could not identify function %s\n", functionName);

   TR::SymbolReference *methodSymRef = symRefTab()->findOrCreateComputedStaticMethodSymbol(JITTED_METHOD_INDEX, -1, resolvedMethod);
   return genCall(methodSymRef, numArgs, argValues, false /*isDirectCall*/);
   }

/*
 * \param numArgs 
 *    Number of actual arguments for the method  plus 1 
 * \param argValues
 *    the computed address followed by the actual arguments
 */
TR::IlValue *
OMR::IlBuilderRecorder::ComputedCall(const char *functionName, int32_t numArgs, TR::IlValue **argValues)
   {
   // TODO: figure out Call REPLAY
   TraceIL("IlBuilder[ %p ]::ComputedCall %s\n", this, functionName);
   TR::ResolvedMethod *resolvedMethod = _methodBuilder->lookupFunction(functionName);
   if (resolvedMethod == NULL && _methodBuilder->RequestFunction(functionName))
      resolvedMethod = _methodBuilder->lookupFunction(functionName);
   TR_ASSERT(resolvedMethod, "Could not identify function %s\n", functionName);

   TR::SymbolReference *methodSymRef = symRefTab()->findOrCreateComputedStaticMethodSymbol(JITTED_METHOD_INDEX, -1, resolvedMethod);
   return genCall(methodSymRef, numArgs, argValues, false /*isDirectCall*/);
   }

TR::IlValue *
OMR::IlBuilderRecorder::Call(const char *functionName, int32_t numArgs, ...)
   {
   // TODO: figure out Call REPLAY
   TraceIL("IlBuilder[ %p ]::Call %s\n", this, functionName);
   va_list args;
   va_start(args, numArgs);
   TR::IlValue **argValues = processCallArgs(_comp, numArgs, args);
   va_end(args);
   TR::ResolvedMethod *resolvedMethod = _methodBuilder->lookupFunction(functionName);
   if (resolvedMethod == NULL && _methodBuilder->RequestFunction(functionName))
      resolvedMethod = _methodBuilder->lookupFunction(functionName);
   TR_ASSERT(resolvedMethod, "Could not identify function %s\n", functionName);

   TR::SymbolReference *methodSymRef = symRefTab()->findOrCreateStaticMethodSymbol(JITTED_METHOD_INDEX, -1, resolvedMethod);
   return genCall(methodSymRef, numArgs, argValues);
   }

TR::IlValue *
OMR::IlBuilderRecorder::Call(const char *functionName, int32_t numArgs, TR::IlValue ** argValues)
   {
   // TODO: figure out Call REPLAY
   TraceIL("IlBuilder[ %p ]::Call %s\n", this, functionName);
   TR::ResolvedMethod *resolvedMethod = _methodBuilder->lookupFunction(functionName);
   if (resolvedMethod == NULL && _methodBuilder->RequestFunction(functionName))
      resolvedMethod = _methodBuilder->lookupFunction(functionName);
   TR_ASSERT(resolvedMethod, "Could not identify function %s\n", functionName);

   TR::SymbolReference *methodSymRef = symRefTab()->findOrCreateStaticMethodSymbol(JITTED_METHOD_INDEX, -1, resolvedMethod);
   return genCall(methodSymRef, numArgs, argValues);
   }

TR::IlValue *
OMR::IlBuilderRecorder::genCall(TR::SymbolReference *methodSymRef, int32_t numArgs, TR::IlValue ** argValues, bool isDirectCall /* true by default*/)
   {
   TR::DataType returnType = methodSymRef->getSymbol()->castToMethodSymbol()->getMethod()->returnType();
   TR::Node *callNode = TR::Node::createWithSymRef(isDirectCall? TR::ILOpCode::getDirectCall(returnType): TR::ILOpCode::getIndirectCall(returnType), numArgs, methodSymRef);

   // TODO: should really verify argument types here
   int32_t childIndex = 0;
   for (int32_t a=0;a < numArgs;a++)
      {
      TR::IlValue *arg = argValues[a];
      if (arg->getDataType() == TR::Int8 || arg->getDataType() == TR::Int16 || Word == Int64 && arg->getDataType() == TR::Int32)
         arg = ConvertTo(Word, arg);
      callNode->setAndIncChild(childIndex++, loadValue(arg));
      }

   // callNode must be anchored by itasIlBuilder
   genTreeTop(callNode);

   if (returnType != TR::NoType)
      {
      TR::IlValue *returnValue = newValue(callNode->getDataType(), callNode);
      return returnValue;
      }

   return NULL;
   }

/** \brief
 *     The service is used to atomically increase memory location [\p baseAddress + \p offset] by amount of \p value. 
 *
 *  \param value
 *     The amount to increase for the memory location.
 * 
 *  \note
 *	   This service currently only supports Int32/Int64.
 *
 *  \return
 *     The old value at the location [\p baseAddress + \p offset].
 */
TR::IlValue *
OMR::IlBuilderRecorder::AtomicAddWithOffset(TR::IlValue * baseAddress, TR::IlValue * offset, TR::IlValue * value)
   {
   TR_ASSERT(comp()->cg()->supportsAtomicAdd(), "this platform doesn't support AtomicAdd() yet");
   TR_ASSERT(baseAddress->getDataType() == TR::Address, "baseAddress must be TR::Address");
   TR_ASSERT(offset == NULL || offset->getDataType() == TR::Int32 || offset->getDataType() == TR::Int64, "offset must be TR::Int32/64 or NULL");

   //Determine the implementation type and returnType by detecting "value"'s type
   TR::DataType returnType = value->getDataType();
   TR_ASSERT(returnType == TR::Int32 || (returnType == TR::Int64 && TR::Compiler->target.is64Bit()), "AtomicAdd currently only supports Int32/64 values");
   TraceIL("IlBuilder[ %p ]::AtomicAddWithOffset (%d, %d, %d)\n", this, baseAddress->getID(), offset == NULL ? 0 : offset->getID(), value->getID());

   OMR::SymbolReferenceTable::CommonNonhelperSymbol atomicBitSymbol = returnType == TR::Int32 ? TR::SymbolReferenceTable::atomicAdd32BitSymbol : TR::SymbolReferenceTable::atomicAdd64BitSymbol;//lock add
   TR::SymbolReference *methodSymRef = symRefTab()->findOrCreateCodeGenInlinedHelper(atomicBitSymbol); 
   TR::Node *callNode;
   //Evaluator will handle if it's (baseAddress+offset) or baseAddress based on the number of children the call node have 
   callNode = TR::Node::createWithSymRef(TR::ILOpCode::getDirectCall(returnType), offset == NULL ? 2 : 3, methodSymRef);
   callNode->setAndIncChild(0, loadValue(baseAddress));
   if (offset == NULL)
      {
      callNode->setAndIncChild(1, loadValue(value));
      }
   else
      {
      callNode->setAndIncChild(1, loadValue(offset));
      callNode->setAndIncChild(2, loadValue(value));
      }

   TR::IlValue *returnValue = newValue(callNode->getDataType(), callNode);
   return returnValue; 
   }

/** \brief
 *     The service is used to atomically increase memory location \p baseAddress by amount of \p value. 
 *
 *  \param value
 *     The amount to increase for the memory location.
 *
 *  \note
 *     This service currently only supports Int32/Int64. 
 *
 *  \return
 *     The old value at the location \p baseAddress.
 */
TR::IlValue *
OMR::IlBuilderRecorder::AtomicAdd(TR::IlValue * baseAddress, TR::IlValue * value)
   {
   return AtomicAddWithOffset(baseAddress, NULL, value);    
   }


/**
 * \brief
 *  The service is for generating a treetop for transaction begin when the user needs to use transactional memory.
 *  At the end of transaction path, service will automatically generate transaction end instruction.
 *
 * \verbatim
 *   
 *    +---------------------------------+
 *    |<block_$tstart>                  |
 *    |==============                   |
 *    | TSTART                          |
 *    |    |                            |
 *    |    |_ <block_$persistentFailure>|
 *    |    |_ <block_$transientFailure> |
 *    |    |_ <block_$transaction>      |+------------------------+----------------------------------+
 *    |                                 |                         |                                  |
 *    +---------------------------------+                         |                                  |
 *                       |                                        |                                  |
 *                       v                                        V                                  V
 *    +-----------------------------------------+   +-------------------------------+    +-------------------------+     
 *    |<block_$transaction>                     |   |<block_$persistentFailure>     |    |<block_$transientFailure>|      
 *    |====================                     |   |===========================    |    |=========================|
 *    |     add (For illustration, we take add  |   |AtomicAdd (For illustration, we|    |   ...                   |
 *    |     ... as an example. User could       |   |...       take atomicAdd as an |    |   ...                   |
 *    |     ... replace it with other non-atomic|   |...       example. User could  |    |   ...                   |
 *    |     ... operations.)                    |   |...       replace it with other|    |   ...                   |
 *    |     ...                                 |   |...       atomic operations.)  |    |   ...                   |
 *    |     TEND                                |   |...                            |    |   ...                   |
 *    |goto --> block_$merge                    |   |goto->block_$merge             |    |goto->block_$merge       |
 *    +----------------------------------------+    +-------------------------------+    +-------------------------+
 *                      |                                          |                                 |
 *                      |                                          |                                 |
 *                      |------------------------------------------+---------------------------------+
 *                      |
 *                      v             
 *              +----------------+     
 *              | block_$merge   |     
 *              | ============== |   
 *              |      ...       |
 *              +----------------+   
 *
 * \endverbatim 
 *
 * \structure & \param value
 *     tstart
 *       |
 *       ----persistentFailure // This is a permanent failure, try atomic way to do it instead
 *       |
 *       ----transientFailure // Temporary failure, user can retry 
 *       |
 *       ----transaction // Success, use general(non-atomic) way to do it
 *                |
 *                ---- non-atomic operations
 *                |
 *                ---- ...
 *                |
 *                ---- tend
 *
 * \note
 *      If user's platform doesn't support TM, go to persistentFailure path directly/
 *      In this case, current IlBuilder walks around transientFailureBuilder and transactionBuilder
 *      and goes to persistentFailureBuilder.
 *      
 *      _currentBuilder
 *          |
 *          ->Goto()
 *              |   transientFailureBuilder 
 *              |   transactionBuilder
 *              |-->persistentFailurie
 */
void
OMR::IlBuilderRecorder::Transaction(TR::IlBuilder **persistentFailureBuilder, TR::IlBuilder **transientFailureBuilder, TR::IlBuilder **transactionBuilder)
   {   
   //This assertion is to rule out platforms which don't have tstart evaluator yet. 
   TR_ASSERT(comp()->cg()->hasTMEvaluator(), "this platform doesn't support tstart or tfinish evaluator yet");   
    
   //ILB_REPLAY("%s->TransactionBegin(%s, %s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(persistentFailureBuilder), REPLAY_BUILDER(transientFailureBuilder), REPLAY_BUILDER(transactionBuilder)); 
   TraceIL("IlBuilder[ %p ]::transactionBegin %p, %p, %p, %p)\n", this, *persistentFailureBuilder, *transientFailureBuilder, *transactionBuilder);

   appendBlock();

   TR::Block *mergeBlock = emptyBlock();
   *persistentFailureBuilder = createBuilderIfNeeded(*persistentFailureBuilder);
   *transientFailureBuilder = createBuilderIfNeeded(*transientFailureBuilder);
   *transactionBuilder = createBuilderIfNeeded(*transactionBuilder);

   if (!comp()->cg()->getSupportsTM())
      {
      //if user's processor doesn't support TM.
      //we will walk around transaction and transientFailure paths

      Goto(persistentFailureBuilder);

      AppendBuilder(*transactionBuilder);
      AppendBuilder(*transientFailureBuilder);

      AppendBuilder(*persistentFailureBuilder);
      appendBlock(mergeBlock);
      return;
      }

   TR::Node *persistentFailureNode = TR::Node::create(TR::branch, 0, (*persistentFailureBuilder)->getEntry()->getEntry());
   TR::Node *transientFailureNode = TR::Node::create(TR::branch, 0, (*transientFailureBuilder)->getEntry()->getEntry());
   TR::Node *transactionNode = TR::Node::create(TR::branch, 0, (*transactionBuilder)->getEntry()->getEntry());

   TR::Node *tStartNode = TR::Node::create(TR::tstart, 3, persistentFailureNode, transientFailureNode, transactionNode);   
   tStartNode->setSymbolReference(comp()->getSymRefTab()->findOrCreateTransactionEntrySymbolRef(comp()->getMethodSymbol()));
   
   genTreeTop(tStartNode);

   //connecting the block having tstart with persistentFailure's and transaction's blocks 
   cfg()->addEdge(_currentBlock, (*persistentFailureBuilder)->getEntry());
   cfg()->addEdge(_currentBlock, (*transientFailureBuilder)->getEntry());
   cfg()->addEdge(_currentBlock, (*transactionBuilder)->getEntry());

   appendNoFallThroughBlock();
   AppendBuilder(*transientFailureBuilder);
   gotoBlock(mergeBlock);

   AppendBuilder(*persistentFailureBuilder);
   gotoBlock(mergeBlock);

   AppendBuilder(*transactionBuilder);
    
   //ending the transaction at the end of transactionBuilder
   appendBlock();
   TR::Node *tEndNode=TR::Node::create(TR::tfinish,0);
   tEndNode->setSymbolReference(comp()->getSymRefTab()->findOrCreateTransactionExitSymbolRef(comp()->getMethodSymbol()));
   genTreeTop(tEndNode);

   //Three IlBuilders above merged here
   appendBlock(mergeBlock);
   }  


/**
 * Generate XABORT instruction to abort transaction
 */
void
OMR::IlBuilderRecorder::TransactionAbort()
   {
   TraceIL("IlBuilder[ %p ]::transactionAbort", this);
   TR::Node *tAbortNode = TR::Node::create(TR::tabort, 0);
   tAbortNode->setSymbolReference(comp()->getSymRefTab()->findOrCreateTransactionAbortSymbolRef(comp()->getMethodSymbol()));
   genTreeTop(tAbortNode);
   }

void
OMR::IlBuilderRecorder::IfCmpNotEqualZero(TR::IlBuilder **target, TR::IlValue *condition)
   {
   *target = createBuilderIfNeeded(*target);
   IfCmpNotEqualZero(*target, condition);
   }

void
OMR::IlBuilderRecorder::IfCmpNotEqualZero(TR::IlBuilder *target, TR::IlValue *condition)
   {
   TR_ASSERT(target != NULL, "This IfCmpNotEqualZero requires a non-NULL builder object");
   ILB_REPLAY("%s->IfCmpNotEqualZero(%s, %s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(target), REPLAY_VALUE(condition));
   TraceIL("IlBuilder[ %p ]::IfCmpNotEqualZero %d? -> [ %p ] B%d\n", this, condition->getID(), target, target->getEntry()->getNumber());
   ifCmpNotEqualZero(condition, target->getEntry());
   }

void
OMR::IlBuilderRecorder::IfCmpNotEqual(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right)
   {
   *target = createBuilderIfNeeded(*target);
   IfCmpNotEqual(*target, left, right);
   }

void
OMR::IlBuilderRecorder::IfCmpNotEqual(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right)
   {
   TR_ASSERT(target != NULL, "This IfCmpNotEqual requires a non-NULL builder object");
   ILB_REPLAY("%s->IfCmpNotEqual(%s, %s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(target), REPLAY_VALUE(left), REPLAY_VALUE(right));
   TraceIL("IlBuilder[ %p ]::IfCmpNotEqual %d == %d? -> [ %p ] B%d\n", this, left->getID(), right->getID(), target, target->getEntry()->getNumber());
   ifCmpCondition(TR_cmpNE, false, left, right, target->getEntry());
   }

void
OMR::IlBuilderRecorder::IfCmpEqualZero(TR::IlBuilder **target, TR::IlValue *condition)
   {
   *target = createBuilderIfNeeded(*target);
   IfCmpEqualZero(*target, condition);
   }

void
OMR::IlBuilderRecorder::IfCmpEqualZero(TR::IlBuilder *target, TR::IlValue *condition)
   {
   TR_ASSERT(target != NULL, "This IfCmpEqualZero requires a non-NULL builder object");
   ILB_REPLAY("%s->IfCmpEqualZero(%s, %s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(target), REPLAY_VALUE(condition));
   TraceIL("IlBuilder[ %p ]::IfCmpEqualZero %d == 0? -> [ %p ] B%d\n", this, condition->getID(), target, target->getEntry()->getNumber());
   ifCmpEqualZero(condition, target->getEntry());
   }

void
OMR::IlBuilderRecorder::IfCmpEqual(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right)
   {
   *target = createBuilderIfNeeded(*target);
   IfCmpEqual(*target, left, right);
   }

void
OMR::IlBuilderRecorder::IfCmpEqual(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right)
   {
   TR_ASSERT(target != NULL, "This IfCmpEqual requires a non-NULL builder object");
   ILB_REPLAY("%s->IfCmpEqual(%s, %s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(target), REPLAY_VALUE(left), REPLAY_VALUE(right));
   TraceIL("IlBuilder[ %p ]::IfCmpEqual %d == %d? -> [ %p ] B%d\n", this, left->getID(), right->getID(), target, target->getEntry()->getNumber());
   ifCmpCondition(TR_cmpEQ, false, left, right, target->getEntry());
   }

void
OMR::IlBuilderRecorder::IfCmpLessThan(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right)
   {
   *target = createBuilderIfNeeded(*target);
   IfCmpLessThan(*target, left, right);
   }

void
OMR::IlBuilderRecorder::IfCmpLessThan(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right)
   {
   TR_ASSERT(target != NULL, "This IfCmpLessThan requires a non-NULL builder object");
   ILB_REPLAY("%s->IfCmpLessThan(%s, %s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(target), REPLAY_VALUE(left), REPLAY_VALUE(right));
   TraceIL("IlBuilder[ %p ]::IfCmpLessThan %d < %d? -> [ %p ] B%d\n", this, left->getID(), right->getID(), target, target->getEntry()->getNumber());
   ifCmpCondition(TR_cmpLT, false, left, right, target->getEntry());
   }

void
OMR::IlBuilderRecorder::IfCmpUnsignedLessThan(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right)
   {
   *target = createBuilderIfNeeded(*target);
   IfCmpUnsignedLessThan(*target, left, right);
   }

void
OMR::IlBuilderRecorder::IfCmpUnsignedLessThan(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right)
   {
   TR_ASSERT(target != NULL, "This IfCmpUnsignedLessThan requires a non-NULL builder object");
   ILB_REPLAY("%s->IfCmpUnsignedLessThan(%s, %s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(target), REPLAY_VALUE(left), REPLAY_VALUE(right));
   TraceIL("IlBuilder[ %p ]::IfCmpUnsignedLessThan %d < %d? -> [ %p ] B%d\n", this, left->getID(), right->getID(), target, target->getEntry()->getNumber());
   ifCmpCondition(TR_cmpLT, true, left, right, target->getEntry());
   }

void
OMR::IlBuilderRecorder::IfCmpLessOrEqual(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right)
   {
   *target = createBuilderIfNeeded(*target);
   IfCmpLessOrEqual(*target, left, right);
   }

void
OMR::IlBuilderRecorder::IfCmpLessOrEqual(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right)
   {
   TR_ASSERT(target != NULL, "This IfCmpLessOrEqual requires a non-NULL builder object");
   ILB_REPLAY("%s->IfCmpLessOrEqual(%s, %s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(target), REPLAY_VALUE(left), REPLAY_VALUE(right));
   TraceIL("IlBuilder[ %p ]::IfCmpLessOrEqual %d <= %d? -> [ %p ] B%d\n", this, left->getID(), right->getID(), target, target->getEntry()->getNumber());
   ifCmpCondition(TR_cmpLE, false, left, right, target->getEntry());
   }

void
OMR::IlBuilderRecorder::IfCmpUnsignedLessOrEqual(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right)
   {
   *target = createBuilderIfNeeded(*target);
   IfCmpUnsignedLessOrEqual(*target, left, right);
   }

void
OMR::IlBuilderRecorder::IfCmpUnsignedLessOrEqual(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right)
   {
   TR_ASSERT(target != NULL, "This IfCmpUnsignedLessOrEqual requires a non-NULL builder object");
   ILB_REPLAY("%s->IfCmpUnsignedLessOrEqual(%s, %s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(target), REPLAY_VALUE(left), REPLAY_VALUE(right));
   TraceIL("IlBuilder[ %p ]::IfCmpUnsignedLessOrEqual %d <= %d? -> [ %p ] B%d\n", this, left->getID(), right->getID(), target, target->getEntry()->getNumber());
   ifCmpCondition(TR_cmpLE, true, left, right, target->getEntry());
   }

void
OMR::IlBuilderRecorder::IfCmpGreaterThan(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right)
   {
   *target = createBuilderIfNeeded(*target);
   IfCmpGreaterThan(*target, left, right);
   }

void
OMR::IlBuilderRecorder::IfCmpGreaterThan(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right)
   {
   ILB_REPLAY("%s->IfCmpGreaterThan(%s, %s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(target), REPLAY_VALUE(left), REPLAY_VALUE(right));
   TraceIL("IlBuilder[ %p ]::IfCmpGreaterThan %d > %d? -> [ %p ] B%d\n", this, left->getID(), right->getID(), target, target->getEntry()->getNumber());
   ifCmpCondition(TR_cmpGT, false, left, right, target->getEntry());
   }

void
OMR::IlBuilderRecorder::IfCmpUnsignedGreaterThan(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right)
   {
   *target = createBuilderIfNeeded(*target);
   IfCmpUnsignedGreaterThan(*target, left, right);
   }

void
OMR::IlBuilderRecorder::IfCmpUnsignedGreaterThan(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right)
   {
   ILB_REPLAY("%s->IfCmpUnsignedGreaterThan(%s, %s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(target), REPLAY_VALUE(left), REPLAY_VALUE(right));
   TraceIL("IlBuilder[ %p ]::IfCmpUnsignedGreaterThan %d > %d? -> [ %p ] B%d\n", this, left->getID(), right->getID(), target, target->getEntry()->getNumber());
   ifCmpCondition(TR_cmpGT, true, left, right, target->getEntry());
   }

void
OMR::IlBuilderRecorder::IfCmpGreaterOrEqual(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right)
   {
   *target = createBuilderIfNeeded(*target);
   IfCmpGreaterOrEqual(*target, left, right);
   }

void
OMR::IlBuilderRecorder::IfCmpGreaterOrEqual(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right)
   {
   ILB_REPLAY("%s->IfCmpGreaterOrEqual(%s, %s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(target), REPLAY_VALUE(left), REPLAY_VALUE(right));
   TraceIL("IlBuilder[ %p ]::IfCmpGreaterOrEqual %d >= %d? -> [ %p ] B%d\n", this, left->getID(), right->getID(), target, target->getEntry()->getNumber());
   ifCmpCondition(TR_cmpGE, false, left, right, target->getEntry());
   }

void
OMR::IlBuilderRecorder::IfCmpUnsignedGreaterOrEqual(TR::IlBuilder **target, TR::IlValue *left, TR::IlValue *right)
   {
   *target = createBuilderIfNeeded(*target);
   IfCmpUnsignedGreaterOrEqual(*target, left, right);
   }

void
OMR::IlBuilderRecorder::IfCmpUnsignedGreaterOrEqual(TR::IlBuilder *target, TR::IlValue *left, TR::IlValue *right)
   {
   ILB_REPLAY("%s->IfCmpUnsignedGreaterOrEqual(%s, %s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(target), REPLAY_VALUE(left), REPLAY_VALUE(right));
   TraceIL("IlBuilder[ %p ]::IfCmpUnsignedGreaterOrEqual %d >= %d? -> [ %p ] B%d\n", this, left->getID(), right->getID(), target, target->getEntry()->getNumber());
   ifCmpCondition(TR_cmpGE, true, left, right, target->getEntry());
   }

void
OMR::IlBuilderRecorder::ifCmpCondition(TR_ComparisonTypes ct, bool isUnsignedCmp, TR::IlValue *left, TR::IlValue *right, TR::Block *target)
   {
   integerizeAddresses(&left, &right);
   TR::Node *leftNode = loadValue(left);
   TR::Node *rightNode = loadValue(right);
   TR::ILOpCode cmpOpCode(TR::ILOpCode::compareOpCode(leftNode->getDataType(), ct, isUnsignedCmp));
   ifjump(cmpOpCode.convertCmpToIfCmp(),
          leftNode,
          rightNode,
          target);
   appendBlock();
   }

void
OMR::IlBuilderRecorder::ifCmpNotEqualZero(TR::IlValue *condition, TR::Block *target)
   {
   ifCmpCondition(TR_cmpNE, false, condition, zeroForValue(condition), target);
   }

void
OMR::IlBuilderRecorder::ifCmpEqualZero(TR::IlValue *condition, TR::Block *target)
   {
   ifCmpCondition(TR_cmpEQ, false, condition, zeroForValue(condition), target);
   }

void
OMR::IlBuilderRecorder::appendGoto(TR::Block *destBlock)
   {
   gotoBlock(destBlock);
   appendBlock();
   }
#endif

/* Flexible builder for if...then...else structures
 * Basically 3 ways to call it:
 *    1) with then and else paths
 *    2) only a then path
 *    3) only an else path
 * #2 and #3 are similar: the existing path is appended and the proper condition causes branch around to the merge point
 * #1: then path exists "out of line", else path falls through from If and then jumps to merge, followed by then path which
 *     falls through to the merge point
 */
void
OMR::IlBuilderRecorder::IfThenElse(TR::IlBuilder *thenPath, TR::IlBuilder *elsePath, TR::IlValue *condition)
   {
   if (recorder())
      {
      recorder()->BeginStatement(asIlBuilder(), recorder()->STATEMENT_IFTHENELSE);
      recorder()->Builder(thenPath);
      if (elsePath)
         recorder()->Builder(elsePath);
      else
         recorder()->Builder(NULL);
      recorder()->Value(condition);
      recorder()->EndStatement();
      }
   }

void
OMR::IlBuilderRecorder::IfThenElse(TR::IlBuilder **thenPath, TR::IlBuilder **elsePath, TR::IlValue *condition)
   {
   TR_ASSERT(thenPath != NULL || elsePath != NULL, "IfThenElse needs at least one conditional path");

   TR::IlBuilder *bThen=NULL;
   if (thenPath)
      bThen = *thenPath = createBuilderIfNeeded(*thenPath);

   TR::IlBuilder *bElse=NULL;
   if (elsePath)
      bElse = *elsePath = createBuilderIfNeeded(*elsePath);

   IfThenElse(bThen, bElse, condition);
   }

#if 0
void
OMR::IlBuilderRecorder::Switch(const char *selectionVar,
                  TR::IlBuilder **defaultBuilder,
                  uint32_t numCases,
                  int32_t *caseValues,
                  TR::IlBuilder **caseBuilders,
                  bool *caseFallsThrough)
   {
   //ILB_REPLAY_BEGIN();
   TR::IlValue *selectorValue = Load(selectionVar);
   TR_ASSERT(selectorValue->getDataType() == TR::Int32, "Switch only supports selector having type Int32");
   *defaultBuilder = createBuilderIfNeeded(*defaultBuilder);

   TR::Node *defaultNode = TR::Node::createCase(0, (*defaultBuilder)->getEntry()->getEntry());
   TR::Node *lookupNode = TR::Node::create(TR::lookup, numCases + 2, loadValue(selectorValue), defaultNode);

   // get the lookup tree into this builder, even though we haven't completely filled it in yet
   genTreeTop(lookupNode);
   TR::Block *switchBlock = _currentBlock;

   // make sure no fall through edge created from the lookup
   appendNoFallThroughBlock();

   TR::IlBuilder *breakBuilder = asIlBuilder()->OrphanBuilder();

   // each case handler is a sequence of two builder objects: first the one passed in via caseBuilder (or will be passed
   //   back via caseBuilders, and second a builder that branches to the breakBuilder (unless this case falls through)
   for (int32_t c=0;c < numCases;c++)
      {
      int32_t value = caseValues[c];
      TR::IlBuilder *handler = NULL;
      if (!caseFallsThrough[c])
         {
         handler = asIlBuilder()->OrphanBuilder();

         caseBuilders[c] = createBuilderIfNeeded(caseBuilders[c]);
         handler->AppendBuilder(caseBuilders[c]);

         // handle "break" with a separate builder so user can add whatever they want into caseBuilders[c]
         TR::IlBuilder *branchToBreak = asIlBuilder()->OrphanBuilder();
         branchToBreak->Goto(&breakBuilder);
         handler->AppendBuilder(branchToBreak);
         }
      else
         {
         caseBuilders[c] = createBuilderIfNeeded(caseBuilders[c]);
         handler = caseBuilders[c];
         }

      TR::Block *caseBlock = handler->getEntry();
      cfg()->addEdge(switchBlock, caseBlock);
      AppendBuilder(handler);

      TR::Node *caseNode = TR::Node::createCase(0, caseBlock->getEntry(), value);
      lookupNode->setAndIncChild(c+2, caseNode);
      }

   cfg()->addEdge(switchBlock, (*defaultBuilder)->getEntry());
   AppendBuilder(*defaultBuilder);

   AppendBuilder(breakBuilder);
   //ILB_REPLAY_END();
   }

void
OMR::IlBuilder::Switch(const char *selectionVar,
                  TR::IlBuilder **defaultBuilder,
                  uint32_t numCases,
                  ...)
   {
   //TODO figure out switch replay
   // ILB_REPLAY("%s->IfCmpGreaterThan(%s, %s);", REPLAY_BUILDER(this), REPLAY_BUILDER(target), REPLAY_VALUE(left), REPLAY_VALUE(right));
   int32_t *caseValues = (int32_t *) _comp->trMemory()->allocateHeapMemory(numCases * sizeof(int32_t));
   TR_ASSERT(0 != caseValues, "out of memory");

   TR::IlBuilder **caseBuilders = (TR::IlBuilder **) _comp->trMemory()->allocateHeapMemory(numCases * sizeof(TR::IlBuilder *));
   TR_ASSERT(0 != caseBuilders, "out of memory");

   bool *caseFallsThrough = (bool *) _comp->trMemory()->allocateHeapMemory(numCases * sizeof(bool));
   TR_ASSERT(0 != caseFallsThrough, "out of memory");

   va_list cases;
   va_start(cases, numCases);
   for (int32_t c=0;c < numCases;c++)
      {
      caseValues[c] = (int32_t) va_arg(cases, int);
      caseBuilders[c] = *(TR::IlBuilder **) va_arg(cases, TR::IlBuilder **);
      caseFallsThrough[c] = (bool) va_arg(cases, int);
      }
   va_end(cases);

   Switch(selectionVar, defaultBuilder, numCases, caseValues, caseBuilders, caseFallsThrough);

   // if Switch created any new builders, we need to put those back into the arguments passed into this Switch call
   va_start(cases, numCases);
   for (int32_t c=0;c < numCases;c++)
      {
      int throwawayValue = va_arg(cases, int);
      TR::IlBuilder **caseBuilder = va_arg(cases, TR::IlBuilder **);
      (*caseBuilder) = caseBuilders[c];
      int throwAwayFallsThrough = va_arg(cases, int);
      }
   va_end(cases);
   }
#endif

void
OMR::IlBuilderRecorder::ForLoop(bool countsUp,
                                const char *indVar,
                                TR::IlBuilder *bLoop,
                                TR::IlBuilder *bBreak,
                                TR::IlBuilder *bContinue,
                                TR::IlValue *initial,
                                TR::IlValue *end,
                                TR::IlValue *increment)
   {
   TR::JitBuilderRecorder *rec = recorder();
   if (rec)
      {
      rec->BeginStatement(asIlBuilder(), rec->STATEMENT_FORLOOP);
      rec->Number((int8_t)countsUp);
      rec->String(indVar);
      rec->Builder(bLoop);
      rec->Builder(bBreak);
      rec->Builder(bContinue);
      rec->Value(initial);
      rec->Value(end);
      rec->Value(increment);
      rec->EndStatement();
      }
   }

void
OMR::IlBuilderRecorder::ForLoop(bool countsUp,
                                const char *indVar,
                                TR::IlBuilder **loopCode,
                                TR::IlBuilder **breakBuilder,
                                TR::IlBuilder **continueBuilder,
                                TR::IlValue *initial,
                                TR::IlValue *end,
                                TR::IlValue *increment)
   {
   TR_ASSERT(loopCode != NULL, "ForLoop needs to have loopCode builder");
   TR::IlBuilder *bLoop = *loopCode = createBuilderIfNeeded(*loopCode);

   TR::IlBuilder *bBreak = NULL;
   if (breakBuilder)
      {
      TR_ASSERT(*breakBuilder == NULL, "ForLoop returns breakBuilder, cannot provide breakBuilder as input");
      bBreak = *breakBuilder = asIlBuilder()->OrphanBuilder();
      }

   TR::IlBuilder *bContinue = NULL;
   if (continueBuilder)
      {
      TR_ASSERT(*continueBuilder == NULL, "ForLoop returns continueBuilder, cannot provide continueBuilder as input");
      bContinue = *continueBuilder = asIlBuilder()->OrphanBuilder();
      }

   ForLoop(countsUp, indVar, *loopCode, bBreak, bContinue, initial, end, increment);
   }

#if 0
void
OMR::IlBuilderRecorder::DoWhileLoop(const char *whileCondition, TR::IlBuilder **body, TR::IlBuilder **breakBuilder, TR::IlBuilder **continueBuilder)
   {
   //ILB_REPLAY_BEGIN();

   methodSymbol()->setMayHaveLoops(true);
   TR_ASSERT(body != NULL, "doWhileLoop needs to have a body");

   if (!_methodBuilder->symbolDefined(whileCondition))
      _methodBuilder->defineValue(whileCondition, Int32);

   *body = createBuilderIfNeeded(*body);
   TraceIL("IlBuilder[ %p ]::DoWhileLoop do body B%d while %s\n", this, (*body)->getEntry()->getNumber(), whileCondition);

   AppendBuilder(*body);
   TR::IlBuilder *loopContinue = NULL;

   if (continueBuilder)
      {
      TR_ASSERT(*continueBuilder == NULL, "DoWhileLoop returns continueBuilder, cannot provide continueBuilder as input");
      loopContinue = *continueBuilder = asIlBuilder()->OrphanBuilder();
      }
   else
      loopContinue = asIlBuilder()->OrphanBuilder();

   AppendBuilder(loopContinue);
   loopContinue->IfCmpNotEqualZero(body,
   loopContinue->   Load(whileCondition));

   if (breakBuilder)
      {
      TR_ASSERT(*breakBuilder == NULL, "DoWhileLoop returns breakBuilder, cannot provide breakBuilder as input");
      *breakBuilder = asIlBuilder()->OrphanBuilder();
      AppendBuilder(*breakBuilder);
      }

   // make sure any subsequent operations go into their own block *after* the loop
   appendBlock();

   //ILB_REPLAY_END();
   //ILB_REPLAY("%s->DoWhileLoop(%s, %s, %s, %s);",
          //REPLAY_BUILDER(this),
          //whileCondition,
          //REPLAY_PTRTOBUILDER(body),
          //REPLAY_PTRTOBUILDER(breakBuilder),
          //REPLAY_PTRTOBUILDER(continueBuilder));
   }

void
OMR::IlBuilderRecorder::WhileDoLoop(const char *whileCondition, TR::IlBuilder **body, TR::IlBuilder **breakBuilder, TR::IlBuilder **continueBuilder)
   {
   //ILB_REPLAY_BEGIN();

   methodSymbol()->setMayHaveLoops(true);
   TR_ASSERT(body != NULL, "WhileDo needs to have a body");
   TraceIL("IlBuilder[ %p ]::WhileDoLoop while %s do body %p\n", this, whileCondition, *body);

   TR::IlBuilder *done = asIlBuilder()->OrphanBuilder();
   if (breakBuilder)
      {
      TR_ASSERT(*breakBuilder == NULL, "WhileDoLoop returns breakBuilder, cannot provide breakBuilder as input");
      *breakBuilder = done;
      }

   TR::IlBuilder *loopContinue = asIlBuilder()->OrphanBuilder();
   if (continueBuilder)
      {
      TR_ASSERT(*continueBuilder == NULL, "WhileDoLoop returns continueBuilder, cannot provide continueBuilder as input");
      *continueBuilder = loopContinue;
      }

   AppendBuilder(loopContinue);
   loopContinue->IfCmpEqualZero(&done,
   loopContinue->   Load(whileCondition));

   *body = createBuilderIfNeeded(*body);
   AppendBuilder(*body);

   Goto(&loopContinue);
   setComesBack(); // this goto is on one particular flow path, doesn't mean every path does a goto

   AppendBuilder(done);

   //ILB_REPLAY_END();
   //ILB_REPLAY("%s->WhileDoLoop(%s, %s, %s, %s);",
          //REPLAY_BUILDER(this),
          //whileCondition,
          //REPLAY_PTRTOBUILDER(body),
          //REPLAY_PTRTOBUILDER(breakBuilder),
          //REPLAY_PTRTOBUILDER(continueBuilder));
   }
#endif
