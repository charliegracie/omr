/*******************************************************************************
 * Copyright (c) 1991, 2016 IBM Corp. and others
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

#ifndef avl_api_h
#define avl_api_h

/**
* @file avl_api.h
* @brief Public API for the AVL module.
*
* This file contains public function prototypes and
* type definitions for the AVL module.
*
*/

#include "omrport.h"
#include "omravldefines.h"
#include "omravl.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------- avlsup.c ---------------- */

/**
* @brief
* @param *tree
* @param *nodeToDelete
* @return OMRAVLTreeNode *
*/
OMRAVLTreeNode *
avl_delete(OMRAVLTree *tree, OMRAVLTreeNode *nodeToDelete);


/**
* @brief
* @param *tree
* @param *nodeToInsert
* @return OMRAVLTreeNode *
*/
OMRAVLTreeNode *
avl_insert(OMRAVLTree *tree, OMRAVLTreeNode *nodeToInsert);


/**
* @brief
* @param *tree
* @param searchValue
* @return OMRAVLTreeNode *
*/
OMRAVLTreeNode *
avl_search(OMRAVLTree *tree, uintptr_t searchValue);


#ifdef __cplusplus
}
#endif

#endif /* avl_api_h */
