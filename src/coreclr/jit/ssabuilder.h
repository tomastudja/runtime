// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#pragma once
#pragma warning(disable : 4503) // 'identifier' : decorated name length exceeded, name was truncated

#include "compiler.h"
#include "ssarenamestate.h"

typedef int LclVarNum;

// Pair of a local var name eg: V01 and Ssa number; eg: V01_01
typedef std::pair<LclVarNum, int> SsaVarName;

class SsaBuilder
{
private:
    inline void EndPhase(Phases phase)
    {
        m_pCompiler->EndPhase(phase);
    }

    bool IncludeInSsa(unsigned lclNum);

public:
    // Constructor
    SsaBuilder(Compiler* pCompiler);

    // Requires stmt nodes to be already sequenced in evaluation order. Analyzes the graph
    // for introduction of phi-nodes as GT_PHI tree nodes at the beginning of each block.
    // Each GT_LCL_VAR is given its ssa number through its GetSsaNum() field in the node.
    // Each GT_PHI node will be under a GT_ASG node with the LHS set to the local node and
    // the RHS to the GT_PHI itself. The inputs to the PHI are represented as a linked list
    // of GT_PHI_ARG nodes. Each use or def is denoted by the corresponding GT_LCL_VAR
    // tree. For example, to get all uses of a particular variable fully defined by its
    // lclNum and ssaNum, one would use m_uses and look up all the uses. Similarly, a single
    // def of an SSA variable can be looked up similarly using m_defs member.
    void Build();

private:
    // Ensures that the basic block graph has a root for the dominator graph, by ensuring
    // that there is a first block that is not in a try region (adding an empty block for that purpose
    // if necessary).  Eventually should move to Compiler.
    void SetupBBRoot();

    // Requires "postOrder" to be an array of size "count". Requires "count" to at least
    // be the size of the flow graph. Sorts the current compiler's flow-graph and places
    // the blocks in post order (i.e., a node's children first) in the array. Returns the
    // number of nodes visited while sorting the graph. In other words, valid entries in
    // the output array.
    int TopologicalSort(BasicBlock** postOrder, int count);

    // Requires "postOrder" to hold the blocks of the flowgraph in topologically sorted
    // order. Requires count to be the valid entries in the "postOrder" array. Computes
    // each block's immediate dominator and records it in the BasicBlock in bbIDom.
    void ComputeImmediateDom(BasicBlock** postOrder, int count);

    // Compute flow graph dominance frontiers.
    void ComputeDominanceFrontiers(BasicBlock** postOrder, int count, BlkToBlkVectorMap* mapDF);

    // Compute the iterated dominance frontier for the specified block.
    void ComputeIteratedDominanceFrontier(BasicBlock* b, const BlkToBlkVectorMap* mapDF, BlkVector* bIDF);

    // Insert a new GT_PHI statement.
    void InsertPhi(BasicBlock* block, unsigned lclNum);

    // Add a new GT_PHI_ARG node to an existing GT_PHI node
    void AddPhiArg(
        BasicBlock* block, Statement* stmt, GenTreePhi* phi, unsigned lclNum, unsigned ssaNum, BasicBlock* pred);

    // Requires "postOrder" to hold the blocks of the flowgraph in topologically sorted order. Requires
    // count to be the valid entries in the "postOrder" array. Inserts GT_PHI nodes at the beginning
    // of basic blocks that require them like so:
    // GT_ASG(GT_LCL_VAR, GT_PHI(GT_PHI_ARG(ssaNum, Block*), GT_PHI_ARG(ssaNum, Block*), ...));
    void InsertPhiFunctions(BasicBlock** postOrder, int count);

    // Rename all definitions and uses within the compiled method.
    void RenameVariables();
    // Rename all definitions and uses within a block.
    void BlockRenameVariables(BasicBlock* block);
    // Rename a local or memory definition generated by a GT_ASG node.
    void RenameDef(GenTreeOp* asgNode, BasicBlock* block);
    // Rename a use of a local variable.
    void RenameLclUse(GenTreeLclVarCommon* lclNode);

    // Assumes that "block" contains a definition for local var "lclNum", with SSA number "ssaNum".
    // IF "block" is within one or more try blocks,
    // and the local variable is live at the start of the corresponding handlers,
    // add this SSA number "ssaNum" to the argument list of the phi for the variable in the start
    // block of those handlers.
    void AddDefToHandlerPhis(BasicBlock* block, unsigned lclNum, unsigned ssaNum);

    // Same as above, for memory.
    void AddMemoryDefToHandlerPhis(MemoryKind memoryKind, BasicBlock* block, unsigned ssaNum);

    // Add GT_PHI_ARG nodes to the GT_PHI nodes within block's successors.
    void AddPhiArgsToSuccessors(BasicBlock* block);

#ifdef DEBUG
    void Print(BasicBlock** postOrder, int count);
#endif

private:
    Compiler*     m_pCompiler;
    CompAllocator m_allocator;

    // Bit vector used by TopologicalSort and ComputeImmediateDom to track already visited blocks.
    BitVecTraits m_visitedTraits;
    BitVec       m_visited;

    SsaRenameState m_renameStack;
};
