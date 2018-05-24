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
 *******************************************************************************/

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>

#include "infra/Assert.hpp"
#include "ilgen/JitBuilderReplayTextFile.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "il/DataTypes.hpp"

OMR::JitBuilderReplayTextFile::JitBuilderReplayTextFile(const TR::MethodBuilder *mb, const char *fileName)
   : TR::JitBuilderReplay(mb), _file(fileName, std::fstream::in)
   {
   start();
   }

void
OMR::JitBuilderReplayTextFile::Close()
   {
   end();
   _file.close();
   }

void
OMR::JitBuilderReplayTextFile::VerifyRecordedType()
   {
   std::string line;
   std::getline(_file, line);
   }

void
OMR::JitBuilderReplayTextFile::HandleMethodBuilderConstruction()
   {
   mapTypeIDToPointer(2, _mb);

   std::string line;
   while (std::getline(_file, line))
      {
      std::string firstToken = line.substr(0, line.find(' '));
      if ("Def" == firstToken)
         {
         std::string idToken = line.substr(firstToken.size() +1 );
         std::string ident = idToken.substr(0, idToken.find(' '));
         std::string::size_type open = idToken.find('[') + 1;
         std::string::size_type close = idToken.find(']');
         std::string value = idToken.substr(open, close - open);

         TypeID id = GetIDFromToken(ident);
         const char * valueChars = getStatementFromString(value);

         mapTypeIDToPointer(id, valueChars);
         }
      else if ('B' == firstToken.at(0))
         {
         TypeID builderID = GetIDFromToken(firstToken);
         TypeMapID::iterator it = _idMap.find(builderID);
         if (it == _idMap.end())
            TR_ASSERT_FATAL(0, "Unexpected ID %u for %s not defined", builderID, firstToken.c_str());
         TR::MethodBuilder *mb = (TR::MethodBuilder *)it->second;

         std::string idToken = line.substr(firstToken.size() + 1 );
         std::string ident = idToken.substr(0, idToken.find(' '));

         std::string::size_type open = idToken.find('[') + 1;
         std::string::size_type close = idToken.find(']');
         std::string value = idToken.substr(open, close - open);

         TypeID id = GetIDFromToken(ident);

         const char * ref = getCharsFromTypeID(id);

         if (0 == strncmp("DefineLineString", ref, 16))
            {
            mb->DefineLine(strdup(value.c_str()));
            }
         else if (0 == strncmp("DefineFile", ref, 10))
            {
            mb->DefineFile(strdup(value.c_str()));
            }
         else if (0 == strncmp("DefineName", ref, 10))
            {
            mb->DefineName(strdup(value.c_str()));
            }
         else if (0 == strncmp("DefineParameter", ref, strlen("DefineParameter")))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string typeString = leftover.substr(0, leftover.find(' '));
            TypeID typeID = GetIDFromToken(typeString);
            TR::IlType *type = getTypeFromTypeID(typeID);

            mb->DefineParameter(strdup(value.c_str()), type);
            }
         else if (0 == strncmp("DefineReturnType", ref, 16))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string typeString = leftover.substr(0, leftover.find(' '));
            TypeID typeID = GetIDFromToken(typeString);
            TR::IlType *type = getTypeFromTypeID(typeID);

            mb->DefineReturnType(type);
            }
         else if (0 == strncmp(STATEMENT_PRIMITIVETYPE, ref, 13))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string typeString = leftover.substr(0, leftover.find(' '));
            TypeID typeID = GetIDFromToken(typeString);

            std::string valueString = leftover.substr(typeString.size() + 1);
            valueString = valueString.substr(0, valueString.find(' '));

            TR::DataType dt((TR::DataTypes)atoi(valueString.c_str()));
            TR::IlType *type = mb->typeDictionary()->PrimitiveType(dt);

            mapTypeIDToPointer(typeID, type);
            }
         else if(0 == strncmp(STATEMENT_ILGENERATIONBEGINNING, ref, 22))
            {
            break;
            }
         }
      }
   }

OMR::JitBuilderReplay::TypeID
OMR::JitBuilderReplayTextFile::GetIDFromToken(std::string token)
   {
   return (TypeID)atoi(token.substr(1).c_str());
   }

const char *
OMR::JitBuilderReplayTextFile::GetStringFromToken(std::string token)
   {
   return NULL;
   }

