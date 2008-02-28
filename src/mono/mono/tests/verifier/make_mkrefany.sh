#! /bin/sh

TEST_NAME=$1
TEST_VALIDITY=$2
TEST_TYPE1=$3
TEST_TYPE2=$4
TEST_OP=$5


TEST_FILE=`echo ${TEST_VALIDITY}_${TEST_NAME} | sed -e 's/ /_/g' -e 's/\./_/g' -e 's/&/mp/g' -e 's/\[/_/g' -e 's/\]/_/g'`_generated.il
echo $TEST_FILE
sed -e "s/TYPE1/${TEST_TYPE1}/g" -e "s/TYPE2/${TEST_TYPE2}/g" -e "s/VALIDITY/${TEST_VALIDITY}/g" -e "s/CODE/${TEST_OP}/g" > $TEST_FILE <<//EOF

.assembly extern mscorlib
{
  .ver 2:0:0:0
  .publickeytoken = (B7 7A 5C 56 19 34 E0 89 ) // .z\V.4..
}

.assembly 'mkrefany_test'
{
  .hash algorithm 0x00008004
  .ver  0:0:0:0
}

.module mkrefany.exe


.class Class extends [mscorlib]System.Object
{
    .field public int32 valid
}

.class public Template\`1<T>
  	extends [mscorlib]System.Object
{
}

.class sealed public StructTemplate\`1<T>
  	extends [mscorlib]System.ValueType
{
	.field public !0 t
}

.class sealed public StructTemplate2\`1<T>
  	extends [mscorlib]System.ValueType
{
	.field public !0 t
}


.class public auto ansi sealed MyStruct
  	extends [mscorlib]System.ValueType
{
	.field public int32 foo
}


.class public auto ansi sealed MyStruct2
  	extends [mscorlib]System.ValueType
{
	.field public int32 foo
}

.method public static int32 Main ()
{
	.entrypoint
	.maxstack 8
	.locals init (TYPE1 V_0)

	ldloca 0

	CODE
	mkrefany TYPE2
	pop

	ldc.i4.0
	ret 
}

//EOF
