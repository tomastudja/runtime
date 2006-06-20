

#if (ANALYZE_DEV_USE_SPECIFIC_OPS)
OPDEF(CEE_LDIND_I1, "ldind.i1", PopI, PushI, InlineNone, X, 1, 0xFF, 0x46, NEXT)
OPDEF(CEE_LDIND_U1, "ldind.u1", PopI, PushI, InlineNone, X, 1, 0xFF, 0x47, NEXT)
OPDEF(CEE_LDIND_I2, "ldind.i2", PopI, PushI, InlineNone, X, 1, 0xFF, 0x48, NEXT)
OPDEF(CEE_LDIND_U2, "ldind.u2", PopI, PushI, InlineNone, X, 1, 0xFF, 0x49, NEXT)
OPDEF(CEE_LDIND_I4, "ldind.i4", PopI, PushI, InlineNone, X, 1, 0xFF, 0x4A, NEXT)
OPDEF(CEE_LDIND_U4, "ldind.u4", PopI, PushI, InlineNone, X, 1, 0xFF, 0x4B, NEXT)
OPDEF(CEE_LDIND_I8, "ldind.i8", PopI, PushI8, InlineNone, X, 1, 0xFF, 0x4C, NEXT)
OPDEF(CEE_LDIND_I, "ldind.i", PopI, PushI, InlineNone, X, 1, 0xFF, 0x4D, NEXT)
OPDEF(CEE_LDIND_R4, "ldind.r4", PopI, PushR4, InlineNone, X, 1, 0xFF, 0x4E, NEXT)
OPDEF(CEE_LDIND_R8, "ldind.r8", PopI, PushR8, InlineNone, X, 1, 0xFF, 0x4F, NEXT)
OPDEF(CEE_LDIND_REF, "ldind.ref", PopI, PushRef, InlineNone, X, 1, 0xFF, 0x50, NEXT)
OPDEF(CEE_LDSTR, "ldstr", Pop0, PushRef, InlineString, X, 1, 0xFF, 0x72, NEXT)
OPDEF(CEE_STOBJ, "stobj", PopI+Pop1, Push0, InlineType, X, 1, 0xFF, 0x81, NEXT)
#endif

#if 0
OPDEF(CEE_CPOBJ, "cpobj", PopI+PopI, Push0, InlineType, X, 1, 0xFF, 0x70, NEXT)
OPDEF(CEE_LDOBJ, "ldobj", PopI, Push1, InlineType, X, 1, 0xFF, 0x71, NEXT)
OPDEF(CEE_CASTCLASS, "castclass", PopRef, PushRef, InlineType, X, 1, 0xFF, 0x74, NEXT)
OPDEF(CEE_UNBOX, "unbox", PopRef, PushI, InlineType, X, 1, 0xFF, 0x79, NEXT)
OPDEF(CEE_UNBOX_ANY, "unbox.any", PopRef, Push1, InlineType, X, 1, 0xFF, 0xA5, NEXT)

OPDEF(CEE_MONO_OBJADDR, "mono_objaddr", Pop1, PushI, InlineNone, X, 2, 0xF0, 0x01, NEXT)
OPDEF(CEE_MONO_LDPTR, "mono_ldptr", Pop0, PushI, InlineI, X, 2, 0xF0, 0x02, NEXT)
OPDEF(CEE_MONO_VTADDR, "mono_vtaddr", Pop1, PushI, InlineNone, X, 2, 0xF0, 0x03, NEXT)
OPDEF(CEE_MONO_LDNATIVEOBJ, "mono_ldnativeobj", PopI, Push1, InlineType, X, 2, 0xF0, 0x06, RETURN)
OPDEF(CEE_MONO_CISINST, "mono_cisinst", PopRef, Push1, InlineType, X, 2, 0xF0, 0x07, NEXT)
OPDEF(CEE_MONO_CCASTCLASS, "mono_ccastclass", PopRef, Push1, InlineType, X, 2, 0xF0, 0x08, NEXT)
#endif