void
OMR::JitBuilderReplayTextFile::mapTypeIDToPointer(OMR::JitBuilderReplay::TypeID id, const void * pointer)
   {
    TypeMapID::iterator it = _idMap.find(id);
    if (it != _idMap.end())
       TR_ASSERT_FATAL(0, "Unexpected ID %zu already defined", id);

    _idMap.insert(std::make_pair(id, pointer));
   }

TR::IlType *
OMR::JitBuilderReplayTextFile::getTypeFromTypeID(OMR::JitBuilderReplay::TypeID id)
   {
   TypeMapID::iterator it = _idMap.find(id);
   if (it == _idMap.end())
      TR_ASSERT_FATAL(0, "Unexpected type ID %u", id);
   return (TR::IlType *)it->second;
   }

TR::IlValue *
OMR::JitBuilderReplayTextFile::getValueFromTypeID(OMR::JitBuilderReplay::TypeID id)
   {
   TypeMapID::iterator it = _idMap.find(id);
   if (it == _idMap.end())
      TR_ASSERT_FATAL(0, "Unexpected value ID %u", id);
   return (TR::IlValue *)it->second;
   }

TR::IlBuilder *
OMR::JitBuilderReplayTextFile::getBuilderFromTypeID(OMR::JitBuilderReplay::TypeID id)
   {
   TypeMapID::iterator it = _idMap.find(id);
   if (it == _idMap.end())
      TR_ASSERT_FATAL(0, "Unexpected builder ID %u", id);
   return (TR::IlBuilder *)it->second;
   }

const char *
OMR::JitBuilderReplayTextFile::getCharsFromTypeID(OMR::JitBuilderReplay::TypeID id)
   {
    TypeMapID::iterator it = _idMap.find(id);
    if (it == _idMap.end())
       TR_ASSERT_FATAL(0, "Unexpected char ID %u", id);
    return (const char *)it->second;
   }

const char *
OMR::JitBuilderReplayTextFile::getStatementFromString(std::string statementString)
   {
   const char *valueChars = NULL;

   if (STATEMENT_DEFINENAME == statementString)
      {
      valueChars = STATEMENT_DEFINENAME;
      }
   else if (STATEMENT_DEFINEFILE == statementString)
      {
      valueChars = STATEMENT_DEFINEFILE;
      }
   else if (STATEMENT_DEFINELINESTRING == statementString)
      {
      valueChars = STATEMENT_DEFINELINESTRING;
      }
   else if (STATEMENT_DEFINELINENUMBER == statementString)
      {
      valueChars = STATEMENT_DEFINELINENUMBER;
      }
   else if (STATEMENT_DEFINEPARAMETER == statementString)
      {
      valueChars = STATEMENT_DEFINEPARAMETER;
      }
   else if (STATEMENT_DEFINEARRAYPARAMETER == statementString)
      {
      valueChars = STATEMENT_DEFINEARRAYPARAMETER;
      }
   else if (STATEMENT_PRIMITIVETYPE == statementString)
      {
      valueChars = STATEMENT_PRIMITIVETYPE;
      }
   else if (STATEMENT_ILGENERATIONBEGINNING == statementString)
      {
      valueChars = STATEMENT_ILGENERATIONBEGINNING;
      }
   else if (STATEMENT_NEWMETHODBUILDER == statementString)
      {
      valueChars = STATEMENT_NEWMETHODBUILDER;
      }
   else if (STATEMENT_DEFINERETURNTYPE == statementString)
      {
      valueChars = STATEMENT_DEFINERETURNTYPE;
      }
   else if (STATEMENT_LOAD == statementString)
      {
	   valueChars = STATEMENT_LOAD;
      }
   else if (STATEMENT_CONSTINT32 == statementString)
      {
      valueChars = STATEMENT_CONSTINT32;
      }
   else if (STATEMENT_ADD == statementString)
      {
      valueChars = STATEMENT_ADD;
      }
   else if (STATEMENT_RETURNVALUE == statementString)
      {
      valueChars = STATEMENT_RETURNVALUE;
      }
   else if (STATEMENT_LESSTHAN == statementString)
      {
      valueChars = STATEMENT_LESSTHAN;
      }
   else if (STATEMENT_NEWILBUILDER == statementString)
      {
      valueChars = STATEMENT_NEWILBUILDER;
      }
   else if (STATEMENT_IFTHENELSE == statementString)
      {
      valueChars = STATEMENT_IFTHENELSE;
      }
   else if (STATEMENT_STORE == statementString)
      {
      valueChars = STATEMENT_STORE;
      }
   else if (STATEMENT_FORLOOP == statementString)
      {
      valueChars = STATEMENT_FORLOOP;
      }
   else if (STATEMENT_DEFINELOCAL == statementString)
       {
       valueChars = STATEMENT_DEFINELOCAL;
       }
   else if (STATEMENT_SUB == statementString)
      {
      valueChars = STATEMENT_SUB;
      }
   else if (STATEMENT_CALL == statementString)
      {
      valueChars = STATEMENT_CALL;
      }
   else if (STATEMENT_CONVERTTO == statementString)
      {
      valueChars = STATEMENT_CONVERTTO;
      }
   else
      {
      TR_ASSERT_FATAL(0, "Unexpected statement %s", statementString.c_str());
      }

   return valueChars;
   }

