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


#ifndef OMR_JITBUILDERRECORDER_INCL
#define OMR_JITBUILDERRECORDER_INCL


#ifndef TR_JITBUILDERRECORDER_DEFINED
#define TR_JITBUILDERRECORDER_DEFINED
#define PUT_OMR_JITBUILDERRECORDER_INTO_TR
#endif


#include <iostream>
#include <fstream>
#include <map>

namespace TR { class IlBuilder; }
namespace TR { class MethodBuilderRecorder; }
namespace TR { class IlType; }
namespace TR { class IlValue; }

namespace OMR
{

class JitBuilderRecorder
   {
   public:
   const int16_t VERSION_MAJOR=0;
   const int16_t VERSION_MINOR=0;
   const int16_t VERSION_PATCH=0;
   const char *RECORDER_SIGNATURE = "JBIL";
   const char *JBIL_COMPLETE      = "Done";

   typedef uint32_t                      TypeID;
   typedef std::map<const void *,TypeID> TypeMapID;

   JitBuilderRecorder(const TR::MethodBuilderRecorder *mb);
   virtual ~JitBuilderRecorder();

   /**
    * @brief Subclasses override these functions to record to different output formats
    */
   virtual void Close()                                       { }
   virtual void String(const char * const string)             { }
   virtual void Number(int8_t num)                            { }
   virtual void Number(int16_t num)                           { }
   virtual void Number(int32_t num)                           { }
   virtual void Number(int64_t num)                           { }
   virtual void Number(float num)                             { }
   virtual void Number(double num)                            { }
   virtual void ID(TypeID id)                                 { }
   virtual void Statement(const char *s)                      { }
   virtual void Type(const TR::IlType *type)                  { }
   virtual void Value(const TR::IlValue *v)                   { }
   virtual void Builder(const TR::IlBuilder *b)               { }
   virtual void Location(const void * location)               { }

   virtual void BeginStatement(const TR::IlBuilder *b, const char *s);
   virtual void BeginStatement(const char *s);
   virtual void EndStatement()                                { }

   void StoreID(const void *ptr);
   bool EnsureAvailableID(const void *ptr);

