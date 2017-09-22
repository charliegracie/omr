/*******************************************************************************
 * Copyright (c) 2016, 2016 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution and
 * is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following
 * Secondary Licenses when the conditions for such availability set
 * forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
 * General Public License, version 2 with the GNU Classpath
 * Exception [1] and GNU General Public License, version 2 with the
 * OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
 *******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dlfcn.h>
#include <errno.h>

#include "Jit.hpp"
#include "ilgen/TypeDictionary.hpp"
#include "ilgen/MethodBuilder.hpp"
#include "MathPerf.hpp"

MathPerfMethod::MathPerfMethod(TR::TypeDictionary *types)
   : MethodBuilder(types)
   {
   DefineLine(LINETOSTR(__LINE__));
   DefineFile(__FILE__);

   DefineName("math");
   DefineParameter("n", Int64);
   DefineReturnType(Double);
   }

bool
MathPerfMethod::buildIL()
   {
   Store("a",
      ConstDouble(1.0));
   Store("b",
      ConstDouble(2.0));
   Store("c",
      ConstDouble(3.0));
   Store("d",
      ConstDouble(4.0));
   Store("e",
      ConstDouble(5.0));
   Store("f",
      ConstDouble(6.0));
   Store("g",
      ConstDouble(7.0));

   Store("result",
      ConstDouble(0.0));

   Store("i",
      Load("n"));

   Store("keepIterating",
      GreaterThan(
         Load("i"),
         ConstInt64(-1)));

   TR::IlBuilder *loopBody = NULL;
   WhileDoLoop("keepIterating", &loopBody);

   loopBody->Store("result",
   loopBody->   Add(
   loopBody->      Load("result"),
   loopBody->      Load("a")));

   loopBody->Store("result",
   loopBody->   Add(
   loopBody->      Load("result"),
   loopBody->      Load("b")));

   loopBody->Store("result",
   loopBody->   Add(
   loopBody->      Load("result"),
   loopBody->      Load("c")));

   loopBody->Store("result",
   loopBody->   Add(
   loopBody->      Load("result"),
   loopBody->      Load("d")));

   loopBody->Store("result",
   loopBody->   Sub(
   loopBody->      Load("result"),
   loopBody->      Load("c")));

   loopBody->Store("result",
   loopBody->   Add(
   loopBody->      Load("result"),
   loopBody->      Load("e")));

   loopBody->Store("i",
   loopBody->   Sub(
   loopBody->      Load("i"),
   loopBody->      ConstInt64(1)));

   loopBody->Store("keepIterating",
   loopBody->   GreaterThan(
   loopBody->      Load("i"),
   loopBody->      ConstInt64(-1)));

   Return(
      Load("result"));

   return true;
   }


int
main(int argc, char *argv[])
   {
   printf("Step 1: initialize JIT\n");
   bool initialized = initializeJit();
   if (!initialized)
      {
      fprintf(stderr, "FAIL: could not initialize JIT\n");
      exit(-1);
      }

   printf("Step 2: define relevant types\n");
   TR::TypeDictionary types;

   printf("Step 3: compile method builder\n");
   MathPerfMethod MathPerfMethod(&types);
   uint8_t *entry=0;
   int32_t rc = compileMethodBuilder(&MathPerfMethod, &entry);
   if (rc != 0)
      {
      fprintf(stderr,"FAIL: compilation error %d\n", rc);
      exit(-2);
      }

   printf("Step 4: invoke compiled code\n");
   MathPerfFunctionType *mathperf = (MathPerfFunctionType *)entry;
   int32_t n = (argc > 1) ? atoi(argv[1]) : 100;
   double r;
   for (int32_t i=0;i < n;i++)
      r = mathperf((int64_t)200000);

   printf("mathperf(200000) is %f\n", r);

   printf ("Step 5: shutdown JIT\n");
   shutdownJit();

   printf("PASS\n");
   }