void
OMR::JitBuilderReplayTextFile::HandleMethodBuilderBuildIL()
   {
   std::string line;
   while (std::getline(_file, line))
      {
      std::string firstToken = line.substr(0, line.find(' '));
      if ("Def" == firstToken)
         {
         std::string idToken = line.substr(firstToken.size() +1 );
         std::string ident = idToken.substr(0, idToken.find(' '));
         std::string::size_type open = idToken.find('[') + 1;
         std::string::size_type close = idToken.find(']');
         std::string value = idToken.substr(open, close - open);

         TypeID id = GetIDFromToken(ident);
         const char * valueChars = getStatementFromString(value);

         mapTypeIDToPointer(id, valueChars);
         }
      else if ('B' == firstToken.at(0))
         {
         TypeID builderID = GetIDFromToken(firstToken);
         TR::IlBuilder *builder = getBuilderFromTypeID(builderID);

         std::string idToken = line.substr(firstToken.size() + 1 );
         std::string ident = idToken.substr(0, idToken.find(' '));

         std::string::size_type open = idToken.find('[') + 1;
         std::string::size_type close = idToken.find(']');
         std::string value = idToken.substr(open, close - open);

         TypeID id = GetIDFromToken(ident);
         const char *ref = getCharsFromTypeID(id);

         if (0 == strncmp(STATEMENT_LOAD, ref, strlen(STATEMENT_LOAD)))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string typeString = leftover.substr(0, leftover.find(' '));
            TypeID typeID = GetIDFromToken(typeString);

            TR::IlValue *ilValue = builder->Load(strdup(value.c_str()));

            mapTypeIDToPointer(typeID, ilValue);
            }
         else if (0 == strncmp(STATEMENT_CONSTINT32, ref, strlen(STATEMENT_CONSTINT32)))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string typeString = leftover.substr(0, leftover.find(' '));
            TypeID typeID = GetIDFromToken(typeString);

            std::string valueString = leftover.substr(typeString.size() + 1);
            valueString = valueString.substr(0, valueString.find(' '));

            TR::IlValue *ilValue = builder->ConstInt32((int32_t)atoi(valueString.c_str()));

            mapTypeIDToPointer(typeID, ilValue);
            }
         else if (0 == strncmp(STATEMENT_ADD, ref, strlen(STATEMENT_ADD)))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string resultString = leftover.substr(0, leftover.find(' '));

            TypeID resultID = GetIDFromToken(resultString);

            std::string leftover2 = leftover.substr(resultString.size() + 1);
            std::string value1 = leftover2.substr(0, leftover2.find(' '));
            TypeID value1ID = GetIDFromToken(value1);

            std::string value2 = leftover2.substr(value1.size() + 1);
            value2 = value2.substr(0, value2.find(' '));
            TypeID value2ID = GetIDFromToken(value2);

            TR::IlValue * val1 = getValueFromTypeID(value1ID);
            TR::IlValue * val2 = getValueFromTypeID(value2ID);

            TR::IlValue *result = builder->Add(val1, val2);

            mapTypeIDToPointer(resultID, result);
            }
         else if (0 == strncmp(STATEMENT_RETURNVALUE, ref, strlen(STATEMENT_RETURNVALUE)))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string valueString = leftover.substr(0, leftover.find(' '));
            TypeID typeID = GetIDFromToken(valueString);

            builder->Return(getValueFromTypeID(typeID));
            }
         else if (0 == strncmp(STATEMENT_LESSTHAN, ref, strlen(STATEMENT_LESSTHAN)))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string resultString = leftover.substr(0, leftover.find(' '));

            TypeID resultID = GetIDFromToken(resultString);

            std::string leftover2 = leftover.substr(resultString.size() + 1);
            std::string value1 = leftover2.substr(0, leftover2.find(' '));
            TypeID value1ID = GetIDFromToken(value1);

            std::string value2 = leftover2.substr(value1.size() + 1);
            value2 = value2.substr(0, value2.find(' '));
            TypeID value2ID = GetIDFromToken(value2);

            TR::IlValue * val1 = getValueFromTypeID(value1ID);
            TR::IlValue * val2 = getValueFromTypeID(value2ID);

            TR::IlValue *result = builder->LessThan(val1, val2);

            mapTypeIDToPointer(resultID, result);
            }
         else if (0 == strncmp(STATEMENT_NEWILBUILDER, ref, strlen(STATEMENT_NEWILBUILDER)))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string valueString = leftover.substr(0, leftover.find(' '));
            TypeID newBuilderID = GetIDFromToken(valueString);

            IlBuilder *newBuilder = builder->OrphanBuilder();

             mapTypeIDToPointer(newBuilderID, newBuilder);
            }
         else if (0 == strncmp(STATEMENT_IFTHENELSE, ref, strlen(STATEMENT_IFTHENELSE)))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string thenPathString = leftover.substr(0, leftover.find(' '));

            TypeID thenPathID = GetIDFromToken(thenPathString);
            TR::IlBuilder *thenPath = getBuilderFromTypeID(thenPathID);

            std::string leftover2 = leftover.substr(thenPathString.size() + 1);
            std::string elsePathString = leftover2.substr(0, leftover2.find(' '));
            TypeID elsePathID = GetIDFromToken(elsePathString);

            std::string valuestring = leftover2.substr(elsePathString.size() + 1);
            valuestring = valuestring.substr(0, valuestring.find(' '));
            TypeID valueID = GetIDFromToken(valuestring);

            TR::IlBuilder * elsePath = NULL;
            TR::IlBuilder **elsePathRef = NULL;
            if (elsePathString != "Def")
               {
               elsePath = getBuilderFromTypeID(elsePathID);
               elsePathRef = &elsePath;
               }

            builder->IfThenElse(&thenPath, elsePathRef, getValueFromTypeID(valueID));
            }
         else if (0 == strncmp(STATEMENT_STORE, ref, strlen(STATEMENT_STORE)))
            {
            std::string valueString = idToken.substr(idToken.find("]") + 3);
            TypeID valueID = GetIDFromToken(valueString);

            builder->Store(strdup(value.c_str()), getValueFromTypeID(valueID));
            }
         else if (0 == strncmp(STATEMENT_FORLOOP, ref, strlen(STATEMENT_FORLOOP)))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string countsUpString = leftover.substr(0, leftover.find(' '));
            bool countsUp = false;
            if ("1" == countsUpString)
               {
               countsUp = true;
               }

            leftover = idToken.substr(idToken.find("]") + 3);
            std::string bodystring = leftover.substr(0, leftover.find(" "));
            TypeID bodybuilderID = GetIDFromToken(bodystring);
            TR::IlBuilder *bodybuilder = getBuilderFromTypeID(bodybuilderID);

            leftover = leftover.substr(bodystring.size() + 1);
            std::string breakstring = leftover.substr(0, leftover.find(" "));
            TR::IlBuilder *breakBuilder = NULL;
            TR::IlBuilder **breakBuilderRef = NULL;
            if ("Def" != breakstring)
               {
               TypeID breakbuilderID = GetIDFromToken(breakstring);
               breakBuilder = getBuilderFromTypeID(breakbuilderID);
               breakBuilderRef = &breakBuilder;
               }

            leftover = leftover.substr(breakstring.size() + 1);
            std::string continuestring = leftover.substr(0, leftover.find(" "));
            TR::IlBuilder *continueBuilder = NULL;
            TR::IlBuilder **continueBuilderRef = NULL;
            if ("Def" != continuestring)
               {
               TypeID continuebuilderID = GetIDFromToken(continuestring);
               continueBuilder = getBuilderFromTypeID(continuebuilderID);
               continueBuilderRef = &continueBuilder;
               }

            leftover = leftover.substr(continuestring.size() + 1);
            std::string initialstring = leftover.substr(0, leftover.find(" "));

            leftover = leftover.substr(initialstring.size() + 1);
            std::string whilestring = leftover.substr(0, leftover.find(" "));

            leftover = leftover.substr(whilestring.size() + 1);
            std::string incrementstring = leftover.substr(0, leftover.find(" "));

            builder->ForLoop(countsUp, strdup(value.c_str()), &bodybuilder, breakBuilderRef, continueBuilderRef, getValueFromTypeID(GetIDFromToken(initialstring)), getValueFromTypeID(GetIDFromToken(whilestring)), getValueFromTypeID(GetIDFromToken(incrementstring)));
            }
         else if (0 == strncmp("DefineLocal", ref, strlen("DefineLocal")))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string typeString = leftover.substr(0, leftover.find(' '));
            TypeID typeID = GetIDFromToken(typeString);
            TR::IlType *type = getTypeFromTypeID(typeID);

            TR::MethodBuilder *methodBuilder = (TR::MethodBuilder *)builder;

            methodBuilder->DefineLocal(strdup(value.c_str()), type);
            }
         else if (0 == strncmp(STATEMENT_SUB, ref, strlen(STATEMENT_SUB)))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string resultString = leftover.substr(0, leftover.find(' '));

            TypeID resultID = GetIDFromToken(resultString);

            std::string leftover2 = leftover.substr(resultString.size() + 1);
            std::string value1 = leftover2.substr(0, leftover2.find(' '));
            TypeID value1ID = GetIDFromToken(value1);

            std::string value2 = leftover2.substr(value1.size() + 1);
            value2 = value2.substr(0, value2.find(' '));
            TypeID value2ID = GetIDFromToken(value2);

            TR::IlValue * val1 = getValueFromTypeID(value1ID);
            TR::IlValue * val2 = getValueFromTypeID(value2ID);

            TR::IlValue *result = builder->Sub(val1, val2);

            mapTypeIDToPointer(resultID, result);
            }
         else if (0 == strncmp(STATEMENT_CALL, ref, strlen(STATEMENT_CALL)))
            {
             std::string leftover = idToken.substr(idToken.find("]") + 3);
             std::string paramCountString = leftover.substr(0, leftover.find(" "));
             int32_t paramCount = (int32_t)atoi(paramCountString.c_str());

             std::string leftover2 = leftover.substr(paramCountString.size() + 1);
             std::string value1 = leftover2.substr(0, leftover2.find(' '));
             TypeID value1ID = GetIDFromToken(value1);

             std::string value2 = leftover2.substr(value1.size() + 1);
             value2 = value2.substr(0, value2.find(' '));
             TypeID value2ID = GetIDFromToken(value2);

             TR::IlValue * val1 = getValueFromTypeID(value1ID);

             TR::IlValue * retValue = builder->Call(strdup(value.c_str()), paramCount, val1);

             mapTypeIDToPointer(value2ID, retValue);
            }
         else if (0 == strncmp(STATEMENT_CONVERTTO, ref, strlen(STATEMENT_CONVERTTO)))
            {
             std::string leftover = idToken.substr(ident.size() + 1);
             std::string resultString = leftover.substr(0, leftover.find(' '));
             TypeID resultID = GetIDFromToken(resultString);

             std::string leftover2 = leftover.substr(resultString.size() + 1);
             std::string typeString = leftover2.substr(0, leftover2.find(' '));
             TypeID ilTypeID = GetIDFromToken(typeString);

             std::string valueString = leftover2.substr(typeString.size() + 1);
             valueString = valueString.substr(0, valueString.find(' '));
             TypeID valueID = GetIDFromToken(valueString);

             TR::IlType * ilType = getTypeFromTypeID(ilTypeID);
             TR::IlValue * val = getValueFromTypeID(valueID);

             TR::IlValue *result = builder->ConvertTo(ilType, val);

             mapTypeIDToPointer(resultID, result);
            }
         else if (0 == strncmp(STATEMENT_PRIMITIVETYPE, ref, strlen(STATEMENT_PRIMITIVETYPE)))
            {
            std::string leftover = idToken.substr(ident.size() + 1);
            std::string typeString = leftover.substr(0, leftover.find(' '));
            TypeID typeID = GetIDFromToken(typeString);

            std::string valueString = leftover.substr(typeString.size() + 1);
            valueString = valueString.substr(0, valueString.find(' '));

            TR::DataType dt((TR::DataTypes)atoi(valueString.c_str()));
            TR::IlType *type = builder->typeDictionary()->PrimitiveType(dt);

            mapTypeIDToPointer(typeID, type);
            }
         else
            {
            TR_ASSERT_FATAL(0, "Unexpected statement %s", ref);
            }
         }
      }
   }

