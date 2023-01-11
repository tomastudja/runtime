// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
//
//                                    Early Value Propagation
//
// This phase performs an SSA-based value propagation optimization that currently only applies to array
// lengths and explicit null checks. An SSA-based backwards tracking of local variables
// is performed at each point of interest, e.g., an array length reference site, a method table reference site, or
// an indirection.
// The tracking continues until an interesting value is encountered. The value is then used to rewrite
// the source site or the value.
//
///////////////////////////////////////////////////////////////////////////////////////

#include "jitpch.h"
#include "ssabuilder.h"

bool Compiler::optDoEarlyPropForFunc()
{
    // TODO-MDArray: bool propMDArrayLen = (optMethodFlags & OMF_HAS_MDNEWARRAY) && (optMethodFlags &
    // OMF_HAS_MDARRAYREF);
    bool propArrayLen  = (optMethodFlags & OMF_HAS_NEWARRAY) && (optMethodFlags & OMF_HAS_ARRAYREF);
    bool propNullCheck = (optMethodFlags & OMF_HAS_NULLCHECK) != 0;
    return propArrayLen || propNullCheck;
}

bool Compiler::optDoEarlyPropForBlock(BasicBlock* block)
{
    // TODO-MDArray: bool bbHasMDArrayRef = (block->bbFlags & BBF_HAS_MD_IDX_LEN) != 0;
    bool bbHasArrayRef  = (block->bbFlags & BBF_HAS_IDX_LEN) != 0;
    bool bbHasNullCheck = (block->bbFlags & BBF_HAS_NULLCHECK) != 0;
    return bbHasArrayRef || bbHasNullCheck;
}

#ifdef DEBUG
//-----------------------------------------------------------------------------
// optCheckFlagsAreSet: Check that the method flag and the basic block flag are set.
//
// Arguments:
//    methodFlag           - The method flag to check.
//    methodFlagStr        - String representation of the method flag.
//    bbFlag               - The basic block flag to check.
//    bbFlagStr            - String representation of the basic block flag.
//    tree                 - Tree that makes the flags required.
//    basicBlock           - The basic block to check the flag on.

void Compiler::optCheckFlagsAreSet(unsigned    methodFlag,
                                   const char* methodFlagStr,
                                   unsigned    bbFlag,
                                   const char* bbFlagStr,
                                   GenTree*    tree,
                                   BasicBlock* basicBlock)
{
    if ((optMethodFlags & methodFlag) == 0)
    {
        printf("%s is not set on optMethodFlags but is required because of the following tree\n", methodFlagStr);
        gtDispTree(tree);
        assert(false);
    }

    if ((basicBlock->bbFlags & bbFlag) == 0)
    {
        printf("%s is not set on " FMT_BB " but is required because of the following tree \n", bbFlagStr,
               basicBlock->bbNum);
        gtDispTree(tree);
        assert(false);
    }
}
#endif

