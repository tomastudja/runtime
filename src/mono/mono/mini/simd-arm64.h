/* Remarks:
 * - This table is used to drive code generation on operations that are defined by the tuple (ins->opcode, ins->inst_c0).
 * - Operand config specifies the order of operands that are to be supplied to the function or macro. These are
 *   variations of: W (width of the vector register), T (element type), D (dest reg number), S (source reg number),
 *   I (immediate value). If _REV is specifed, the order of source registers is reversed. Note that not all 
 *   options are supported. To specify more options, add the respective macros to the files that include this
 *   (e.g. mini-arm64.c).
 * - To specify that a particular operation is not supported for some data type, use  _UNDEF.
 */

/* 64-bit vectors */
/*        Width   Opcode          Function              Operand config      I8                I16               I32               I64               F32               F64         */
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
SIMD_OP  (64,  OP_XCOMPARE,    CMP_EQ,               WTDSS,              arm_neon_cmeq,    arm_neon_cmeq,    arm_neon_cmeq,    _UNDEF,           _UNDEF,           _UNDEF)
SIMD_OP  (64,  OP_XCOMPARE,    CMP_GT,               WTDSS,              arm_neon_cmgt,    arm_neon_cmgt,    arm_neon_cmgt,    _UNDEF,           _UNDEF,           _UNDEF)
SIMD_OP  (64,  OP_XCOMPARE,    CMP_GT_UN,            WTDSS,              arm_neon_cmhi,    arm_neon_cmhi,    arm_neon_cmhi,    _UNDEF,           _UNDEF,           _UNDEF)
SIMD_OP  (64,  OP_XCOMPARE,    CMP_GE,               WTDSS,              arm_neon_cmge,    arm_neon_cmge,    arm_neon_cmge,    _UNDEF,           _UNDEF,           _UNDEF)
SIMD_OP  (64,  OP_XCOMPARE,    CMP_GE_UN,            WTDSS,              arm_neon_cmhs,    arm_neon_cmhs,    arm_neon_cmhs,    _UNDEF,           _UNDEF,           _UNDEF)
SIMD_OP  (64,  OP_XCOMPARE,    CMP_LT,               WTDSS_REV,          arm_neon_cmgt,    arm_neon_cmgt,    arm_neon_cmgt,    _UNDEF,           _UNDEF,           _UNDEF)
SIMD_OP  (64,  OP_XCOMPARE,    CMP_LT_UN,            WTDSS_REV,          arm_neon_cmhi,    arm_neon_cmhi,    arm_neon_cmhi,    _UNDEF,           _UNDEF,           _UNDEF)
SIMD_OP  (64,  OP_XCOMPARE,    CMP_LE,               WTDSS_REV,          arm_neon_cmge,    arm_neon_cmge,    arm_neon_cmge,    _UNDEF,           _UNDEF,           _UNDEF)
SIMD_OP  (64,  OP_XCOMPARE,    CMP_LE_UN,            WTDSS_REV,          arm_neon_cmhs,    arm_neon_cmhs,    arm_neon_cmhs,    _UNDEF,           _UNDEF,           _UNDEF)

SIMD_OP  (64,  OP_XCOMPARE_FP, CMP_EQ,               WTDSS,               _UNDEF,          _UNDEF,           _UNDEF,           _UNDEF,           arm_neon_fcmeq,   _UNDEF)
SIMD_OP  (64,  OP_XCOMPARE_FP, CMP_GT,               WTDSS,               _UNDEF,          _UNDEF,           _UNDEF,           _UNDEF,           arm_neon_fcmgt,   _UNDEF)
SIMD_OP  (64,  OP_XCOMPARE_FP, CMP_GE,               WTDSS,               _UNDEF,          _UNDEF,           _UNDEF,           _UNDEF,           arm_neon_fcmge,   _UNDEF)
SIMD_OP  (64,  OP_XCOMPARE_FP, CMP_LT,               WTDSS_REV,           _UNDEF,          _UNDEF,           _UNDEF,           _UNDEF,           arm_neon_fcmgt,   _UNDEF)
SIMD_OP  (64,  OP_XCOMPARE_FP, CMP_LE,               WTDSS_REV,           _UNDEF,          _UNDEF,           _UNDEF,           _UNDEF,           arm_neon_fcmge,   _UNDEF)

SIMD_OP  (64,  OP_XBINOP,      OP_IADD,              WTDSS,              arm_neon_add,     arm_neon_add,     arm_neon_add,     _UNDEF,           _UNDEF,           _UNDEF)  
SIMD_OP  (64,  OP_XBINOP,      OP_FADD,              WTDSS,              _UNDEF,           _UNDEF,           _UNDEF,           _UNDEF,           arm_neon_fadd,    _UNDEF)