   /**
    * @brief constant strings used to define output format
    */
   const char *STATEMENT_DEFINENAME                 = "DefineName";
   const char *STATEMENT_DEFINEFILE                 = "DefineFile";
   const char *STATEMENT_DEFINELINESTRING           = "DefineLineString";
   const char *STATEMENT_DEFINELINENUMBER           = "DefineLineNumber";
   const char *STATEMENT_DEFINEPARAMETER            = "DefineParameter";
   const char *STATEMENT_DEFINEARRAYPARAMETER       = "DefineArrayParameter";
   const char *STATEMENT_DEFINERETURNTYPE           = "DefineReturnType";
   const char *STATEMENT_DEFINELOCAL                = "DefineLocal";
   const char *STATEMENT_DEFINEMEMORY               = "DefineMemory";
   const char *STATEMENT_DEFINEFUNCTION             = "DefineFunction";
   const char *STATEMENT_DEFINESTRUCT               = "DefineStruct";
   const char *STATEMENT_DEFINEUNION                = "DefineUnion";
   const char *STATEMENT_DEFINEFIELD                = "DefineField";
   const char *STATEMENT_PRIMITIVETYPE              = "PrimitiveType";
   const char *STATEMENT_POINTERTYPE                = "PointerType";
   const char *STATEMENT_NEWMETHODBUILDER           = "NewMethodBuilder";
   const char *STATEMENT_NEWILBUILDER               = "NewIlBuilder";
   const char *STATEMENT_NEWBYTECODEBUILDER         = "NewBytecodeBuilder";
   const char *STATEMENT_ALLLOCALSHAVEBEENDEFINED   = "AllLocalsHaveBeenDefined";
   const char *STATEMENT_NULLADDRESS                = "NullAddress";
   const char *STATEMENT_CONSTINT8                  = "ConstInt8";
   const char *STATEMENT_CONSTINT16                 = "ConstInt16";
   const char *STATEMENT_CONSTINT32                 = "ConstInt32";
   const char *STATEMENT_CONSTINT64                 = "ConstInt64";
   const char *STATEMENT_CONSTFLOAT                 = "ConstFloat";
   const char *STATEMENT_CONSTDOUBLE                = "ConstDouble";
   const char *STATEMENT_CONSTSTRING                = "ConstString";
   const char *STATEMENT_CONSTADDRESS               = "ConstAddress";
   const char *STATEMENT_INDEXAT                    = "IndexAt";
   const char *STATEMENT_LOAD                       = "Load";
   const char *STATEMENT_LOADAT                     = "LoadAt";
   const char *STATEMENT_LOADINDIRECT               = "LoadIndirect";
   const char *STATEMENT_STORE                      = "Store";
   const char *STATEMENT_STOREOVER                  = "StoreOver";
   const char *STATEMENT_STOREAT                    = "StoreAt";
   const char *STATEMENT_STOREINDIRECT              = "StoreIndirect";
   const char *STATEMENT_CREATELOCALARRAY           = "CreateLocalArray";
   const char *STATEMENT_CREATELOCALSTRUCT          = "CreateLocalStruct";
   const char *STATEMENT_VECTORLOAD                 = "VectorLoad";
   const char *STATEMENT_VECTORLOADAT               = "VectorLoadAt";
   const char *STATEMENT_VECTORSTORE                = "VectorStore";
   const char *STATEMENT_VECTORSTOREAT              = "VectorStoreAt";
   const char *STATEMENT_STRUCTFIELDINSTANCEADDRESS = "StructFieldInstance";
   const char *STATEMENT_UNIONFIELDINSTANCEADDRESS  = "UnionFieldInstance";
   const char *STATEMENT_CONVERTTO                  = "ConvertTo";
   const char *STATEMENT_UNSIGNEDCONVERTTO          = "UnsignedConvertTo";
   const char *STATEMENT_ADD                        = "Add";
   const char *STATEMENT_SUB                        = "Sub";
   const char *STATEMENT_LESSTHAN                   = "LessThan";
   const char *STATEMENT_NOTEQUALTO                 = "NotEqualTo";
   const char *STATEMENT_APPENDBUILDER              = "AppendBuilder";
   const char *STATEMENT_APPENDBYTECODEBUILDER      = "AppendBytecodeBuilder";
   const char *STATEMENT_GOTO                       = "Goto";
   const char *STATEMENT_RETURN                     = "Return";
   const char *STATEMENT_RETURNVALUE                = "ReturnValue";
   const char *STATEMENT_IFTHENELSE                 = "IfThenElse";
   const char *STATEMENT_FORLOOP                    = "ForLoop";

   protected:

   /*
    * @brief constant strings only used internally by recorders
    */
   const char *STATEMENT_ID16BIT                    = "ID16BIT";
   const char *STATEMENT_ID32BIT                    = "ID32BIT";

   void start();
   bool knownID(const void *ptr);
   TypeID lookupID(const void *ptr);
   void ensureStatementDefined(const char *s);
   void end();

   TypeID getNewID();
   TypeID myID();

   const TR::MethodBuilderRecorder * _mb;
   TypeID                            _nextID;
   TypeMapID                         _idMap;
   uint8_t                           _idSize;
   };

} // namespace OMR


#if defined(PUT_OMR_JITBUILDERRECORDER_INTO_TR)

namespace TR
{
   class JitBuilderRecorder : public OMR::JitBuilderRecorder
      {
      public:
         JitBuilderRecorder(const TR::MethodBuilderRecorder *mb)
            : OMR::JitBuilderRecorder(mb)
            { }
         virtual ~JitBuilderRecorder()
            { }
      };

} // namespace TR

#endif // defined(PUT_OMR_JITBUILDERRECORDER_INTO_TR)

#endif // !defined(OMR_JITBUILDERRECORDER_INCL)