//------------------------------------------------------------------------------------------
// optEarlyProp: The entry point of the early value propagation.
//
// Returns:
//    suitable phase status
//
// Notes:
//    This phase performs an SSA-based value propagation, including array
//    length propagation and null check folding.
//
//    For array length propagation, a demand-driven SSA-based backwards tracking of constant
//    array lengths is performed at each array length reference site which is in form of a
//    GT_ARR_LENGTH node. When a GT_ARR_LENGTH node is seen, the array ref pointer which is
//    the only child node of the GT_ARR_LENGTH is tracked. This is only done for array ref
//    pointers that have valid SSA forms.The tracking is along SSA use-def chain and stops
//    at the original array allocation site where we can grab the array length. The
//    GT_ARR_LENGTH node will then be rewritten to a GT_CNS_INT node if the array length is
//    constant.
//
//    Null check folding tries to find GT_INDIR(obj + const) that GT_NULLCHECK(obj) can be folded into
//    and removed. Currently, the algorithm only matches GT_INDIR and GT_NULLCHECK in the same basic block.
//
//    TODO: support GT_MDARR_LENGTH, GT_MDARRAY_LOWER_BOUND
//
PhaseStatus Compiler::optEarlyProp()
{
    if (!optDoEarlyPropForFunc())
    {
        // We perhaps should verify the OMF are set properly
        //
        JITDUMP("no arrays or null checks in the method\n");
        return PhaseStatus::MODIFIED_NOTHING;
    }

    assert(fgSsaPassesCompleted == 1);
    unsigned numChanges = 0;

    for (BasicBlock* const block : Blocks())
    {
#ifndef DEBUG
        if (!optDoEarlyPropForBlock(block))
        {
            continue;
        }
#endif

        compCurBB = block;

        CompAllocator                 allocator(getAllocator(CMK_EarlyProp));
        LocalNumberToNullCheckTreeMap nullCheckMap(allocator);

        for (Statement* stmt = block->firstStmt(); stmt != nullptr;)
        {
            // Preserve the next link before the propagation and morph.
            Statement* next = stmt->GetNextStmt();

            compCurStmt = stmt;

            // Walk the stmt tree in linear order to rewrite any array length reference with a
            // constant array length.
            bool isRewritten = false;
            for (GenTree* tree = stmt->GetTreeList(); tree != nullptr; tree = tree->gtNext)
            {
                GenTree* rewrittenTree = optEarlyPropRewriteTree(tree, &nullCheckMap);
                if (rewrittenTree != nullptr)
                {
                    gtUpdateSideEffects(stmt, rewrittenTree);
                    isRewritten = true;
                    tree        = rewrittenTree;
                }
            }

            // Update the evaluation order and the statement info if the stmt has been rewritten.
            if (isRewritten)
            {
                // Make sure the transformation happens in debug, check, and release build.
                assert(optDoEarlyPropForFunc() && optDoEarlyPropForBlock(block));
                gtSetStmtInfo(stmt);
                fgSetStmtSeq(stmt);
                numChanges++;
            }

            stmt = next;
        }
    }

    JITDUMP("\nOptimized %u trees\n", numChanges);
    return numChanges > 0 ? PhaseStatus::MODIFIED_EVERYTHING : PhaseStatus::MODIFIED_NOTHING;
}

