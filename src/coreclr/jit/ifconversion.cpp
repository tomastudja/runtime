// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XX                                                                           XX
XX                              OptIfConversion                              XX
XX                                                                           XX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/

#include "jitpch.h"
#ifdef _MSC_VER
#pragma hdrstop
#endif

//-----------------------------------------------------------------------------
// OptIfConversionDsc:     Descriptor used for If conversion
//
class OptIfConversionDsc
{
public:
    OptIfConversionDsc(Compiler* comp, BasicBlock* startBlock)
    {
        m_comp       = comp;
        m_startBlock = startBlock;
    }

private:
    Compiler* m_comp; // The Compiler instance.

    BasicBlock* m_startBlock;           // First block in the If Conversion.
    BasicBlock* m_finalBlock = nullptr; // Block where the flows merge. In a return case, this can be nullptr.

    // The node, statement and block of an assignment.
    struct IfConvertOperation
    {
        BasicBlock* block = nullptr;
        Statement*  stmt  = nullptr;
        GenTree*    node  = nullptr;
    };

    GenTree*           m_cond;          // The condition in the conversion
    IfConvertOperation m_thenOperation; // The single operation in the Then case.
    IfConvertOperation m_elseOperation; // The single operation in the Else case.

    int m_checkLimit = 4; // Max number of chained blocks to allow in both the True and Else cases.

    genTreeOps m_mainOper         = GT_COUNT; // The main oper of the if conversion.
    bool       m_doElseConversion = false;    // Does the If conversion have an else statement.
    bool       m_flowFound        = false;    // Has a valid flow been found.

    bool IfConvertCheckInnerBlockFlow(BasicBlock* block);
    bool IfConvertCheckThenFlow();
    void IfConvertFindFlow();
    bool IfConvertCheckStmts(BasicBlock* fromBlock, IfConvertOperation* foundOperation);
    void IfConvertJoinStmts(BasicBlock* fromBlock);

#ifdef DEBUG
    void IfConvertDump();
#endif

    bool IsHWIntrinsicCC(GenTree* node);

public:
    bool optIfConvert();
};