#if (SSAPRE_SPECIFIC_OPS)
OPDEF(CEE_LDARG, "ldarg", Pop0, Push1, InlineVar, X, 2, 0xFE, 0x09, NEXT)
OPDEF(CEE_LDARGA, "ldarga", Pop0, PushI, InlineVar, X, 2, 0xFE, 0x0A, NEXT)
OPDEF(CEE_LDLOC, "ldloc", Pop0, Push1, InlineVar, X, 2, 0xFE, 0x0C, NEXT)
OPDEF(CEE_LDLOCA, "ldloca", Pop0, PushI, InlineVar, X, 2, 0xFE, 0x0D, NEXT)
#endif


#if (TREEMOVE_SPECIFIC_OPS)
OPDEF(CEE_LDFLD, "ldfld", PopRef, Push1, InlineField, X, 1, 0xFF, 0x7B, NEXT)
OPDEF(CEE_LDFLDA, "ldflda", PopRef, PushI, InlineField, X, 1, 0xFF, 0x7C, NEXT)
OPDEF(CEE_STFLD, "stfld", PopRef+Pop1, Push0, InlineField, X, 1, 0xFF, 0x7D, NEXT)
OPDEF(CEE_LDSFLD, "ldsfld", Pop0, Push1, InlineField, X, 1, 0xFF, 0x7E, NEXT)
OPDEF(CEE_LDSFLDA, "ldsflda", Pop0, PushI, InlineField, X, 1, 0xFF, 0x7F, NEXT)
OPDEF(CEE_STSFLD, "stsfld", Pop1, Push0, InlineField, X, 1, 0xFF, 0x80, NEXT)
#endif


#if (TREEMOVE_SPECIFIC_OPS)
OPDEF(CEE_LDLEN, "ldlen", PopRef, PushI, InlineNone, X, 1, 0xFF, 0x8E, NEXT)
OPDEF(CEE_LDELEMA, "ldelema", PopRef+PopI, PushI, InlineType, X, 1, 0xFF, 0x8F, NEXT)
OPDEF(CEE_LDELEM_I1, "ldelem.i1", PopRef+PopI, PushI, InlineNone, X, 1, 0xFF, 0x90, NEXT)
OPDEF(CEE_LDELEM_U1, "ldelem.u1", PopRef+PopI, PushI, InlineNone, X, 1, 0xFF, 0x91, NEXT)
OPDEF(CEE_LDELEM_I2, "ldelem.i2", PopRef+PopI, PushI, InlineNone, X, 1, 0xFF, 0x92, NEXT)
OPDEF(CEE_LDELEM_U2, "ldelem.u2", PopRef+PopI, PushI, InlineNone, X, 1, 0xFF, 0x93, NEXT)
OPDEF(CEE_LDELEM_I4, "ldelem.i4", PopRef+PopI, PushI, InlineNone, X, 1, 0xFF, 0x94, NEXT)
OPDEF(CEE_LDELEM_U4, "ldelem.u4", PopRef+PopI, PushI, InlineNone, X, 1, 0xFF, 0x95, NEXT)
OPDEF(CEE_LDELEM_I8, "ldelem.i8", PopRef+PopI, PushI8, InlineNone, X, 1, 0xFF, 0x96, NEXT)
OPDEF(CEE_LDELEM_I, "ldelem.i", PopRef+PopI, PushI, InlineNone, X, 1, 0xFF, 0x97, NEXT)
OPDEF(CEE_LDELEM_R4, "ldelem.r4", PopRef+PopI, PushR4, InlineNone, X, 1, 0xFF, 0x98, NEXT)
OPDEF(CEE_LDELEM_R8, "ldelem.r8", PopRef+PopI, PushR8, InlineNone, X, 1, 0xFF, 0x99, NEXT)
OPDEF(CEE_LDELEM_REF, "ldelem.ref", PopRef+PopI, PushRef, InlineNone, X, 1, 0xFF, 0x9A, NEXT)
OPDEF(CEE_LDELEM_ANY, "ldelem.any", PopRef+PopI, Push1, InlineType, X, 1, 0xFF, 0xA3, NEXT)
#endif