//----------------------------------------------------------------
// optEarlyPropRewriteValue: Rewrite a tree to the actual value.
//
// Arguments:
//    tree           - The input tree node to be rewritten.
//    nullCheckMap   - Map of the local numbers to the latest NULLCHECKs on those locals in the current basic block.
//
// Return Value:
//    Return a new tree if the original tree was successfully rewritten.
//    The containing tree links are updated.
//
GenTree* Compiler::optEarlyPropRewriteTree(GenTree* tree, LocalNumberToNullCheckTreeMap* nullCheckMap)
{
    GenTree*    objectRefPtr = nullptr;
    optPropKind propKind     = optPropKind::OPK_INVALID;
    bool        folded       = false;

    if (tree->OperIsIndirOrArrMetaData())
    {
        // optFoldNullCheck takes care of updating statement info if a null check is removed.
        folded = optFoldNullCheck(tree, nullCheckMap);
    }
    else
    {
        return nullptr;
    }

    if (tree->OperGet() == GT_ARR_LENGTH)
    {
        objectRefPtr = tree->AsOp()->gtOp1;
        propKind     = optPropKind::OPK_ARRAYLEN;
    }
    else
    {
        return folded ? tree : nullptr;
    }

    if (!objectRefPtr->OperIsScalarLocal() || !lvaInSsa(objectRefPtr->AsLclVarCommon()->GetLclNum()))
    {
        return folded ? tree : nullptr;
    }
#ifdef DEBUG
    else
    {
        if (propKind == optPropKind::OPK_ARRAYLEN)
        {
            optCheckFlagsAreSet(OMF_HAS_ARRAYREF, "OMF_HAS_ARRAYREF", BBF_HAS_IDX_LEN, "BBF_HAS_IDX_LEN", tree,
                                compCurBB);
        }
    }
#endif

    unsigned lclNum    = objectRefPtr->AsLclVarCommon()->GetLclNum();
    unsigned ssaNum    = objectRefPtr->AsLclVarCommon()->GetSsaNum();
    GenTree* actualVal = optPropGetValue(lclNum, ssaNum, propKind);

    if (actualVal != nullptr)
    {
        assert(propKind == optPropKind::OPK_ARRAYLEN);
        assert(actualVal->IsCnsIntOrI() && !actualVal->IsIconHandle());
        assert(actualVal->GetNodeSize() == TREE_NODE_SZ_SMALL);

        ssize_t actualConstVal = actualVal->AsIntCon()->IconValue();

        if (propKind == optPropKind::OPK_ARRAYLEN)
        {
            if ((actualConstVal < 0) || (actualConstVal > INT32_MAX))
            {
                // Don't propagate array lengths that are beyond the maximum value of a GT_ARR_LENGTH or negative.
                // node. CORINFO_HELP_NEWARR_1_OBJ helper call allows to take a long integer as the
                // array length argument, but the type of GT_ARR_LENGTH is always INT32.
                return nullptr;
            }

            // When replacing GT_ARR_LENGTH nodes with constants we can end up with GT_BOUNDS_CHECK
            // nodes that have constant operands and thus can be trivially proved to be useless. It's
            // better to remove these range checks here, otherwise they'll pass through assertion prop
            // (creating useless (c1 < c2)-like assertions) and reach RangeCheck where they are finally
            // removed. Common patterns like new int[] { x, y, z } benefit from this.

            if ((tree->gtNext != nullptr) && tree->gtNext->OperIs(GT_BOUNDS_CHECK))
            {
                GenTreeBoundsChk* check = tree->gtNext->AsBoundsChk();

                if ((check->GetArrayLength() == tree) && check->GetIndex()->IsCnsIntOrI())
                {
                    ssize_t checkConstVal = check->GetIndex()->AsIntCon()->IconValue();
                    if ((checkConstVal >= 0) && (checkConstVal < actualConstVal))
                    {
                        GenTree* comma = check->gtGetParent(nullptr);

                        // We should never see cases other than these in the IR,
                        // as the check node does not produce a value.
                        assert(((comma != nullptr) && comma->OperIs(GT_COMMA) &&
                                (comma->gtGetOp1() == check || comma->TypeIs(TYP_VOID))) ||
                               (check == compCurStmt->GetRootNode()));

                        // Still, we guard here so that release builds do not try to optimize trees we don't understand.
                        if (((comma != nullptr) && comma->OperIs(GT_COMMA) && (comma->gtGetOp1() == check)) ||
                            (check == compCurStmt->GetRootNode()))
                        {
                            // Both `tree` and `check` have been removed from the statement.
                            // 'tree' was replaced with 'nop' or side effect list under 'comma'.
                            // optRemoveRangeCheck returns this modified tree.
                            return optRemoveRangeCheck(check, comma, compCurStmt);
                        }
                    }
                }
            }
        }

#ifdef DEBUG
        if (verbose)
        {
            printf("optEarlyProp Rewriting " FMT_BB "\n", compCurBB->bbNum);
            gtDispStmt(compCurStmt);
            printf("\n");
        }
#endif

        GenTree* actualValClone = gtCloneExpr(actualVal);

        if (actualValClone->gtType != tree->gtType)
        {
            assert(actualValClone->gtType == TYP_LONG);
            assert(tree->gtType == TYP_INT);
            assert((actualConstVal >= 0) && (actualConstVal <= INT32_MAX));
            actualValClone->gtType = tree->gtType;
        }

        // actualValClone has small tree node size, it is safe to use CopyFrom here.
        tree->ReplaceWith(actualValClone, this);

        // update SSA accounting
        optRecordSsaUses(tree, compCurBB);

        // Propagating a constant may create an opportunity to use a division by constant optimization
        //
        if ((tree->gtNext != nullptr) && tree->gtNext->OperIsBinary())
        {
            // We need to mark the parent divide/mod operation when this occurs
            tree->gtNext->AsOp()->CheckDivideByConstOptimized(this);
        }

#ifdef DEBUG
        if (verbose)
        {
            printf("to\n");
            gtDispStmt(compCurStmt);
            printf("\n");
        }
#endif
        return tree;
    }

    return nullptr;
}

//-------------------------------------------------------------------------------------------
// optPropGetValue: Given an SSA object ref pointer, get the value needed based on valueKind.
//
// Arguments:
//    lclNum         - The local var number of the ref pointer.
//    ssaNum         - The SSA var number of the ref pointer.
//    valueKind      - The kind of value of interest.
//
// Return Value:
//    Return the corresponding value based on valueKind.

GenTree* Compiler::optPropGetValue(unsigned lclNum, unsigned ssaNum, optPropKind valueKind)
{
    return optPropGetValueRec(lclNum, ssaNum, valueKind, 0);
}

