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

#ifndef OMRHASHTABLE_H
#define OMRHASHTABLE_H

/*
 * @ddr_namespace: default
 */

#ifdef __cplusplus
extern "C" {
#endif


/* DO NOT DIRECTLY INCLUDE THIS FILE! */
/* Include hashtable_api.h instead */


#include "omravl.h"
#include "omrcomp.h"
#include "omrport.h"
#include "pool_api.h"

/**
 * Hash table flags
 */
#define OMRHASH_TABLE_DO_NOT_GROW	0x00000001	/*!< Do not grow & rehash the table while set */
#define OMRHASH_TABLE_COLLISION_RESILIENT	0x00000002	/*!< Use hash table using avl trees for collision resolution instead of lists */
#define OMRHASH_TABLE_ALLOCATE_ELEMENTS_USING_MALLOC32	0x00000004	/*!< Allocate table elements using the malloc32 function */
#define OMRHASH_TABLE_ALLOW_SIZE_OPTIMIZATION	0x00000008	/*!< Allow space optimized hashTable, some functions not supported */
#define OMRHASH_TABLE_DO_NOT_REHASH	0x00000010	/*!< Do not rehash the table while set */

#define OMRHASH_TABLE_AVL_TREE_TAG_BIT ((uintptr_t)0x00000001) /*!< Bit to indicate that hastable slot contains a pointer to an AVL tree */

/**
 * Hash Table state constants for iteration
 */
#define OMRHASH_TABLE_ITERATE_STATE_LIST_NODES 0
#define OMRHASH_TABLE_ITERATE_STATE_TREE_NODES 1
#define OMRHASH_TABLE_ITERATE_STATE_FINISHED  2

/**
 * Macros for getting at data directly from AVLTreeNodes
 */
#define AVL_NODE_TO_DATA(p) ((void *)((uint8_t *)(p) + sizeof(OMRAVLTreeNode)))
#define AVL_DATA_TO_NODE(p) (((OMRAVLTreeNode *)((uint8_t *)(p) - sizeof(OMRAVLTreeNode))))

/**
 * Hash table flag macros
 */
#define hashTableCanGrow(table) (((table)->flags & OMRHASH_TABLE_DO_NOT_GROW) ? 0 : 1)
#define hashTableCanRehash(table) (((table)->flags & OMRHASH_TABLE_DO_NOT_REHASH) ? 0 : 1)
#define hashTableSetFlag(table,flag) ((table)->flags |= (flag))
#define hashTableResetFlag(table,flag) ((table)->flags &= ~(flag))

/**
* Hash table state queries
*/
#define hashTableIsSpaceOptimized(table) (NULL == table->listNodePool)


struct OMRHashTable; /* Forward struct declaration */
struct OMRAVLTreeNode; /* Forward struct declaration */
typedef uintptr_t (*OMRHashTableHashFn)(void *entry, void *userData);  /* Forward struct declaration */
typedef uintptr_t (*OMRHashTableEqualFn)(void *leftEntry, void *rightEntry, void *userData);  /* Forward struct declaration */
typedef intptr_t (*OMRHashTableComparatorFn)(struct OMRAVLTree *tree, struct OMRAVLTreeNode *leftNode, struct OMRAVLTreeNode *rightNode);  /* Forward struct declaration */
typedef void (*OMRHashTablePrintFn)(OMRPortLibrary *portLibrary, void *entry, void *userData);  /* Forward struct declaration */
typedef uintptr_t (*OMRHashTableDoFn)(void *entry, void *userData);  /* Forward struct declaration */
typedef struct OMRHashTable {
	const char *tableName;
	uint32_t tableSize;
	uint32_t numberOfNodes;
	uint32_t numberOfTreeNodes;
	uint32_t entrySize;
	uint32_t listNodeSize;
	uint32_t treeNodeSize;
	uint32_t nodeAlignment;
	uint32_t flags;
	uint32_t memoryCategory;
	uint32_t listToTreeThreshold;
	void **nodes;
	struct J9Pool *listNodePool;
	struct J9Pool *treeNodePool;
	struct J9Pool *treePool;
	struct OMRAVLTree *avlTreeTemplate;
	uintptr_t (*hashFn)(void *key, void *userData) ;
	uintptr_t (*hashEqualFn)(void *leftKey, void *rightKey, void *userData) ;
	void (*printFn)(OMRPortLibrary *portLibrary, void *key, void *userData) ;
	struct OMRPortLibrary *portLibrary;
	void *equalFnUserData;
	void *hashFnUserData;
	struct OMRHashTable *previous;
} OMRHashTable;

typedef struct OMRHashTableState {
	struct OMRHashTable *table;
	uint32_t bucketIndex;
	uint32_t didDeleteCurrentNode;
	void **pointerToCurrentNode;
	uintptr_t iterateState;
	struct J9PoolState poolState;
} OMRHashTableState;

//TODO Remove once all downstream project use OMR namespace instead of J9
#define J9HashTable OMRHashTable
#define J9HashTableState OMRHashTableState
#define J9HashTableComparatorFn OMRHashTableComparatorFn
#define J9HashTableDoFn OMRHashTableDoFn
#define J9HashTableEqualFn OMRHashTableEqualFn
#define J9HashTableHashFn OMRHashTableHashFn
#define J9HashTablePrintFn OMRHashTablePrintFn
#define J9HASH_TABLE_DO_NOT_GROW OMRHASH_TABLE_DO_NOT_GROW
#define J9HASH_TABLE_COLLISION_RESILIENT OMRHASH_TABLE_COLLISION_RESILIENT
#define J9HASH_TABLE_ALLOCATE_ELEMENTS_USING_MALLOC32 OMRHASH_TABLE_ALLOCATE_ELEMENTS_USING_MALLOC32
#define J9HASH_TABLE_ALLOW_SIZE_OPTIMIZATION OMRHASH_TABLE_ALLOW_SIZE_OPTIMIZATION
#define J9HASH_TABLE_DO_NOT_REHASH OMRHASH_TABLE_DO_NOT_REHASH
#define J9HASH_TABLE_AVL_TREE_TAG_BIT OMRHASH_TABLE_AVL_TREE_TAG_BIT
#define J9HASH_TABLE_ITERATE_STATE_LIST_NODES OMRHASH_TABLE_ITERATE_STATE_LIST_NODES
#define J9HASH_TABLE_ITERATE_STATE_TREE_NODES OMRHASH_TABLE_ITERATE_STATE_TREE_NODES
#define J9HASH_TABLE_ITERATE_STATE_FINISHED OMRHASH_TABLE_ITERATE_STATE_FINISHED

#ifdef __cplusplus
}
#endif

#endif /* OMRHASHTABLE_H */