OPDEF(CEE_ADD, "add", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0x58, NEXT)
OPDEF(CEE_SUB, "sub", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0x59, NEXT)
OPDEF(CEE_MUL, "mul", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0x5A, NEXT)
OPDEF(CEE_DIV, "div", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0x5B, NEXT)
OPDEF(CEE_DIV_UN, "div.un", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0x5C, NEXT)
OPDEF(CEE_REM, "rem", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0x5D, NEXT)
OPDEF(CEE_REM_UN, "rem.un", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0x5E, NEXT)
OPDEF(CEE_AND, "and", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0x5F, NEXT)
OPDEF(CEE_OR, "or", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0x60, NEXT)
OPDEF(CEE_XOR, "xor", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0x61, NEXT)
OPDEF(CEE_SHL, "shl", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0x62, NEXT)
OPDEF(CEE_SHR, "shr", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0x63, NEXT)
OPDEF(CEE_SHR_UN, "shr.un", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0x64, NEXT)
OPDEF(CEE_NEG, "neg", Pop1, Push1, InlineNone, X, 1, 0xFF, 0x65, NEXT)
OPDEF(CEE_NOT, "not", Pop1, Push1, InlineNone, X, 1, 0xFF, 0x66, NEXT)
OPDEF(CEE_CONV_I1, "conv.i1", Pop1, PushI, InlineNone, X, 1, 0xFF, 0x67, NEXT)
OPDEF(CEE_CONV_I2, "conv.i2", Pop1, PushI, InlineNone, X, 1, 0xFF, 0x68, NEXT)
OPDEF(CEE_CONV_I4, "conv.i4", Pop1, PushI, InlineNone, X, 1, 0xFF, 0x69, NEXT)
OPDEF(CEE_CONV_I8, "conv.i8", Pop1, PushI8, InlineNone, X, 1, 0xFF, 0x6A, NEXT)
OPDEF(CEE_CONV_R4, "conv.r4", Pop1, PushR4, InlineNone, X, 1, 0xFF, 0x6B, NEXT)
OPDEF(CEE_CONV_R8, "conv.r8", Pop1, PushR8, InlineNone, X, 1, 0xFF, 0x6C, NEXT)
OPDEF(CEE_CONV_U4, "conv.u4", Pop1, PushI, InlineNone, X, 1, 0xFF, 0x6D, NEXT)
OPDEF(CEE_CONV_U8, "conv.u8", Pop1, PushI8, InlineNone, X, 1, 0xFF, 0x6E, NEXT)
OPDEF(CEE_CONV_R_UN, "conv.r.un", Pop1, PushR8, InlineNone, X, 1, 0xFF, 0x76, NEXT)

OPDEF(CEE_CONV_U2, "conv.u2", Pop1, PushI, InlineNone, X, 1, 0xFF, 0xD1, NEXT)
OPDEF(CEE_CONV_U1, "conv.u1", Pop1, PushI, InlineNone, X, 1, 0xFF, 0xD2, NEXT)
OPDEF(CEE_CONV_I, "conv.i", Pop1, PushI, InlineNone, X, 1, 0xFF, 0xD3, NEXT)
OPDEF(CEE_CONV_U, "conv.u", Pop1, PushI, InlineNone, X, 1, 0xFF, 0xE0, NEXT)