//-----------------------------------------------------------------------------------
// optPropGetValueRec: Given an SSA object ref pointer, get the value needed based on valueKind
//                     within a recursion bound.
//
// Arguments:
//    lclNum         - The local var number of the array pointer.
//    ssaNum         - The SSA var number of the array pointer.
//    valueKind      - The kind of value of interest.
//    walkDepth      - Current recursive walking depth.
//
// Return Value:
//    Return the corresponding value based on valueKind.

GenTree* Compiler::optPropGetValueRec(unsigned lclNum, unsigned ssaNum, optPropKind valueKind, int walkDepth)
{
    if (ssaNum == SsaConfig::RESERVED_SSA_NUM)
    {
        return nullptr;
    }

    GenTree* value = nullptr;

    // Bound the recursion with a hard limit.
    if (walkDepth > optEarlyPropRecurBound)
    {
        return nullptr;
    }

    // Track along the use-def chain to get the array length
    LclSsaVarDsc* ssaVarDsc = lvaTable[lclNum].GetPerSsaData(ssaNum);
    GenTreeOp*    ssaDefAsg = ssaVarDsc->GetAssignment();

    // Incoming parameters or live-in variables don't have actual definition tree node for
    // their FIRST_SSA_NUM. Definitions induced by calls do not record the store node. See
    // SsaBuilder::RenameDef.
    if (ssaDefAsg != nullptr)
    {
        assert(ssaDefAsg->OperIs(GT_ASG));

        GenTree* treeLhs = ssaDefAsg->gtGetOp1();
        GenTree* treeRhs = ssaDefAsg->gtGetOp2();

        // Recursively track the Rhs for "entire" stores.
        if (treeLhs->OperIs(GT_LCL_VAR) && (treeLhs->AsLclVar()->GetLclNum() == lclNum) && treeRhs->OperIs(GT_LCL_VAR))
        {
            unsigned rhsLclNum = treeRhs->AsLclVarCommon()->GetLclNum();
            unsigned rhsSsaNum = treeRhs->AsLclVarCommon()->GetSsaNum();

            value = optPropGetValueRec(rhsLclNum, rhsSsaNum, valueKind, walkDepth + 1);
        }
        else
        {
            if (valueKind == optPropKind::OPK_ARRAYLEN)
            {
                value = getArrayLengthFromAllocation(treeRhs DEBUGARG(ssaVarDsc->GetBlock()));
                if (value != nullptr)
                {
                    if (!value->IsCnsIntOrI())
                    {
                        // Leave out non-constant-sized array
                        value = nullptr;
                    }
                }
            }
        }
    }

    return value;
}

//----------------------------------------------------------------
// optFoldNullChecks: Try to find a GT_NULLCHECK node that can be folded into the indirection node mark it for removal
// if possible.
//
// Arguments:
//    tree           - The input indirection tree.
//    nullCheckMap   - Map of the local numbers to the latest NULLCHECKs on those locals in the current basic block
//
// Returns:
//    true if a null check was folded
//
// Notes:
//    If a GT_NULLCHECK node is post-dominated by an indirection node on the same local and the trees between
//    the GT_NULLCHECK and the indirection don't have unsafe side effects, the GT_NULLCHECK can be removed.
//    The indir will cause a NullReferenceException if and only if GT_NULLCHECK will cause the same
//    NullReferenceException.