//-----------------------------------------------------------------------------
// IfConvertCheckInnerBlockFlow
//
// Check if the flow of a block is valid for use as an inner block (either a Then or Else block)
// in an If Conversion.
//
// Assumptions:
//   m_startBlock and m_doElseConversion are set.
//
// Arguments:
//   block -- Block to check.
//
// Returns:
//   True if Checks are ok, else false.
//
bool OptIfConversionDsc::IfConvertCheckInnerBlockFlow(BasicBlock* block)
{
    // Block should have a single successor or be a return.
    if (!(block->GetUniqueSucc() != nullptr || (m_doElseConversion && (block->bbJumpKind == BBJ_RETURN))))
    {
        return false;
    }

    // Check that we have linear flow and are still in the same EH region

    if (block->GetUniquePred(m_comp) == nullptr)
    {
        return false;
    }

    if (!BasicBlock::sameEHRegion(block, m_startBlock))
    {
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
// IfConvertCheckThenFlow
//
// Check all the Then blocks between m_startBlock and m_finalBlock are valid.
//
// Assumptions:
//   m_startBlock, m_finalBlock and m_doElseConversion are set.
//
// Returns:
//   If a conversion is found, then set m_flowFound and return true.
//   If a conversion is not found, and it's ok to keep searching, return true.
//   Otherwise, return false.
//
// Notes:
//   Sets m_flowFound and m_mainOper.
//
bool OptIfConversionDsc::IfConvertCheckThenFlow()
{
    m_flowFound           = false;
    BasicBlock* thenBlock = m_startBlock->bbNext;

    for (int thenLimit = 0; thenLimit < m_checkLimit; thenLimit++)
    {
        if (!IfConvertCheckInnerBlockFlow(thenBlock))
        {
            // Then block is not in a valid flow.
            return true;
        }
        BasicBlock* thenBlockNext = thenBlock->GetUniqueSucc();

        if (thenBlockNext == m_finalBlock)
        {
            // All the Then blocks up to m_finalBlock are in a valid flow.
            m_flowFound = true;
            if (thenBlock->bbJumpKind == BBJ_RETURN)
            {
                assert(m_finalBlock == nullptr);
                m_mainOper = GT_RETURN;
            }
            else
            {
                m_mainOper = GT_ASG;
            }
            return true;
        }

        if (thenBlockNext == nullptr)
        {
            // Invalid Then and Else combination.
            return false;
        }

        thenBlock = thenBlockNext;
    }

    // Nothing found. Still valid to continue.
    return true;
}

//-----------------------------------------------------------------------------
// IfConvertFindFlow
//
// Find a valid if conversion flow from m_startBlock to a final block.
// There might be multiple Then and Else blocks in the flow - use m_checkLimit to limit this.
//
// Notes:
//   Sets m_flowFound, m_finalBlock, m_doElseConversion and m_mainOper.
//
void OptIfConversionDsc::IfConvertFindFlow()
{
    // First check for flow with no else case. The final block is the destination of the jump.
    m_doElseConversion = false;
    m_finalBlock       = m_startBlock->bbJumpDest;
    assert(m_finalBlock != nullptr);
    if (!IfConvertCheckThenFlow() || m_flowFound)
    {
        // Either the flow is invalid, or a flow was found.
        return;
    }

    // Look for flows with else blocks. The final block is the block after the else block.
    m_doElseConversion = true;
    for (int elseLimit = 0; elseLimit < m_checkLimit; elseLimit++)
    {
        BasicBlock* elseBlock = m_finalBlock;
        if (elseBlock == nullptr || !IfConvertCheckInnerBlockFlow(elseBlock))
        {
            // Need a valid else block in a valid flow .
            return;
        }

        m_finalBlock = elseBlock->GetUniqueSucc();

        if (!IfConvertCheckThenFlow() || m_flowFound)
        {
            // Either the flow is invalid, or a flow was found.
            return;
        }
    }
}

//-----------------------------------------------------------------------------
// IfConvertCheckStmts
//
// From the given block to the final block, check all the statements and nodes are
// valid for an If conversion. Chain of blocks must contain only a single assignment
// and no other operations.
//
// Arguments:
//   fromBlock  -- Block inside the if statement to start from (Either Then or Else path).
//   foundOperation -- Returns the found operation.
//
// Returns:
//   If everything is valid, then set foundOperation to the assignment and return true.
//   Otherwise return false.
//
bool OptIfConversionDsc::IfConvertCheckStmts(BasicBlock* fromBlock, IfConvertOperation* foundOperation)
{
    bool found = false;

    for (BasicBlock* block = fromBlock; block != m_finalBlock; block = block->GetUniqueSucc())
    {
        assert(block != nullptr);

        // Can all the nodes within the block be made to conditionally execute?
        for (Statement* const stmt : block->Statements())
        {
            GenTree* tree = stmt->GetRootNode();
            switch (tree->gtOper)
            {
                case GT_ASG:
                {
                    GenTree* op1 = tree->gtGetOp1();
                    GenTree* op2 = tree->gtGetOp2();

                    // Only one per operation per block can be conditionally executed.
                    if (found)
                    {
                        return false;
                    }

                    // Ensure the operarand is a local variable with integer type.
                    if (!op1->OperIs(GT_LCL_VAR) || !varTypeIsIntegralOrI(op1))
                    {
                        return false;
                    }

#ifndef TARGET_64BIT
                    // Disallow 64-bit operands on 32-bit targets as the backend currently cannot
                    // handle contained relops efficiently after decomposition.
                    if (varTypeIsLong(tree))
                    {
                        return false;
                    }
#endif

                    // Ensure it won't cause any additional side effects.
                    if ((op1->gtFlags & (GTF_SIDE_EFFECT | GTF_ORDER_SIDEEFF)) != 0 ||
                        (op2->gtFlags & (GTF_SIDE_EFFECT | GTF_ORDER_SIDEEFF)) != 0)
                    {
                        return false;
                    }

                    // Ensure the source isn't a phi.
                    if (op2->OperIs(GT_PHI))
                    {
                        return false;
                    }

                    // Evaluating unconditionally effectively has the same effect as reordering
                    // with the condition (for example, the condition could be an explicit bounds
                    // check and the operand could read an array element). Disallow this except
                    // for some common cases that we know are always side effect free.
                    if (((m_cond->gtFlags & GTF_ORDER_SIDEEFF) != 0) && !op2->IsInvariant() && !op2->OperIsLocal())
                    {
                        return false;
                    }

                    found                 = true;
                    foundOperation->block = block;
                    foundOperation->stmt  = stmt;
                    foundOperation->node  = tree;
                    break;
                }

                case GT_RETURN:
                {
                    GenTree* op1 = tree->gtGetOp1();

                    // Only allow RETURNs if else conversion is being used.
                    if (!m_doElseConversion)
                    {
                        return false;
                    }

                    // Only one per operation per block can be conditionally executed.
                    if (found || op1 == nullptr)
                    {
                        return false;
                    }

                    // Ensure the operation has integer type.
                    if (!varTypeIsIntegralOrI(tree))
                    {
                        return false;
                    }

#ifndef TARGET_64BIT
                    // Disallow 64-bit operands on 32-bit targets as the backend currently cannot
                    // handle contained relops efficiently after decomposition.
                    if (varTypeIsLong(tree))
                    {
                        return false;
                    }
#endif

                    // Ensure it won't cause any additional side effects.
                    if ((op1->gtFlags & (GTF_SIDE_EFFECT | GTF_ORDER_SIDEEFF)) != 0)
                    {
                        return false;
                    }

                    // Evaluating unconditionally effectively has the same effect as reordering
                    // with the condition (for example, the condition could be an explicit bounds
                    // check and the operand could read an array element). Disallow this except
                    // for some common cases that we know are always side effect free.
                    if (((m_cond->gtFlags & GTF_ORDER_SIDEEFF) != 0) && !op1->IsInvariant() && !op1->OperIsLocal())
                    {
                        return false;
                    }

                    found                 = true;
                    foundOperation->block = block;
                    foundOperation->stmt  = stmt;
                    foundOperation->node  = tree;
                    break;
                }

                // These do not need conditional execution.
                case GT_NOP:
                    if (tree->gtGetOp1() != nullptr || (tree->gtFlags & (GTF_SIDE_EFFECT | GTF_ORDER_SIDEEFF)) != 0)
                    {
                        return false;
                    }
                    break;

                // Cannot optimise this block.
                default:
                    return false;
            }
        }
    }
    return found;
}

//-----------------------------------------------------------------------------
// IfConvertJoinStmts
//
// Move all the statements from a block onto the end of the start block.
//
// Arguments:
//   fromBlock  -- Source block
//
void OptIfConversionDsc::IfConvertJoinStmts(BasicBlock* fromBlock)
{
    Statement* stmtList1 = m_startBlock->firstStmt();
    Statement* stmtList2 = fromBlock->firstStmt();
    Statement* stmtLast1 = m_startBlock->lastStmt();
    Statement* stmtLast2 = fromBlock->lastStmt();
    stmtLast1->SetNextStmt(stmtList2);
    stmtList2->SetPrevStmt(stmtLast1);
    stmtList1->SetPrevStmt(stmtLast2);
    fromBlock->bbStmtList = nullptr;
}

//-----------------------------------------------------------------------------
// IfConvertDump
//
// Dump all the blocks in the If Conversion.
//
#ifdef DEBUG
void OptIfConversionDsc::IfConvertDump()
{
    assert(m_startBlock != nullptr);
    m_comp->fgDumpBlock(m_startBlock);
    for (BasicBlock* dumpBlock = m_startBlock->bbNext; dumpBlock != m_finalBlock;
         dumpBlock             = dumpBlock->GetUniqueSucc())
    {
        m_comp->fgDumpBlock(dumpBlock);
    }
    if (m_doElseConversion)
    {
        for (BasicBlock* dumpBlock = m_startBlock->bbJumpDest; dumpBlock != m_finalBlock;
             dumpBlock             = dumpBlock->GetUniqueSucc())
        {
            m_comp->fgDumpBlock(dumpBlock);
        }
    }
}
#endif

#ifdef TARGET_XARCH
//-----------------------------------------------------------------------------
// IsHWIntrinsicCC:
//   Check if this is a HW intrinsic node that can be compared efficiently
//   against 0.
//
// Returns:
//   True if so.
//
// Notes:
//   For xarch, we currently skip if-conversion for these cases as the backend can handle them more efficiently
//   when they are normal compares.
//
bool OptIfConversionDsc::IsHWIntrinsicCC(GenTree* node)
{
#ifdef FEATURE_HW_INTRINSICS
    if (!node->OperIs(GT_HWINTRINSIC))
    {
        return false;
    }

    switch (node->AsHWIntrinsic()->GetHWIntrinsicId())
    {
        case NI_SSE_CompareScalarOrderedEqual:
        case NI_SSE_CompareScalarOrderedNotEqual:
        case NI_SSE_CompareScalarOrderedLessThan:
        case NI_SSE_CompareScalarOrderedLessThanOrEqual:
        case NI_SSE_CompareScalarOrderedGreaterThan:
        case NI_SSE_CompareScalarOrderedGreaterThanOrEqual:
        case NI_SSE_CompareScalarUnorderedEqual:
        case NI_SSE_CompareScalarUnorderedNotEqual:
        case NI_SSE_CompareScalarUnorderedLessThanOrEqual:
        case NI_SSE_CompareScalarUnorderedLessThan:
        case NI_SSE_CompareScalarUnorderedGreaterThanOrEqual:
        case NI_SSE_CompareScalarUnorderedGreaterThan:
        case NI_SSE2_CompareScalarOrderedEqual:
        case NI_SSE2_CompareScalarOrderedNotEqual:
        case NI_SSE2_CompareScalarOrderedLessThan:
        case NI_SSE2_CompareScalarOrderedLessThanOrEqual:
        case NI_SSE2_CompareScalarOrderedGreaterThan:
        case NI_SSE2_CompareScalarOrderedGreaterThanOrEqual:
        case NI_SSE2_CompareScalarUnorderedEqual:
        case NI_SSE2_CompareScalarUnorderedNotEqual:
        case NI_SSE2_CompareScalarUnorderedLessThanOrEqual:
        case NI_SSE2_CompareScalarUnorderedLessThan:
        case NI_SSE2_CompareScalarUnorderedGreaterThanOrEqual:
        case NI_SSE2_CompareScalarUnorderedGreaterThan:
        case NI_SSE41_TestC:
        case NI_SSE41_TestZ:
        case NI_SSE41_TestNotZAndNotC:
        case NI_AVX_TestC:
        case NI_AVX_TestZ:
        case NI_AVX_TestNotZAndNotC:
            return true;
        default:
            return false;
    }
#else
    return false;
#endif
}
#endif

//-----------------------------------------------------------------------------
// optIfConvert
//
// Find blocks representing simple if statements represented by conditional jumps
// over another block. Try to replace the jumps by use of SELECT nodes.
//
// Returns:
//   true if any IR changes possibly made.
//
// Notes:
//
// Example of simple if conversion:
//
// This is optimising a simple if statement. There is a single condition being
// tested, and a single assignment inside the body. There must be no else
// statement. For example:
// if (x < 7) { a = 5; }
//
// This is represented in IR by two basic blocks. The first block (block) ends with
// a JTRUE statement which conditionally jumps to the second block (thenBlock).
// The second block just contains a single assign statement. Both blocks then jump
// to the same destination (finalBlock).  Note that the first block may contain
// additional statements prior to the JTRUE statement.
//
// For example:
//
// ------------ BB03 [009..00D) -> BB05 (cond), preds={BB02} succs={BB04,BB05}
// STMT00004
//   *  JTRUE     void   $VN.Void
//   \--*  GE        int    $102
//      +--*  LCL_VAR   int    V02
//      \--*  CNS_INT   int    7 $46
//
// ------------ BB04 [00D..010), preds={BB03} succs={BB05}
// STMT00005
//   *  ASG       int    $VN.Void
// +--*  LCL_VAR   int    V00 arg0
// \--*  CNS_INT   int    5 $47
//
//
// This is optimised by conditionally executing the store and removing the conditional
// jumps. First the JTRUE is replaced with a NOP. The assignment is updated so that
// the source of the store is a SELECT node with the condition set to the inverse of
// the original JTRUE condition. If the condition passes the original assign happens,
// otherwise the existing source value is used.
//
// In the example above, local var 0 is set to 5 if the LT returns true, otherwise
// the existing value of local var 0 is used:
//
// ------------ BB03 [009..00D) -> BB05 (always), preds={BB02} succs={BB05}
// STMT00004
//   *  NOP       void
//
// STMT00005
//   *  ASG       int    $VN.Void
//   +--*  LCL_VAR   int    V00 arg0
//   \--*  SELECT    int
//      +--*  LT        int    $102
//      |  +--*  LCL_VAR   int    V02
//      |  \--*  CNS_INT   int    7 $46
//      +--*  CNS_INT   int    5 $47
//      \--*  LCL_VAR   int    V00
//
// ------------ BB04 [00D..010), preds={} succs={BB05}
//
//
// Example of simple if conversion with an else condition
//
// This is similar to the simple if conversion above, but with an else statement
// that assigns to the same variable as the then statement. For example:
// if (x < 7) { a = 5; } else { a = 9; }
//
// ------------ BB03 [009..00D) -> BB05 (cond), preds={BB02} succs={BB04,BB05}
// STMT00004
//   *  JTRUE     void   $VN.Void
//   \--*  GE        int    $102
//      +--*  LCL_VAR   int    V02
//      \--*  CNS_INT   int    7 $46
//
// ------------ BB04 [00D..010), preds={BB03} succs={BB06}
// STMT00005
//   *  ASG       int    $VN.Void
// +--*  LCL_VAR   int    V00 arg0
// \--*  CNS_INT   int    5 $47
//
// ------------ BB05 [00D..010), preds={BB03} succs={BB06}
// STMT00006
//   *  ASG       int    $VN.Void
// +--*  LCL_VAR   int    V00 arg0
// \--*  CNS_INT   int    9 $48
//
// Again this is squashed into a single block, with the SELECT node handling both cases.
//
// ------------ BB03 [009..00D) -> BB05 (always), preds={BB02} succs={BB05}
// STMT00004
//   *  NOP       void
//
// STMT00005
//   *  ASG       int    $VN.Void
//   +--*  LCL_VAR   int    V00 arg0
//   \--*  SELECT    int
//      +--*  LT        int    $102
//      |  +--*  LCL_VAR   int    V02
//      |  \--*  CNS_INT   int    7 $46
//      +--*  CNS_INT   int    5 $47
//      +--*  CNS_INT   int    9 $48
//
// STMT00006
//   *  NOP       void
//
// ------------ BB04 [00D..010), preds={} succs={BB06}
// ------------ BB05 [00D..010), preds={} succs={BB06}
//
// Alternatively, an if conversion with an else condition may use RETURNs.
// return (x < 7) ? 5 : 9;
//
// ------------ BB03 [009..00D) -> BB05 (cond), preds={BB02} succs={BB04,BB05}
// STMT00004
//   *  JTRUE     void   $VN.Void
//   \--*  GE        int    $102
//      +--*  LCL_VAR   int    V02
//      \--*  CNS_INT   int    7 $46
//
// ------------ BB04 [00D..010), preds={BB03} succs={BB06}
// STMT00005
//   *  RETURN    int    $VN.Void
// +--*  CNS_INT   int    5 $41
//
// ------------ BB05 [00D..010), preds={BB03} succs={BB06}
// STMT00006
//   *  RETURN    int    $VN.Void
// +--*  CNS_INT   int    9 $43
//
// becomes:
//
// ------------ BB03 [009..00D) -> BB05 (always), preds={BB02} succs={BB05}
// STMT00004
//   *  NOP       void
//
// STMT00005
//   *  RETURN    int    $VN.Void
//   \--*  SELECT    int
//      +--*  LT        int    $102
//      |  +--*  LCL_VAR   int    V02
//      |  \--*  CNS_INT   int    7 $46
//      +--*  CNS_INT   int    5 $41
//      +--*  CNS_INT   int    9 $43
//
// STMT00006
//   *  NOP       void
//
// ------------ BB04 [00D..010), preds={} succs={BB06}
// ------------ BB05 [00D..010), preds={} succs={BB06}
//
bool OptIfConversionDsc::optIfConvert()
{
    // Does the block end by branching via a JTRUE after a compare?
    if (m_startBlock->bbJumpKind != BBJ_COND || m_startBlock->NumSucc() != 2)
    {
        return false;
    }

    // Verify the test block ends with a condition that we can manipulate.
    GenTree* last = m_startBlock->lastStmt()->GetRootNode();
    noway_assert(last->OperIs(GT_JTRUE));
    m_cond = last->gtGetOp1();
    if (!m_cond->OperIsCompare())
    {
        return false;
    }

    // Look for valid flow of Then and Else blocks.
    IfConvertFindFlow();
    if (!m_flowFound)
    {
        return false;
    }

    // Check the Then and Else blocks have a single operation each.
    if (!IfConvertCheckStmts(m_startBlock->bbNext, &m_thenOperation))
    {
        return false;
    }
    assert(m_thenOperation.node->gtOper == GT_ASG || m_thenOperation.node->gtOper == GT_RETURN);
    if (m_doElseConversion)
    {
        if (!IfConvertCheckStmts(m_startBlock->bbJumpDest, &m_elseOperation))
        {
            return false;
        }

        // Both operations must be the same node type.
        if (m_thenOperation.node->gtOper != m_elseOperation.node->gtOper)
        {
            return false;
        }

        // Currently can only support Else Asg Blocks that have the same destination as the Then block.
        if (m_thenOperation.node->gtOper == GT_ASG)
        {
            unsigned lclNumThen = m_thenOperation.node->gtGetOp1()->AsLclVarCommon()->GetLclNum();
            unsigned lclNumElse = m_elseOperation.node->gtGetOp1()->AsLclVarCommon()->GetLclNum();
            if (lclNumThen != lclNumElse)
            {
                return false;
            }
        }
    }

#ifdef DEBUG
    if (m_comp->verbose)
    {
        JITDUMP("\nConditionally executing " FMT_BB, m_thenOperation.block->bbNum);
        if (m_doElseConversion)
        {
            JITDUMP(" and " FMT_BB, m_elseOperation.block->bbNum);
        }
        JITDUMP(" inside " FMT_BB "\n", m_startBlock->bbNum);
        IfConvertDump();
    }
#endif

    // Using SELECT nodes means that both Then and Else operations are fully evaluated.
    // Put a limit on the original source and destinations.
    if (!m_comp->compStressCompile(Compiler::STRESS_IF_CONVERSION_COST, 25))
    {
        int thenCost = 0;
        int elseCost = 0;

        if (m_mainOper == GT_ASG)
        {
            thenCost = m_thenOperation.node->gtGetOp2()->GetCostEx() +
                       (m_comp->gtIsLikelyRegVar(m_thenOperation.node->gtGetOp1()) ? 0 : 2);
            if (m_doElseConversion)
            {
                elseCost = m_elseOperation.node->gtGetOp2()->GetCostEx() +
                           (m_comp->gtIsLikelyRegVar(m_elseOperation.node->gtGetOp1()) ? 0 : 2);
            }
        }
        else
        {
            assert(m_mainOper == GT_RETURN);
            thenCost = m_thenOperation.node->gtGetOp1()->GetCostEx();
            if (m_doElseConversion)
            {
                elseCost = m_elseOperation.node->gtGetOp1()->GetCostEx();
            }
        }

#ifdef TARGET_XARCH
        // Currently the xarch backend does not handle SELECT (EQ/NE (arithmetic op that sets ZF) 0) ...
        // as efficiently as JTRUE (EQ/NE (arithmetic op that sets ZF) 0). The support is complicated
        // to add due to the destructive nature of xarch instructions.
        // The exception is for cases that can be transformed into TEST_EQ/TEST_NE.
        // TODO-CQ: Fix this.
        if (m_cond->OperIs(GT_EQ, GT_NE) && m_cond->gtGetOp2()->IsIntegralConst(0) &&
            !m_cond->gtGetOp1()->OperIs(GT_AND) &&
            (m_cond->gtGetOp1()->SupportsSettingZeroFlag() || IsHWIntrinsicCC(m_cond->gtGetOp1())))
        {
            JITDUMP("Skipping if-conversion where condition is EQ/NE 0 with operation that sets ZF");
            return false;
        }

        // However, in some cases bit tests can emit 'bt' when not going
        // through the GT_SELECT path.
        if (m_cond->OperIs(GT_EQ, GT_NE) && m_cond->gtGetOp1()->OperIs(GT_AND) &&
            m_cond->gtGetOp2()->IsIntegralConst(0))
        {
            // A bit test that can be transformed into 'bt' will look like
            // EQ/NE(AND(x, LSH(1, y)), 0)

            GenTree* andOp1 = m_cond->gtGetOp1()->gtGetOp1();
            GenTree* andOp2 = m_cond->gtGetOp1()->gtGetOp2();

            if (andOp2->OperIs(GT_LSH) && andOp2->gtGetOp1()->IsIntegralConst(1))
            {
                JITDUMP("Skipping if-conversion where condition is amenable to be transformed to BT");
                return false;
            }
        }
#endif

        // Cost to allow for "x = cond ? a + b : c + d".
        if (thenCost > 7 || elseCost > 7)
        {
            JITDUMP("Skipping if-conversion that will evaluate RHS unconditionally at costs %d,%d\n", thenCost,
                    elseCost);
            return false;
        }
    }

    if (!m_comp->compStressCompile(Compiler::STRESS_IF_CONVERSION_INNER_LOOPS, 25))
    {
        // Don't optimise the block if it is inside a loop
        // When inside a loop, branches are quicker than selects.
        // Detect via the block weight as that will be high when inside a loop.
        if (m_startBlock->getBBWeight(m_comp) > BB_UNITY_WEIGHT)
        {
            JITDUMP("Skipping if-conversion inside loop (via weight)\n");
            return false;
        }

        // We may be inside an unnatural loop, so do the expensive check.
        if (m_comp->optReachable(m_finalBlock, m_startBlock, nullptr))
        {
            JITDUMP("Skipping if-conversion inside loop (via FG walk)\n");
            return false;
        }
    }

    // Get the select node inputs.
    var_types selectType;
    GenTree*  selectTrueInput;
    GenTree*  selectFalseInput;
    if (m_mainOper == GT_ASG)
    {
        if (m_doElseConversion)
        {
            selectTrueInput  = m_elseOperation.node->gtGetOp2();
            selectFalseInput = m_thenOperation.node->gtGetOp2();
        }
        else
        {
            // Duplicate the destination of the Then assignment.
            assert(m_thenOperation.node->gtGetOp1()->IsLocal());
            selectTrueInput = m_comp->gtCloneExpr(m_thenOperation.node->gtGetOp1());
            selectTrueInput->gtFlags &= GTF_EMPTY;

            selectFalseInput = m_thenOperation.node->gtGetOp2();
        }

        // Pick the type as the type of the local, which should always be compatible even for implicit coercions.
        selectType = genActualType(m_thenOperation.node->gtGetOp1());
    }
    else
    {
        assert(m_mainOper == GT_RETURN);
        assert(m_doElseConversion);
        assert(m_thenOperation.node->TypeGet() == m_elseOperation.node->TypeGet());

        selectTrueInput  = m_elseOperation.node->gtGetOp1();
        selectFalseInput = m_thenOperation.node->gtGetOp1();
        selectType       = genActualType(m_thenOperation.node);
    }

    // Create a select node.
    GenTreeConditional* select =
        m_comp->gtNewConditionalNode(GT_SELECT, m_cond, selectTrueInput, selectFalseInput, selectType);
    m_thenOperation.node->AsOp()->gtFlags |= (select->gtFlags & GTF_ALL_EFFECT);

    // Use the select as the source of the Then operation.
    if (m_mainOper == GT_ASG)
    {
        m_thenOperation.node->AsOp()->gtOp2 = select;
    }
    else
    {
        m_thenOperation.node->AsOp()->gtOp1 = select;
    }
    m_comp->gtSetEvalOrder(m_thenOperation.node);
    m_comp->fgSetStmtSeq(m_thenOperation.stmt);

    // Remove statements.
    last->ReplaceWith(m_comp->gtNewNothingNode(), m_comp);
    m_comp->gtSetEvalOrder(last);
    m_comp->fgSetStmtSeq(m_startBlock->lastStmt());
    if (m_doElseConversion)
    {
        m_elseOperation.node->ReplaceWith(m_comp->gtNewNothingNode(), m_comp);
        m_comp->gtSetEvalOrder(m_elseOperation.node);
        m_comp->fgSetStmtSeq(m_elseOperation.stmt);
    }

    // Merge all the blocks.
    IfConvertJoinStmts(m_thenOperation.block);
    if (m_doElseConversion)
    {
        IfConvertJoinStmts(m_elseOperation.block);
    }

    // Update the flow from the original block.
    m_comp->fgRemoveAllRefPreds(m_startBlock->bbNext, m_startBlock);
    m_startBlock->bbJumpKind = BBJ_ALWAYS;

#ifdef DEBUG
    if (m_comp->verbose)
    {
        JITDUMP("\nAfter if conversion\n");
        IfConvertDump();
    }
#endif

    return true;
}

//-----------------------------------------------------------------------------
// optIfConversion: If conversion
//
// Returns:
//   suitable phase status
//
PhaseStatus Compiler::optIfConversion()
{
    if (!opts.OptimizationEnabled())
    {
        return PhaseStatus::MODIFIED_NOTHING;
    }

#if defined(DEBUG)
    if (JitConfig.JitDoIfConversion() == 0)
    {
        return PhaseStatus::MODIFIED_NOTHING;
    }
#endif

    bool madeChanges = false;

    // This phase does not repect SSA: assignments are deleted/moved.
    assert(!fgDomsComputed);
    optReachableBitVecTraits = nullptr;

#if defined(TARGET_ARM64) || defined(TARGET_XARCH)
    // Reverse iterate through the blocks.
    BasicBlock* block = fgLastBB;
    while (block != nullptr)
    {
        OptIfConversionDsc optIfConversionDsc(this, block);
        madeChanges |= optIfConversionDsc.optIfConvert();
        block = block->bbPrev;
    }
#endif

    return madeChanges ? PhaseStatus::MODIFIED_EVERYTHING : PhaseStatus::MODIFIED_NOTHING;
}
