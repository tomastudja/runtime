// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#include "jitpch.h"

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "fgprofilesynthesis.h"

// TODO
//
// * faster way of doing fgGetPredForBlock
// * vet against some real data
// * IR based heuristics (perhaps)
// * PGO blend modes, random, etc
// * Repair mode
// * Inlinee flow graphs
// * During Cp, avoid repeatedly propagating through nested loops
// * Fake BB0 or always force scratch BB
// * Reconcile with our other loop finding stuff
// * Stop the upweight/downweight of loops in rest of jit
// * Durable edge properties (exit, back)
// * Proper weight comp for finallies
// * Tweak RunRarely to be at or near zero
// * OSR entry weight
// * Special handling for deep nests?
// * Plan for irreducible cases -- MoveNext's

//------------------------------------------------------------------------
// fgProfileSynthesis: update edge likelihoods and block counts based
//   on various strategies
//
// Arguments:
//   options - options to control synthesis
//
void ProfileSynthesis::Run(ProfileSynthesisOption option)
{
    // Just root instances for now.
    // Need to sort out issues with flow analysis for inlinees.
    //
    if (m_comp->compIsForInlining())
    {
        return;
    }

    BuildReversePostorder();
    FindLoops();

    // Retain or compute edge likelihood information
    //
    switch (option)
    {
        case ProfileSynthesisOption::AssignLikelihoods:
            AssignLikelihoods();
            break;

        case ProfileSynthesisOption::RetainLikelihoods:
            break;

        case ProfileSynthesisOption::BlendLikelihoods:
            BlendLikelihoods();
            break;

        case ProfileSynthesisOption::ResetAndSynthesize:
            ClearLikelihoods();
            AssignLikelihoods();
            break;

        case ProfileSynthesisOption::ReverseLikelihoods:
            ReverseLikelihoods();
            break;

        case ProfileSynthesisOption::RandomLikelihoods:
            RandomizeLikelihoods();
            break;

        default:
            assert(!"unexpected profile synthesis option");
            break;
    }

    // Determine cyclic probabilities
    //
    ComputeCyclicProbabilities();

    // Assign weights to entry points in the flow graph
    //
    AssignInputWeights();

    // Compute the block weights given the inputs and edge likelihoods
    //
    ComputeBlockWeights();

    // For now, since we have installed synthesized profile data,
    // act as if we don't have "real" profile data.
    //
    m_comp->fgPgoHaveWeights = false;

#ifdef DEBUG
    if (JitConfig.JitCheckSynthesizedCounts() > 0)
    {
        // Verify consistency, provided we didn't see any improper headers
        // or cap any Cp values.
        //
        if ((m_improperLoopHeaders == 0) && (m_cappedCyclicProbabilities == 0))
        {
            // verify likely weights, assert on failure, check all blocks
            m_comp->fgDebugCheckProfileWeights(ProfileChecks::CHECK_LIKELY | ProfileChecks::RAISE_ASSERT |
                                               ProfileChecks::CHECK_ALL_BLOCKS);
        }
    }
#endif
}

//------------------------------------------------------------------------
// AssignLikelihoods: update edge likelihoods and block counts based
//   entirely on heuristics.
//
// Notes:
//   any existing likelihoods are removed in favor of heuristic
//   likelihoods
//
void ProfileSynthesis::AssignLikelihoods()
{
    JITDUMP("Assigning edge likelihoods based on heuristics\n");

    for (BasicBlock* const block : m_comp->Blocks())
    {
        switch (block->bbJumpKind)
        {
            case BBJ_THROW:
            case BBJ_RETURN:
            case BBJ_EHFINALLYRET:
                // No successor cases
                // (todo: finally ret may have succs)
                break;

            case BBJ_NONE:
            case BBJ_CALLFINALLY:
                // Single successor next cases
                //
                // Note we handle flow to the finally
                // specially; this represents return
                // from the finally.
                AssignLikelihoodNext(block);
                break;

            case BBJ_ALWAYS:
            case BBJ_LEAVE:
            case BBJ_EHCATCHRET:
            case BBJ_EHFILTERRET:
                // Single successor jump cases
                AssignLikelihoodJump(block);
                break;

            case BBJ_COND:
                // Two successor cases
                AssignLikelihoodCond(block);
                break;

            case BBJ_SWITCH:
                // N successor cases
                AssignLikelihoodSwitch(block);
                break;

            default:
                unreached();
        }
    }
}