bool Compiler::optFoldNullCheck(GenTree* tree, LocalNumberToNullCheckTreeMap* nullCheckMap)
{
#ifdef DEBUG
    if (tree->OperGet() == GT_NULLCHECK)
    {
        optCheckFlagsAreSet(OMF_HAS_NULLCHECK, "OMF_HAS_NULLCHECK", BBF_HAS_NULLCHECK, "BBF_HAS_NULLCHECK", tree,
                            compCurBB);
    }
#else
    if ((compCurBB->bbFlags & BBF_HAS_NULLCHECK) == 0)
    {
        return false;
    }
#endif

    GenTree*   nullCheckTree   = optFindNullCheckToFold(tree, nullCheckMap);
    GenTree*   nullCheckParent = nullptr;
    Statement* nullCheckStmt   = nullptr;
    bool       folded          = false;
    if ((nullCheckTree != nullptr) && optIsNullCheckFoldingLegal(tree, nullCheckTree, &nullCheckParent, &nullCheckStmt))
    {
#ifdef DEBUG
        // Make sure the transformation happens in debug, check, and release build.
        assert(optDoEarlyPropForFunc() && optDoEarlyPropForBlock(compCurBB) &&
               (compCurBB->bbFlags & BBF_HAS_NULLCHECK) != 0);
        if (verbose)
        {
            printf("optEarlyProp Marking a null check for removal\n");
            gtDispTree(nullCheckTree);
            printf("\n");
        }
#endif
        // Remove the null check
        nullCheckTree->gtFlags &= ~(GTF_EXCEPT | GTF_DONT_CSE);

        // Set this flag to prevent reordering
        nullCheckTree->gtFlags |= GTF_ORDER_SIDEEFF;
        nullCheckTree->gtFlags |= GTF_IND_NONFAULTING;

        if (nullCheckParent != nullptr)
        {
            nullCheckParent->gtFlags &= ~GTF_DONT_CSE;
        }

        nullCheckMap->Remove(nullCheckTree->gtGetOp1()->AsLclVarCommon()->GetLclNum());

        // Re-morph the statement.
        Statement* curStmt = compCurStmt;
        fgMorphBlockStmt(compCurBB, nullCheckStmt DEBUGARG("optFoldNullCheck"));
        optRecordSsaUses(nullCheckStmt->GetRootNode(), compCurBB);
        compCurStmt = curStmt;

        folded = true;
    }

    if ((tree->OperGet() == GT_NULLCHECK) && (tree->gtGetOp1()->OperGet() == GT_LCL_VAR))
    {
        nullCheckMap->Set(tree->gtGetOp1()->AsLclVarCommon()->GetLclNum(), tree,
                          LocalNumberToNullCheckTreeMap::SetKind::Overwrite);
    }

    return folded;
}

//----------------------------------------------------------------
// optFindNullCheckToFold: Try to find a GT_NULLCHECK node that can be folded into the indirection node.
//
// Arguments:
//    tree           - The input indirection tree.
//    nullCheckMap   - Map of the local numbers to the latest NULLCHECKs on those locals in the current basic block
//
// Notes:
//    Check for cases where
//    1. One of the following trees
//
//       nullcheck(x)
//       or
//       x = comma(nullcheck(y), add(y, const1))
//
//       is post-dominated in the same basic block by one of the following trees
//
//       indir(x)
//       or
//       indir(add(x, const2))
//
//       (indir is any node for which OperIsIndirOrArrMetaData() is true.)
//
//     2.  const1 + const2 if sufficiently small.