/* 128-bit vectors */
/*         Width  Opcode          Function              Operand config      I8                I16               I32               I64               F32               F64         */
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
SIMD_OP  (128, OP_XCOMPARE,    CMP_EQ,               WTDSS,              arm_neon_cmeq,    arm_neon_cmeq,    arm_neon_cmeq,    arm_neon_cmeq,    _UNDEF,           _UNDEF)
SIMD_OP  (128, OP_XCOMPARE,    CMP_GT,               WTDSS,              arm_neon_cmgt,    arm_neon_cmgt,    arm_neon_cmgt,    arm_neon_cmgt,    _UNDEF,           _UNDEF)
SIMD_OP  (128, OP_XCOMPARE,    CMP_GT_UN,            WTDSS,              arm_neon_cmhi,    arm_neon_cmhi,    arm_neon_cmhi,    arm_neon_cmhi,    _UNDEF,           _UNDEF)
SIMD_OP  (128, OP_XCOMPARE,    CMP_GE,               WTDSS,              arm_neon_cmge,    arm_neon_cmge,    arm_neon_cmge,    arm_neon_cmge,    _UNDEF,           _UNDEF)
SIMD_OP  (128, OP_XCOMPARE,    CMP_GE_UN,            WTDSS,              arm_neon_cmhs,    arm_neon_cmhs,    arm_neon_cmhs,    arm_neon_cmhs,    _UNDEF,           _UNDEF)
SIMD_OP  (128, OP_XCOMPARE,    CMP_LT,               WTDSS_REV,          arm_neon_cmgt,    arm_neon_cmgt,    arm_neon_cmgt,    arm_neon_cmgt,    _UNDEF,           _UNDEF)
SIMD_OP  (128, OP_XCOMPARE,    CMP_LT_UN,            WTDSS_REV,          arm_neon_cmhi,    arm_neon_cmhi,    arm_neon_cmhi,    arm_neon_cmhi,    _UNDEF,           _UNDEF)
SIMD_OP  (128, OP_XCOMPARE,    CMP_LE,               WTDSS_REV,          arm_neon_cmge,    arm_neon_cmge,    arm_neon_cmge,    arm_neon_cmge,    _UNDEF,           _UNDEF)
SIMD_OP  (128, OP_XCOMPARE,    CMP_LE_UN,            WTDSS_REV,          arm_neon_cmhs,    arm_neon_cmhs,    arm_neon_cmhs,    arm_neon_cmhs,    _UNDEF,           _UNDEF)

SIMD_OP  (128, OP_XCOMPARE_FP, CMP_EQ,               WTDSS,              _UNDEF,           _UNDEF,           _UNDEF,           _UNDEF,           arm_neon_fcmeq,   arm_neon_fcmeq)
SIMD_OP  (128, OP_XCOMPARE_FP, CMP_GT,               WTDSS,              _UNDEF,           _UNDEF,           _UNDEF,           _UNDEF,           arm_neon_fcmgt,   arm_neon_fcmgt)
SIMD_OP  (128, OP_XCOMPARE_FP, CMP_GE,               WTDSS,              _UNDEF,           _UNDEF,           _UNDEF,           _UNDEF,           arm_neon_fcmge,   arm_neon_fcmge)
SIMD_OP  (128, OP_XCOMPARE_FP, CMP_LT,               WTDSS_REV,          _UNDEF,           _UNDEF,           _UNDEF,           _UNDEF,           arm_neon_fcmgt,   arm_neon_fcmgt)
SIMD_OP  (128, OP_XCOMPARE_FP, CMP_LE,               WTDSS_REV,          _UNDEF,           _UNDEF,           _UNDEF,           _UNDEF,           arm_neon_fcmge,   arm_neon_fcmge)

SIMD_OP  (128, OP_XBINOP,      OP_IADD,              WTDSS,              arm_neon_add,     arm_neon_add,     arm_neon_add,    arm_neon_add,      _UNDEF,           _UNDEF)
SIMD_OP  (128, OP_XBINOP,      OP_FADD,              WTDSS,              _UNDEF,           _UNDEF,           _UNDEF,          _UNDEF,            arm_neon_fadd,    arm_neon_fadd)
SIMD_OP  (128, OP_XBINOP,      OP_ISUB,              WTDSS,              arm_neon_sub,     arm_neon_sub,     arm_neon_sub,    arm_neon_sub,      _UNDEF,           _UNDEF)
SIMD_OP  (128, OP_XBINOP,      OP_FSUB,              WTDSS,              _UNDEF,           _UNDEF,           _UNDEF,          _UNDEF,            arm_neon_fsub,    arm_neon_fsub)
SIMD_OP  (128, OP_XBINOP_FORCEINT,    XBINOP_FORCEINT_AND,    WDSS,      arm_neon_and,     arm_neon_and,     arm_neon_and,    arm_neon_and,      arm_neon_and,     arm_neon_and)
SIMD_OP  (128, OP_XBINOP_FORCEINT,    XBINOP_FORCEINT_OR,     WDSS,      arm_neon_orr,     arm_neon_orr,     arm_neon_orr,    arm_neon_orr,      arm_neon_orr,     arm_neon_orr)
SIMD_OP  (128, OP_XBINOP_FORCEINT,    XBINOP_FORCEINT_XOR,    WDSS,      arm_neon_eor,     arm_neon_eor,     arm_neon_eor,    arm_neon_eor,      arm_neon_eor,     arm_neon_eor)