//------------------------------------------------------------------------
// IsDfsAncestor: see if block `x` is ancestor of block `y` in the depth
//   first spanning tree
//
// Arguments:
//   x -- block that is possible ancestor
//   y -- block that is possible descendant
//
// Returns:
//   True if x is ancestor of y in the depth first spanning tree.
//
// Notes:
//   If return value is false, then x does not dominate y.
//
bool ProfileSynthesis::IsDfsAncestor(BasicBlock* x, BasicBlock* y)
{
    return ((x->bbPreorderNum <= y->bbPreorderNum) && (y->bbPostorderNum <= x->bbPostorderNum));
}

//------------------------------------------------------------------------
// GetLoopFromHeader: see if a block is a loop header, and if so return
//   the associated loop.
//
// Arguments:
//    block - block in question
//
// Returns:
//    loop headed by block, or nullptr
//
SimpleLoop* ProfileSynthesis::GetLoopFromHeader(BasicBlock* block)
{
    for (SimpleLoop* loop : *m_loops)
    {
        if (loop->m_head == block)
        {
            return loop;
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------
// IsLoopBackEdge: see if an edge is a loop back edge
//
// Arguments:
//   edge - edge in question
//
// Returns:
//   True if edge is a backedge in some recognized loop.
//
// Notes:
//   Different than asking IsDfsAncestor since we disqualify some
//   natural backedges for complex loop strctures.
//
// Todo:
//   Annotate the edge directly
//
bool ProfileSynthesis::IsLoopBackEdge(FlowEdge* edge)
{
    for (SimpleLoop* loop : *m_loops)
    {
        for (FlowEdge* loopBackEdge : loop->m_backEdges)
        {
            if (loopBackEdge == edge)
            {
                return true;
            }
        }
    }

    return false;
}

//------------------------------------------------------------------------
// IsLoopExitEdge: see if a flow edge is a loop exit edge
//
// Arguments:
//   edge - edge in question
//
// Returns:
//   True if edge is an exit edge in some recognized loop
//
// Todo:
//   Annotate the edge directly
//
//   Decide if we want to report that the edge exits
//   multiple loops.

bool ProfileSynthesis::IsLoopExitEdge(FlowEdge* edge)
{
    for (SimpleLoop* loop : *m_loops)
    {
        for (FlowEdge* loopExitEdge : loop->m_exitEdges)
        {
            if (loopExitEdge == edge)
            {
                return true;
            }
        }
    }

    return false;
}

//------------------------------------------------------------------------
// AssignLikelihoodNext: update edge likelihood for block that always
//   transfers control to bbNext
//
// Arguments;
//   block -- block in question
//
void ProfileSynthesis::AssignLikelihoodNext(BasicBlock* block)
{
    FlowEdge* const edge = m_comp->fgGetPredForBlock(block->bbNext, block);
    edge->setLikelihood(1.0);
}

//------------------------------------------------------------------------
// AssignLikelihoodJump: update edge likelihood for a block that always
//   transfers control to bbJumpDest
//
// Arguments;
//   block -- block in question
//
void ProfileSynthesis::AssignLikelihoodJump(BasicBlock* block)
{
    FlowEdge* const edge = m_comp->fgGetPredForBlock(block->bbJumpDest, block);
    edge->setLikelihood(1.0);
}

//------------------------------------------------------------------------
// AssignLikelihoodCond: update edge likelihood for a block that
//   ends in a conditional branch
//
// Arguments;
//   block -- block in question (BBJ_COND)
//
void ProfileSynthesis::AssignLikelihoodCond(BasicBlock* block)
{
    BasicBlock* const jump = block->bbJumpDest;
    BasicBlock* const next = block->bbNext;

    // Watch for degenerate case
    //
    if (jump == next)
    {
        AssignLikelihoodNext(block);
        return;
    }

    FlowEdge* const jumpEdge = m_comp->fgGetPredForBlock(jump, block);
    FlowEdge* const nextEdge = m_comp->fgGetPredForBlock(next, block);

    // THROW heuristic
    //
    bool const isJumpThrow = (jump->bbJumpKind == BBJ_THROW);
    bool const isNextThrow = (next->bbJumpKind == BBJ_THROW);

    if (isJumpThrow != isNextThrow)
    {
        if (isJumpThrow)
        {
            jumpEdge->setLikelihood(0.0);
            nextEdge->setLikelihood(1.0);
        }
        else
        {
            jumpEdge->setLikelihood(1.0);
            nextEdge->setLikelihood(0.0);
        }

        return;
    }

    // LOOP BACK EDGE heuristic
    //
    bool const isJumpEdgeBackEdge = IsLoopBackEdge(jumpEdge);
    bool const isNextEdgeBackEdge = IsLoopBackEdge(nextEdge);

    if (isJumpEdgeBackEdge != isNextEdgeBackEdge)
    {
        if (isJumpEdgeBackEdge)
        {
            JITDUMP(FMT_BB "->" FMT_BB " is loop back edge\n", block->bbNum, jump->bbNum);
            jumpEdge->setLikelihood(0.9);
            nextEdge->setLikelihood(0.1);
        }
        else
        {
            JITDUMP(FMT_BB "->" FMT_BB " is loop back edge\n", block->bbNum, next->bbNum);
            jumpEdge->setLikelihood(0.1);
            nextEdge->setLikelihood(0.9);
        }

        return;
    }

    // LOOP EXIT EDGE heuristic
    //
    // Consider: adjust probability if loop has multiple exit edges, so that
    // overall exit probability is around 0.1.
    //
    bool const isJumpEdgeExitEdge = IsLoopExitEdge(jumpEdge);
    bool const isNextEdgeExitEdge = IsLoopExitEdge(nextEdge);

    if (isJumpEdgeExitEdge != isNextEdgeExitEdge)
    {
        if (isJumpEdgeExitEdge)
        {
            JITDUMP(FMT_BB "->" FMT_BB " is loop exit edge\n", block->bbNum, jump->bbNum);
            jumpEdge->setLikelihood(0.1);
            nextEdge->setLikelihood(0.9);
        }
        else
        {
            JITDUMP(FMT_BB "->" FMT_BB " is loop exit edge\n", block->bbNum, next->bbNum);
            jumpEdge->setLikelihood(0.9);
            nextEdge->setLikelihood(0.1);
        }

        return;
    }

    // RETURN heuristic
    //
    bool const isJumpReturn = (jump->bbJumpKind == BBJ_RETURN);
    bool const isNextReturn = (next->bbJumpKind == BBJ_RETURN);

    if (isJumpReturn != isNextReturn)
    {
        if (isJumpReturn)
        {
            jumpEdge->setLikelihood(0.2);
            nextEdge->setLikelihood(0.8);
        }
        else
        {
            jumpEdge->setLikelihood(0.8);
            nextEdge->setLikelihood(0.2);
        }

        return;
    }

    // IL OFFSET heuristic
    //
    // Give slight preference to bbNext
    //
    jumpEdge->setLikelihood(0.48);
    nextEdge->setLikelihood(0.52);
}

//------------------------------------------------------------------------
// AssignLikelihoodSwitch: update edge likelihood for a block that
//   ends in a switch
//
// Arguments;
//   block -- block in question (BBJ_SWITCH)
//
void ProfileSynthesis::AssignLikelihoodSwitch(BasicBlock* block)
{
    // Assume each switch case is equally probable
    //
    const unsigned n = block->NumSucc();
    assert(n != 0);
    const weight_t p = 1 / (weight_t)n;

    // Each unique edge gets some multiple of that basic probability
    //
    for (BasicBlock* const succ : block->Succs(m_comp))
    {
        FlowEdge* const edge = m_comp->fgGetPredForBlock(succ, block);
        edge->setLikelihood(p * edge->getDupCount());
    }
}

//------------------------------------------------------------------------
// fgBlendLikelihoods: blend existing and heuristic likelihoods for all blocks
//
void ProfileSynthesis::BlendLikelihoods()
{
}

void ProfileSynthesis::ClearLikelihoods()
{
}

void ProfileSynthesis::ReverseLikelihoods()
{
}

void ProfileSynthesis::RandomizeLikelihoods()
{
}

//------------------------------------------------------------------------
// fgBuildReversePostorder: compute depth first spanning tree and pre
//   and post numbers for the blocks
//
void ProfileSynthesis::BuildReversePostorder()
{
    m_comp->EnsureBasicBlockEpoch();
    m_comp->fgBBReversePostorder = new (m_comp, CMK_Pgo) BasicBlock*[m_comp->fgBBNumMax + 1]{};
    m_comp->fgComputeEnterBlocksSet();
    m_comp->fgDfsReversePostorder();

    // Build map from bbNum to Block*.
    //
    m_bbNumToBlockMap = new (m_comp, CMK_Pgo) BasicBlock*[m_comp->fgBBNumMax + 1]{};
    for (BasicBlock* const block : m_comp->Blocks())
    {
        m_bbNumToBlockMap[block->bbNum] = block;
    }

#ifdef DEBUG
    if (m_comp->verbose)
    {
        printf("\nAfter doing a post order traversal of the BB graph, this is the ordering:\n");
        for (unsigned i = 1; i <= m_comp->fgBBNumMax; ++i)
        {
            BasicBlock* const block = m_comp->fgBBReversePostorder[i];
            printf("%02u -> " FMT_BB "[%u, %u]\n", i, block->bbNum, block->bbPreorderNum, block->bbPostorderNum);
        }
        printf("\n");
    }
#endif // DEBUG
}

//------------------------------------------------------------------------
// FindLoops: locate and classify loops
//
void ProfileSynthesis::FindLoops()
{
    CompAllocator allocator = m_comp->getAllocator(CMK_Pgo);
    m_loops                 = new (allocator) LoopVector(allocator);

    // Identify loops
    //
    for (unsigned i = 1; i <= m_comp->fgBBNumMax; i++)
    {
        BasicBlock* const block = m_comp->fgBBReversePostorder[i];

        // If a block is a DFS ancestor of one if its predecessors then the block is a loop header.
        //
        SimpleLoop* loop = nullptr;

        for (FlowEdge* predEdge : block->PredEdges())
        {
            if (IsDfsAncestor(block, predEdge->getSourceBlock()))
            {
                if (loop == nullptr)
                {
                    loop = new (allocator) SimpleLoop(block, allocator);
                    JITDUMP("\n");
                }

                JITDUMP(FMT_BB " -> " FMT_BB " is a backedge\n", predEdge->getSourceBlock()->bbNum, block->bbNum);
                loop->m_backEdges.push_back(predEdge);
            }
        }

        if (loop == nullptr)
        {
            continue;
        }

        JITDUMP(FMT_BB " is head of a DFS loop with %d back edges\n", block->bbNum, loop->m_backEdges.size());

        // Now walk back in flow along the back edges from block to determine if
        // this is a natural loop and to find all the blocks in the loop.
        //
        loop->m_blocks = BlockSetOps::MakeEmpty(m_comp);
        BlockSetOps::AddElemD(m_comp, loop->m_blocks, block->bbNum);

        // todo: hoist this out and just do a reset here
        jitstd::list<BasicBlock*> worklist(allocator);

        // Seed the worklist
        //
        for (FlowEdge* backEdge : loop->m_backEdges)
        {
            BasicBlock* const backEdgeSource = backEdge->getSourceBlock();

            if (BlockSetOps::IsMember(m_comp, loop->m_blocks, backEdgeSource->bbNum))
            {
                continue;
            }

            worklist.push_back(backEdgeSource);
        }

        bool isNaturalLoop = true;

        // Work back through flow to loop head or to another pred
        // that is clearly outside the loop.
        //
        // TODO: verify that we can indeed get back to the loop head
        // and not get stopped somewhere (eg loop through EH).
        //
        while (!worklist.empty() & isNaturalLoop)
        {
            BasicBlock* const loopBlock = worklist.back();
            worklist.pop_back();
            BlockSetOps::AddElemD(m_comp, loop->m_blocks, loopBlock->bbNum);

            for (FlowEdge* const predEdge : loopBlock->PredEdges())
            {
                BasicBlock* const predBlock = predEdge->getSourceBlock();

                // `block` cannot dominate `predBlock` unless it is a DFS ancestor.
                //
                if (!IsDfsAncestor(block, predBlock))
                {
                    // Does this represent flow out of some handler?
                    // If so we will ignore it.
                    //
                    // Might want to vet that handler's try region entry
                    // is a dfs ancestor...?
                    //
                    if (!BasicBlock::sameHndRegion(block, predBlock))
                    {
                        continue;
                    }

                    JITDUMP("Loop is not natural; witness " FMT_BB " -> " FMT_BB "\n", predBlock->bbNum,
                            loopBlock->bbNum);

                    isNaturalLoop = false;
                    m_improperLoopHeaders++;
                    break;
                }

                if (BlockSetOps::IsMember(m_comp, loop->m_blocks, predBlock->bbNum))
                {
                    continue;
                }

                worklist.push_back(predBlock);
            }
        }

        if (!isNaturalLoop)
        {
            continue;
        }

        JITDUMP("Loop has %d blocks\n", BlockSetOps::Count(m_comp, loop->m_blocks));

        // Find the exit edges
        //
        BlockSetOps::Iter iter(m_comp, loop->m_blocks);
        unsigned          bbNum = 0;
        while (iter.NextElem(&bbNum))
        {
            BasicBlock* const loopBlock = m_bbNumToBlockMap[bbNum];

            for (BasicBlock* const succBlock : loopBlock->Succs(m_comp))
            {
                if (!BlockSetOps::IsMember(m_comp, loop->m_blocks, succBlock->bbNum))
                {
                    FlowEdge* const exitEdge = m_comp->fgGetPredForBlock(succBlock, loopBlock);
                    JITDUMP(FMT_BB " -> " FMT_BB " is an exit edge\n", loopBlock->bbNum, succBlock->bbNum);
                    loop->m_exitEdges.push_back(exitEdge);
                }
            }
        }

        // Find the entry edges
        //
        // Note if fgEntryBB is a loop head we won't have an entry edge.
        // So it needs to be special cased later on when processing
        // entry edges.
        //
        for (FlowEdge* const predEdge : loop->m_head->PredEdges())
        {
            if (!IsDfsAncestor(block, predEdge->getSourceBlock()))
            {
                JITDUMP(FMT_BB " -> " FMT_BB " is an entry edge\n", predEdge->getSourceBlock()->bbNum,
                        loop->m_head->bbNum);
                loop->m_entryEdges.push_back(predEdge);
            }
        }

        // Search for parent loop, validate proper nesting.
        //
        // Since loops record in outer->inner order the parent will be the
        // most recently recorded loop that contains this loop's header.
        //
        for (auto it = m_loops->rbegin(), itEnd = m_loops->rend(); it != itEnd; ++it)
        {
            SimpleLoop* const otherLoop = *it;

            if (BlockSetOps::IsMember(m_comp, otherLoop->m_blocks, block->bbNum))
            {
                // Ancestor loop; should contain all blocks of this loop
                //
                assert(BlockSetOps::IsSubset(m_comp, loop->m_blocks, otherLoop->m_blocks));

                if (loop->m_parent == nullptr)
                {
                    loop->m_parent = otherLoop;
                    loop->m_depth  = otherLoop->m_depth + 1;
                    JITDUMP("at depth %u, nested within loop starting at " FMT_BB "\n", loop->m_depth,
                            otherLoop->m_head->bbNum);

                    // Note we could break here but that would bypass the non-overlap check
                    // just below, so for now we check against all known loops.
                }
            }
            else
            {
                // Non-ancestor loop; should have no blocks in common with current loop
                //
                assert(BlockSetOps::IsEmptyIntersection(m_comp, loop->m_blocks, otherLoop->m_blocks));
            }
        }

        if (loop->m_parent == nullptr)
        {
            JITDUMP("top-level loop\n");
            loop->m_depth = 1;
        }

        // Record this loop
        //
        m_loops->push_back(loop);
    }

    if (m_loops->size() > 0)
    {
        JITDUMP("\nFound %d loops\n", m_loops->size());
    }

    if (m_improperLoopHeaders > 0)
    {
        JITDUMP("Rejected %d loop headers\n", m_improperLoopHeaders);
    }
}

//------------------------------------------------------------------------
// FindCyclicProbabilities: for each loop, compute how much flow returns
//   to the loop head given one external count.
//
void ProfileSynthesis::ComputeCyclicProbabilities()
{
    // We found loop walking in reverse postorder, so the loop vector
    // is naturally organized with outer loops before inner.
    //
    // Walk it backwards here so we compute inner loop cyclic probabilities
    // first. We rely on that when processing outer loops.
    //
    for (auto it = m_loops->rbegin(), itEnd = m_loops->rend(); it != itEnd; ++it)
    {
        SimpleLoop* const loop = *it;
        ComputeCyclicProbabilities(loop);
    }
}

//------------------------------------------------------------------------
// FindCyclicProbabilities: for a given loop, compute how much flow returns
//   to the loop head given one external count.
//
void ProfileSynthesis::ComputeCyclicProbabilities(SimpleLoop* loop)
{
    // Initialize
    //
    BlockSetOps::Iter iter(m_comp, loop->m_blocks);
    unsigned          bbNum = 0;
    while (iter.NextElem(&bbNum))
    {
        BasicBlock* const loopBlock = m_bbNumToBlockMap[bbNum];
        loopBlock->bbWeight         = 0.0;
    }

    // Process loop blocks in RPO. Just takes one pass through the loop blocks
    // as any cyclic contributions are handled by cyclic probabilities.
    //
    for (unsigned int i = 1; i <= m_comp->fgBBNumMax; i++)
    {
        BasicBlock* const block = m_comp->fgBBReversePostorder[i];

        if (!BlockSetOps::IsMember(m_comp, loop->m_blocks, block->bbNum))
        {
            continue;
        }

        // Loop head gets external count of 1
        //
        if (block == loop->m_head)
        {
            JITDUMP("ccp: " FMT_BB " :: 1.0\n", block->bbNum);
            block->bbWeight = 1.0;
        }
        else
        {
            SimpleLoop* const nestedLoop = GetLoopFromHeader(block);

            if (nestedLoop != nullptr)
            {
                // We should have figured this out already.
                //
                assert(nestedLoop->m_cyclicProbability != 0);

                // Sum entry edges, multply by Cp
                //
                weight_t newWeight = 0.0;

                for (FlowEdge* const edge : nestedLoop->m_entryEdges)
                {
                    if (BasicBlock::sameHndRegion(block, edge->getSourceBlock()))
                    {
                        newWeight += edge->getLikelyWeight();
                    }
                }

                newWeight *= nestedLoop->m_cyclicProbability;
                block->bbWeight = newWeight;

                JITDUMP("ccp (nested header): " FMT_BB " :: " FMT_WT "\n", block->bbNum, newWeight);
            }
            else
            {
                weight_t newWeight = 0.0;

                for (FlowEdge* const edge : block->PredEdges())
                {
                    if (BasicBlock::sameHndRegion(block, edge->getSourceBlock()))
                    {
                        newWeight += edge->getLikelyWeight();
                    }
                }

                block->bbWeight = newWeight;

                JITDUMP("ccp: " FMT_BB " :: " FMT_WT "\n", block->bbNum, newWeight);
            }
        }
    }

    // Now look at cyclic flow back to the head block.
    //
    weight_t cyclicWeight = 0;
    bool     capped       = false;

    for (FlowEdge* const edge : loop->m_backEdges)
    {
        JITDUMP("ccp backedge " FMT_BB " (" FMT_WT ") -> " FMT_BB " likelihood " FMT_WT "\n",
                edge->getSourceBlock()->bbNum, edge->getSourceBlock()->bbWeight, loop->m_head->bbNum,
                edge->getLikelihood());

        cyclicWeight += edge->getLikelyWeight();
    }

    // Allow for a bit of rounding error, but not too much.
    // (todo: decrease loop gain if we are in a deep nest?)
    // assert(cyclicWeight <= 1.01);
    //
    if (cyclicWeight > 0.999)
    {
        capped       = true;
        cyclicWeight = 0.999;
        m_cappedCyclicProbabilities++;
    }

    weight_t cyclicProbability = 1.0 / (1.0 - cyclicWeight);

    JITDUMP("For loop at " FMT_BB " cyclic weight is " FMT_WT " cyclic probability is " FMT_WT "%s\n",
            loop->m_head->bbNum, cyclicWeight, cyclicProbability, capped ? " [capped]" : "");

    loop->m_cyclicProbability = cyclicProbability;
}

//------------------------------------------------------------------------
// fgAssignInputWeights: provide initial profile weights for all blocks
//
//
// Notes:
//   For finallys we may want to come back and rescale once we know the
//   weights of all the callfinallies, or perhaps just use the weight
//   of the first block in the associated try.
//
void ProfileSynthesis::AssignInputWeights()
{
    const weight_t entryWeight = BB_UNITY_WEIGHT;
    const weight_t ehWeight    = entryWeight / 1000;

    for (BasicBlock* block : m_comp->Blocks())
    {
        block->bbWeight = 0.0;
    }

    m_comp->fgFirstBB->bbWeight = entryWeight;

    for (EHblkDsc* const HBtab : EHClauses(m_comp))
    {
        if (HBtab->HasFilter())
        {
            HBtab->ebdFilter->bbWeight = ehWeight;
        }

        HBtab->ebdHndBeg->bbWeight = ehWeight;
    }
}

//------------------------------------------------------------------------
// ComputeBlockWeights: compute weights for all blocks
//   based on input weights, edge likelihoods, and cyclic probabilities
//
void ProfileSynthesis::ComputeBlockWeights()
{
    JITDUMP("Computing block weights\n");

    for (unsigned int i = 1; i <= m_comp->fgBBNumMax; i++)
    {
        BasicBlock* const block = m_comp->fgBBReversePostorder[i];
        SimpleLoop* const loop  = GetLoopFromHeader(block);

        if (loop != nullptr)
        {
            // Start with initial weight, sum entry edges, multiply by Cp
            //
            weight_t newWeight = block->bbWeight;

            for (FlowEdge* const edge : loop->m_entryEdges)
            {
                if (BasicBlock::sameHndRegion(block, edge->getSourceBlock()))
                {
                    newWeight += edge->getLikelyWeight();
                }
            }

            newWeight *= loop->m_cyclicProbability;
            block->bbWeight = newWeight;

            JITDUMP("cbw (header): " FMT_BB " :: " FMT_WT "\n", block->bbNum, block->bbWeight);
        }
        else
        {
            // start with initial weight, sum all incoming edges
            //
            weight_t newWeight = block->bbWeight;

            for (FlowEdge* const edge : block->PredEdges())
            {
                if (BasicBlock::sameHndRegion(block, edge->getSourceBlock()))
                {
                    newWeight += edge->getLikelyWeight();
                }
            }

            block->bbWeight = newWeight;

            JITDUMP("cbw: " FMT_BB " :: " FMT_WT "\n", block->bbNum, block->bbWeight);
        }

        // Todo: just use weight to determine run rarely, not flag
        //
        if (block->bbWeight == 0.0)
        {
            block->bbSetRunRarely();
        }
        else
        {
            block->bbFlags &= ~BBF_RUN_RARELY;
        }
    }
}