GenTree* Compiler::optFindNullCheckToFold(GenTree* tree, LocalNumberToNullCheckTreeMap* nullCheckMap)
{
    assert(tree->OperIsIndirOrArrMetaData());

    GenTree* addr = tree->GetIndirOrArrMetaDataAddr();

    ssize_t offsetValue = 0;

    if ((addr->OperGet() == GT_ADD) && addr->gtGetOp2()->IsCnsIntOrI())
    {
        offsetValue += addr->gtGetOp2()->AsIntConCommon()->IconValue();
        addr = addr->gtGetOp1();
    }

    if (addr->OperGet() != GT_LCL_VAR)
    {
        return nullptr;
    }

    GenTreeLclVarCommon* const lclVarNode = addr->AsLclVarCommon();
    const unsigned             ssaNum     = lclVarNode->GetSsaNum();

    if (ssaNum == SsaConfig::RESERVED_SSA_NUM)
    {
        return nullptr;
    }

    const unsigned lclNum          = lclVarNode->GetLclNum();
    GenTree*       nullCheckTree   = nullptr;
    unsigned       nullCheckLclNum = BAD_VAR_NUM;

    // Check if we saw a nullcheck on this local in this basic block
    // This corresponds to nullcheck(x) tree in the header comment.
    if (nullCheckMap->Lookup(lclNum, &nullCheckTree))
    {
        GenTree* nullCheckAddr = nullCheckTree->AsIndir()->Addr();
        if ((nullCheckAddr->OperGet() != GT_LCL_VAR) || (nullCheckAddr->AsLclVarCommon()->GetSsaNum() != ssaNum))
        {
            nullCheckTree = nullptr;
        }
        else
        {
            nullCheckLclNum = nullCheckAddr->AsLclVarCommon()->GetLclNum();
        }
    }

    if (nullCheckTree == nullptr)
    {
        // Check if we have x = comma(nullcheck(y), add(y, const1)) pattern.

        // Find the definition of the indirected local ('x' in the pattern above).
        LclSsaVarDsc* defLoc = lvaTable[lclNum].GetPerSsaData(ssaNum);

        if (compCurBB != defLoc->GetBlock())
        {
            return nullptr;
        }

        GenTree* defNode = defLoc->GetAssignment();
        if (defNode == nullptr)
        {
            return nullptr;
        }

        GenTree* defLHS = defNode->gtGetOp1();
        if (!defLHS->OperIs(GT_LCL_VAR) || (defLHS->AsLclVar()->GetLclNum() != lclNum))
        {
            return nullptr;
        }

        GenTree* defRHS = defNode->gtGetOp2();
        if (defRHS->OperGet() != GT_COMMA)
        {
            return nullptr;
        }

        const bool commaOnly              = true;
        GenTree*   commaOp1EffectiveValue = defRHS->gtGetOp1()->gtEffectiveVal(commaOnly);

        if (commaOp1EffectiveValue->OperGet() != GT_NULLCHECK)
        {
            return nullptr;
        }

        GenTree* nullCheckAddress = commaOp1EffectiveValue->gtGetOp1();

        if ((nullCheckAddress->OperGet() != GT_LCL_VAR) || (defRHS->gtGetOp2()->OperGet() != GT_ADD))
        {
            return nullptr;
        }

        // We found a candidate for 'y' in the pattern above.

        GenTree* additionNode = defRHS->gtGetOp2();
        GenTree* additionOp1  = additionNode->gtGetOp1();
        GenTree* additionOp2  = additionNode->gtGetOp2();
        if ((additionOp1->OperGet() == GT_LCL_VAR) &&
            (additionOp1->AsLclVarCommon()->GetLclNum() == nullCheckAddress->AsLclVarCommon()->GetLclNum()) &&
            (additionOp2->IsCnsIntOrI()))
        {
            offsetValue += additionOp2->AsIntConCommon()->IconValue();
            nullCheckTree = commaOp1EffectiveValue;
        }
    }

    if (fgIsBigOffset(offsetValue))
    {
        return nullptr;
    }
    else
    {
        return nullCheckTree;
    }
}

//----------------------------------------------------------------
// optIsNullCheckFoldingLegal: Check the nodes between the GT_NULLCHECK node and the indirection to determine
//                             if null check folding is legal.
//
// Arguments:
//    tree                - The input indirection tree.
//    nullCheckTree       - The GT_NULLCHECK tree that is a candidate for removal.
//    nullCheckParent     - The parent of the GT_NULLCHECK tree that is a candidate for removal (out-parameter).
//    nullCheckStatement  - The statement of the GT_NULLCHECK tree that is a candidate for removal (out-parameter).

