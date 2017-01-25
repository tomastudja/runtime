OPTFLAG(PEEPHOLE ,0, "peephole",   "Peephole postpass")
OPTFLAG(BRANCH   ,1, "branch",     "Branch optimizations")
OPTFLAG(INLINE   ,2, "inline",     "Inline method calls")
OPTFLAG(CFOLD    ,3, "cfold",      "Constant folding")
OPTFLAG(CONSPROP ,4, "consprop",   "Constant propagation")
OPTFLAG(COPYPROP ,5, "copyprop",   "Copy propagation")
OPTFLAG(DEADCE   ,6, "deadce",     "Dead code elimination")
OPTFLAG(LINEARS  ,7, "linears",    "Linear scan global reg allocation")
OPTFLAG(CMOV     ,8, "cmov",       "Conditional moves")
OPTFLAG(SHARED   ,9, "shared",     "Emit per-domain code")
OPTFLAG(SCHED    ,10, "sched",      "Instruction scheduling")
OPTFLAG(INTRINS  ,11, "intrins",    "Intrinsic method implementations")
OPTFLAG(TAILC    ,12, "tailc",      "Tail recursion and tail calls")
OPTFLAG(LOOP     ,13, "loop",       "Loop related optimizations")
OPTFLAG(FCMOV    ,14, "fcmov",      "Fast x86 FP compares")
OPTFLAG(LEAF     ,15, "leaf",       "Leaf procedures optimizations")
OPTFLAG(AOT      ,16, "aot",        "Usage of Ahead Of Time compiled code")
OPTFLAG(PRECOMP  ,17, "precomp",    "Precompile all methods before executing Main")
OPTFLAG(ABCREM   ,18, "abcrem",     "Array bound checks removal")
OPTFLAG(SSAPRE   ,19, "ssapre",     "SSA based Partial Redundancy Elimination (obsolete)")
OPTFLAG(EXCEPTION,20, "exception",  "Optimize exception catch blocks")
OPTFLAG(SSA      ,21, "ssa",        "Use plain SSA form")
OPTFLAG(SSE2     ,23, "sse2",       "SSE2 instructions on x86")
/* The id has to be smaller than gshared's, the parser code depends on this */
OPTFLAG(GSHAREDVT,24, "gsharedvt",	"Generic sharing for valuetypes")
OPTFLAG (GSHARED, 25, "gshared", "Generic Sharing")
OPTFLAG(SIMD	 ,26, "simd",	    "Simd intrinsics")
OPTFLAG(UNSAFE	 ,27, "unsafe",	    "Remove bound checks and perform other dangerous changes")
OPTFLAG(ALIAS_ANALYSIS	 ,28, "alias-analysis",      "Alias analysis of locals")
OPTFLAG(FLOAT32  ,29, "float32",    "Use 32 bit float arithmetic if possible")