#if 0
OPDEF(CEE_CONV_OVF_I1_UN, "conv.ovf.i1.un", Pop1, PushI, InlineNone, X, 1, 0xFF, 0x82, NEXT)
OPDEF(CEE_CONV_OVF_I2_UN, "conv.ovf.i2.un", Pop1, PushI, InlineNone, X, 1, 0xFF, 0x83, NEXT)
OPDEF(CEE_CONV_OVF_I4_UN, "conv.ovf.i4.un", Pop1, PushI, InlineNone, X, 1, 0xFF, 0x84, NEXT)
OPDEF(CEE_CONV_OVF_I8_UN, "conv.ovf.i8.un", Pop1, PushI8, InlineNone, X, 1, 0xFF, 0x85, NEXT)
OPDEF(CEE_CONV_OVF_U1_UN, "conv.ovf.u1.un", Pop1, PushI, InlineNone, X, 1, 0xFF, 0x86, NEXT)
OPDEF(CEE_CONV_OVF_U2_UN, "conv.ovf.u2.un", Pop1, PushI, InlineNone, X, 1, 0xFF, 0x87, NEXT)
OPDEF(CEE_CONV_OVF_U4_UN, "conv.ovf.u4.un", Pop1, PushI, InlineNone, X, 1, 0xFF, 0x88, NEXT)
OPDEF(CEE_CONV_OVF_U8_UN, "conv.ovf.u8.un", Pop1, PushI8, InlineNone, X, 1, 0xFF, 0x89, NEXT)
OPDEF(CEE_CONV_OVF_I_UN, "conv.ovf.i.un", Pop1, PushI, InlineNone, X, 1, 0xFF, 0x8A, NEXT)
OPDEF(CEE_CONV_OVF_U_UN, "conv.ovf.u.un", Pop1, PushI, InlineNone, X, 1, 0xFF, 0x8B, NEXT)

OPDEF(CEE_CONV_OVF_I1, "conv.ovf.i1", Pop1, PushI, InlineNone, X, 1, 0xFF, 0xB3, NEXT)
OPDEF(CEE_CONV_OVF_U1, "conv.ovf.u1", Pop1, PushI, InlineNone, X, 1, 0xFF, 0xB4, NEXT)
OPDEF(CEE_CONV_OVF_I2, "conv.ovf.i2", Pop1, PushI, InlineNone, X, 1, 0xFF, 0xB5, NEXT)
OPDEF(CEE_CONV_OVF_U2, "conv.ovf.u2", Pop1, PushI, InlineNone, X, 1, 0xFF, 0xB6, NEXT)
OPDEF(CEE_CONV_OVF_I4, "conv.ovf.i4", Pop1, PushI, InlineNone, X, 1, 0xFF, 0xB7, NEXT)
OPDEF(CEE_CONV_OVF_U4, "conv.ovf.u4", Pop1, PushI, InlineNone, X, 1, 0xFF, 0xB8, NEXT)
OPDEF(CEE_CONV_OVF_I8, "conv.ovf.i8", Pop1, PushI8, InlineNone, X, 1, 0xFF, 0xB9, NEXT)
OPDEF(CEE_CONV_OVF_U8, "conv.ovf.u8", Pop1, PushI8, InlineNone, X, 1, 0xFF, 0xBA, NEXT)

OPDEF(CEE_CONV_OVF_I, "conv.ovf.i", Pop1, PushI, InlineNone, X, 1, 0xFF, 0xD4, NEXT)
OPDEF(CEE_CONV_OVF_U, "conv.ovf.u", Pop1, PushI, InlineNone, X, 1, 0xFF, 0xD5, NEXT)
OPDEF(CEE_ADD_OVF, "add.ovf", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0xD6, NEXT)
OPDEF(CEE_ADD_OVF_UN, "add.ovf.un", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0xD7, NEXT)
OPDEF(CEE_MUL_OVF, "mul.ovf", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0xD8, NEXT)
OPDEF(CEE_MUL_OVF_UN, "mul.ovf.un", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0xD9, NEXT)
OPDEF(CEE_SUB_OVF, "sub.ovf", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0xDA, NEXT)
OPDEF(CEE_SUB_OVF_UN, "sub.ovf.un", Pop1+Pop1, Push1, InlineNone, X, 1, 0xFF, 0xDB, NEXT)
#endif
