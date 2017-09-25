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

#ifndef j9nongenerated_h
#define j9nongenerated_h

#include "omrcomp.h"
#include "omrport.h"
#include "omrsrp.h"

/* NOTE: Use j9generated.h unless you receive different guidance */

typedef struct OMRAVLTreeNode {
	J9WSRP leftChild;
	J9WSRP rightChild;
} OMRAVLTreeNode;

typedef struct OMRAVLTree {
	intptr_t (*insertionComparator)(struct OMRAVLTree *tree, struct OMRAVLTreeNode *insertNode, struct OMRAVLTreeNode *walkNode) ;
	intptr_t (*searchComparator)(struct OMRAVLTree *tree, uintptr_t searchValue, struct OMRAVLTreeNode *node) ;
	void (*genericActionHook)(struct OMRAVLTree *tree, struct OMRAVLTreeNode *node, uintptr_t action) ; /* TODO Remove this as soon as all downstream projects stop setting it */
	uintptr_t (*performNodeAction)(struct OMRAVLTree *tree, struct OMRAVLTreeNode *node, uintptr_t action, void *userData) ;
	uintptr_t flags;
	struct OMRAVLTreeNode *rootNode;
	struct OMRPortLibrary *portLibrary;
	void *userData;
} OMRAVLTree;

#define OMRAVLTREENODE_LEFTCHILD(base) AVL_SRP_GETNODE((base)->leftChild)
#define OMRAVLTREENODE_RIGHTCHILD(base) AVL_SRP_GETNODE((base)->rightChild)

/* TODO Remove AVL typedefs once downstream project use OMR namespace */
#define J9AVLTreeNode OMRAVLTreeNode
#define J9AVLTree OMRAVLTree
#define J9AVLTREENODE_LEFTCHILD OMRAVLTREENODE_LEFTCHILD
#define J9AVLTREENODE_RIGHTCHILD OMRAVLTREENODE_RIGHTCHILD
#define J9AVLTREE_IS_SHARED_TREE OMRAVLTREE_IS_SHARED_TREE
#define J9AVLTREE_SHARED_TREE_INITIALIZED OMRAVLTREE_SHARED_TREE_INITIALIZED
#define J9AVLTREE_DISABLE_SHARED_TREE_UPDATES OMRAVLTREE_DISABLE_SHARED_TREE_UPDATES
#define J9AVLTREE_TEST_INTERNAVL OMRAVLTREE_TEST_INTERNAVL
#define J9AVLTREE_DO_VERIFY_TREE_STRUCT_AND_ACCESS OMRAVLTREE_DO_VERIFY_TREE_STRUCT_AND_ACCESS

/*
 * @ddr_namespace: map_to_type=J9JITHashTable
 */

/* NOTE: JIT HashTable (and walk state) likely need to be pushed into the JIT side of jvm/jit compilation */
typedef struct J9JITHashTable {
	OMRAVLTreeNode parentAVLTreeNode;
	uintptr_t *buckets;
	uintptr_t start;
	uintptr_t end;
	uintptr_t flags;
	uintptr_t *methodStoreStart;
	uintptr_t *methodStoreEnd;
	uintptr_t *currentAllocate;
} J9JITHashTable;

#define JIT_HASH_IN_DATA_CACHE  1

#define J9JITHASHTABLE_LEFTCHILD(base) AVL_SRP_GETNODE((base)->parentAVLTreeNode.leftChild)
#define J9JITHASHTABLE_RIGHTCHILD(base) AVL_SRP_GETNODE((base)->parentAVLTreeNode.rightChild)

typedef struct J9JITHashTableWalkState {
	struct J9JITHashTable *table;
	uintptr_t index;
	uintptr_t *bucket;
} J9JITHashTableWalkState;

/* NOTE: J9MEMAVLTreeNode should be pushed into a memcheck specific definition */
typedef struct J9MEMAVLTreeNode {
	OMRAVLTreeNode parentAVLTreeNode;
	const char *callSite;
	struct J9MemoryCheckStats *stats;
	struct J9MemoryCheckStats *prevStats;
} J9MEMAVLTreeNode;

#define J9MEMAVLTREENODE_LEFTCHILD(base) AVL_SRP_GETNODE((base)->parentAVLTreeNode.leftChild)
#define J9MEMAVLTREENODE_RIGHTCHILD(base) AVL_SRP_GETNODE((base)->parentAVLTreeNode.rightChild)

typedef struct J9CmdLineMapping {
	char *j9Name;
	char *mapName;
	uintptr_t flags;
} J9CmdLineMapping;

typedef struct J9CmdLineOption {
	struct J9CmdLineMapping *mapping;
	uintptr_t flags;
	char *fromEnvVar;
} J9CmdLineOption;

/*TODO Temporary duplicate of J9VMGCSublistFragment from j9generated.h */

/*
 * @ddr_namespace: map_to_type=J9VMGC_SublistFragment
 */

typedef struct J9VMGC_SublistFragment {
	uintptr_t *fragmentCurrent;
	uintptr_t *fragmentTop;
	uintptr_t fragmentSize;
	void *parentList;
	uintptr_t deferredFlushID;
	uintptr_t count;
} J9VMGC_SublistFragment;

#define J9SIZEOF_J9VMGC_SublistFragment 48