bool Compiler::optIsNullCheckFoldingLegal(GenTree*    tree,
                                          GenTree*    nullCheckTree,
                                          GenTree**   nullCheckParent,
                                          Statement** nullCheckStmt)
{
    // Check all nodes between the GT_NULLCHECK and the indirection to see
    // if any nodes have unsafe side effects.
    unsigned       nullCheckLclNum    = nullCheckTree->gtGetOp1()->AsLclVarCommon()->GetLclNum();
    bool           isInsideTry        = compCurBB->hasTryIndex();
    bool           canRemoveNullCheck = true;
    const unsigned maxNodesWalked     = 50;
    unsigned       nodesWalked        = 0;

    // First walk the nodes in the statement containing the GT_NULLCHECK in forward execution order
    // until we get to the indirection or process the statement root.
    GenTree* previousTree = nullCheckTree;
    GenTree* currentTree  = nullCheckTree->gtNext;
    assert(fgNodeThreading == NodeThreading::AllTrees);
    while (canRemoveNullCheck && (currentTree != tree) && (currentTree != nullptr))
    {
        if ((*nullCheckParent == nullptr) && currentTree->TryGetUse(nullCheckTree))
        {
            *nullCheckParent = currentTree;
        }
        const bool checkExceptionSummary = false;
        if ((nodesWalked++ > maxNodesWalked) ||
            !optCanMoveNullCheckPastTree(currentTree, nullCheckLclNum, isInsideTry, checkExceptionSummary))
        {
            canRemoveNullCheck = false;
        }
        else
        {
            previousTree = currentTree;
            currentTree  = currentTree->gtNext;
        }
    }

    if (currentTree == tree)
    {
        // The GT_NULLCHECK and the indirection are in the same statements.
        *nullCheckStmt = compCurStmt;
    }
    else
    {
        // The GT_NULLCHECK and the indirection are in different statements.
        // Walk the nodes in the statement containing the indirection
        // in reverse execution order starting with the indirection's
        // predecessor.
        GenTree* nullCheckStatementRoot = previousTree;
        currentTree                     = tree->gtPrev;
        while (canRemoveNullCheck && (currentTree != nullptr))
        {
            const bool checkExceptionSummary = false;
            if ((nodesWalked++ > maxNodesWalked) ||
                !optCanMoveNullCheckPastTree(currentTree, nullCheckLclNum, isInsideTry, checkExceptionSummary))
            {
                canRemoveNullCheck = false;
            }
            else
            {
                currentTree = currentTree->gtPrev;
            }
        }

        // Finally, walk the statement list in reverse execution order
        // until we get to the statement containing the null check.
        // We only check the side effects at the root of each statement.
        Statement* curStmt = compCurStmt->GetPrevStmt();
        currentTree        = curStmt->GetRootNode();
        while (canRemoveNullCheck && (currentTree != nullCheckStatementRoot))
        {
            const bool checkExceptionSummary = true;
            if ((nodesWalked++ > maxNodesWalked) ||
                !optCanMoveNullCheckPastTree(currentTree, nullCheckLclNum, isInsideTry, checkExceptionSummary))
            {
                canRemoveNullCheck = false;
            }
            else
            {
                curStmt     = curStmt->GetPrevStmt();
                currentTree = curStmt->GetRootNode();
            }
        }
        *nullCheckStmt = curStmt;
    }

    if (canRemoveNullCheck && (*nullCheckParent == nullptr))
    {
        *nullCheckParent = nullCheckTree->gtGetParent(nullptr);
    }

    return canRemoveNullCheck;
}

//----------------------------------------------------------------
// optCanMoveNullCheckPastTree: Check if a nullcheck node that is before `tree`
//                              in execution order may be folded into an indirection node that
//                              is after `tree` is execution order.
//
// Arguments:
//    tree                  - The tree to check.
//    nullCheckLclNum       - The local variable that GT_NULLCHECK checks.
//    isInsideTry           - True if tree is inside try, false otherwise.
//    checkSideEffectSummary -If true, check side effect summary flags only,
//                            otherwise check the side effects of the operation itself.
//
// Return Value:
//    True if nullcheck may be folded into a node that is after tree in execution order,
//    false otherwise.

bool Compiler::optCanMoveNullCheckPastTree(GenTree* tree,
                                           unsigned nullCheckLclNum,
                                           bool     isInsideTry,
                                           bool     checkSideEffectSummary)
{
    bool result = true;

    if ((tree->gtFlags & GTF_CALL) != 0)
    {
        result = !checkSideEffectSummary && !tree->OperRequiresCallFlag(this);
    }

    if (result && (tree->gtFlags & GTF_EXCEPT) != 0)
    {
        result = !checkSideEffectSummary && !tree->OperMayThrow(this);
    }

    if (result && ((tree->gtFlags & GTF_ASG) != 0))
    {
        if (tree->OperGet() == GT_ASG)
        {
            GenTree* lhs = tree->gtGetOp1();
            GenTree* rhs = tree->gtGetOp2();
            if (checkSideEffectSummary && ((rhs->gtFlags & GTF_ASG) != 0))
            {
                result = false;
            }
            else if (isInsideTry)
            {
                // Inside try we allow only assignments to locals not live in handlers.
                // lvVolatileHint is set to true on variables that are line in handlers.
                result = (lhs->OperGet() == GT_LCL_VAR) && !lvaTable[lhs->AsLclVarCommon()->GetLclNum()].lvVolatileHint;
            }
            else
            {
                // We disallow only assignments to global memory.
                result = ((lhs->gtFlags & GTF_GLOB_REF) == 0);
            }
        }
        else if (checkSideEffectSummary)
        {
            result = !isInsideTry && ((tree->gtFlags & GTF_GLOB_REF) == 0);
        }
        else
        {
            result = !isInsideTry && (!tree->OperRequiresAsgFlag() || ((tree->gtFlags & GTF_GLOB_REF) == 0));
        }
    }

    return result;
}