#define J9MMCONSTANT_IMPLICIT_GC_DEFAULT  0
#define J9MMCONSTANT_EXPLICIT_GC_NOT_AGGRESSIVE  1
#define J9MMCONSTANT_EXPLICIT_GC_RASDUMP_COMPACT  2
#define J9MMCONSTANT_EXPLICIT_GC_EXCLUSIVE_VMACCESS_ALREADY_ACQUIRED  2
#define J9MMCONSTANT_EXPLICIT_GC_SYSTEM_GC  3
#define J9MMCONSTANT_EXPLICIT_GC_NATIVE_OUT_OF_MEMORY  4
#define J9MMCONSTANT_IMPLICIT_GC_AGGRESSIVE  5
#define J9MMCONSTANT_IMPLICIT_GC_PERCOLATE  6
#define J9MMCONSTANT_IMPLICIT_GC_PERCOLATE_AGGRESSIVE  7
#define J9MMCONSTANT_IMPLICIT_GC_EXCESSIVE  8
#define J9MMCONSTANT_IMPLICIT_GC_PERCOLATE_UNLOADING_CLASSES  9
#define J9MMCONSTANT_IMPLICIT_GC_PERCOLATE_CRITICAL_REGIONS  10
#if defined(OMR_GC_CONCURRENT_SCAVENGER)
#define J9MMCONSTANT_IMPLICIT_GC_PERCOLATE_ABORTED_SCAVENGE 11
#endif
#if defined(OMR_GC_IDLE_HEAP_MANAGER)
#define J9MMCONSTANT_EXPLICIT_GC_IDLE_GC 12
#endif

typedef struct J9MemorySpaceDescription {
	uintptr_t oldSpaceSize;
	uintptr_t newSpaceSize;
} J9MemorySpaceDescription;

/*
 * @ddr_namespace: map_to_type=J9MemorySegment
 */

typedef struct J9MemorySegment {
	OMRAVLTreeNode parentAVLTreeNode;
	uintptr_t type;
	uintptr_t size;
	uint8_t *baseAddress;
	uint8_t *heapBase;
	uint8_t *heapTop;
	uint8_t *heapAlloc;
	struct J9MemorySegment *nextSegment;
	struct J9MemorySegment *previousSegment;
	struct J9MemorySegmentList *memorySegmentList;
	uintptr_t unused1;
	struct J9ClassLoader *classLoader;
	void *memorySpace;
	struct J9MemorySegment *nextSegmentInClassLoader;
	struct J9PortVmemIdentifier vmemIdentifier;
} J9MemorySegment;

#define MEMORY_TYPE_OLD  1
#define MEMORY_TYPE_NEW_RAM  10
#define MEMORY_TYPE_SCOPED  0x2000
#define MEMORY_TYPE_ALLOCATED  64
#define MEMORY_TYPE_IMMORTAL  0x1000
#define MEMORY_TYPE_DEBUG_INFO  0x200
#define MEMORY_TYPE_BASETYPE_ROM_CLASS  0x200000
#define MEMORY_TYPE_DYNAMIC_LOADED_CLASSES  0x20040
#define MEMORY_TYPE_NEW  2
#define MEMORY_TYPE_DISCARDABLE  0x80
#define MEMORY_TYPE_NUMA  0x4000
#define MEMORY_TYPE_ROM_CLASS  0x20000
#define MEMORY_TYPE_UNCOMMITTED  0x800
#define MEMORY_TYPE_FROM_JXE  0x4000000
#define MEMORY_TYPE_OLD_ROM  5
#define MEMORY_TYPE_SHARED_META  0x8000000
/* MEMORY_TYPE_VIRTUAL is expected to be used along with other types like MEMORY_TYPE_JIT_SCRATCH_SPACE
 * or MEMORY_TYPE_JIT_PERSISTENT to allocate virtual memory instead of malloc'ed memory.
 */
#define MEMORY_TYPE_VIRTUAL  0x400
/* MEMORY_TYPE_FIXED_RAM_CLASS is virtually allocated, setting MEMORY_TYPE_VIRTUAL is not required. */
#define MEMORY_TYPE_FIXED_RAM_CLASS  0x8000
#define MEMORY_TYPE_RAM_CLASS  0x10000
#define MEMORY_TYPE_IGC_SCAN_QUEUE  0x400000
#define MEMORY_TYPE_RAM  8
#define MEMORY_TYPE_FIXED  16
#define MEMORY_TYPE_JIT_SCRATCH_SPACE  0x1000000
#define MEMORY_TYPE_FIXED_RAM  24
#define MEMORY_TYPE_OLD_RAM  9
/* MEMORY_TYPE_CODE is used for virtually allocated JIT code segments, setting MEMORY_TYPE_VIRTUAL is not required. */
#define MEMORY_TYPE_CODE  32
#define MEMORY_TYPE_ROM  4
#define MEMORY_TYPE_CLASS_FILE_BYTES  0x40000
#define MEMORY_TYPE_UNDEAD_CLASS  0x80000
#define MEMORY_TYPE_JIT_PERSISTENT  0x800000
#define MEMORY_TYPE_FIXEDSIZE  0x100
#define MEMORY_TYPE_DEFAULT  0x2000000

#define J9MEMORYSEGMENT_LEFTCHILD(base) AVL_SRP_GETNODE((base)->parentAVLTreeNode.leftChild)
#define J9MEMORYSEGMENT_RIGHTCHILD(base) AVL_SRP_GETNODE((base)->parentAVLTreeNode.rightChild)

typedef struct J9MemorySegmentList {
	struct J9Pool *segmentPool;
	struct J9MemorySegment *nextSegment;
	uintptr_t totalSegmentSize;
	omrthread_monitor_t segmentMutex;
	struct OMRAVLTree avlTreeData;
	uintptr_t flags;
} J9MemorySegmentList;

#endif /* j9nongenerated_h */
