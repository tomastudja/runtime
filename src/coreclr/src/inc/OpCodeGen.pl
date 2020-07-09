# Licensed to the .NET Foundation under one or more agreements.
# The .NET Foundation licenses this file to you under the MIT license.
#
# OpCodeGen.pl
#
# PERL script used to generate the numbering of the reference opcodes
#
#use strict 'vars';
#use strict 'subs';
#use strict 'refs';


my $ret = 0;
my %opcodeEnum;
my %oneByte;
my %twoByte;
my %controlFlow;
my @singleByteArg;
my %stackbehav;
my %opcodetype;
my %operandtype;
my %opcodes;
my $popstate;
my $pushstate;

$ctrlflowcount = 0;

$count = 0;

my @lowercaseAlphabet = ('a'..'z','0'..'9');
my %upcaseAlphabet = ();

foreach $letter (@lowercaseAlphabet) {
    $j = $letter;
    $j=~tr/a-z/A-Z/;
    $upcaseAlphabet{$letter}=$j;
}

$license = "// Licensed to the .NET Foundation under one or more agreements.\n";
$license .= "// The .NET Foundation licenses this file to you under the MIT license.\n";

$startHeaderComment = "/*============================================================\n**\n";
$endHeaderComment = "**\n** THIS FILE IS AUTOMATICALLY GENERATED. DO NOT EDIT BY HAND!\n";
$endHeaderComment .= "** See \$(RepoRoot)\\src\\inc\\OpCodeGen.pl for more information.**\n";
$endHeaderComment .= "==============================================================*/\n\n";

$usingAndRefEmitNmsp = "namespace System.Reflection.Emit\n{\n\n";
$obsoleteAttr = "        [Obsolete(\"This API has been deprecated. https://go.microsoft.com/fwlink/?linkid=14202\")]\n";

# Open source file and target files

open (OPCODE, "opcode.def") or die "Couldn't open opcode.def: $!\n";
open (OUTPUT, ">OpCodes.cs") or die "Couldn't open OpCodes.cs: $!\n";
open (FCOUTPUT, ">FlowControl.cs") or die "Couldn't open FlowControl.cs: $!\n";
open (SOUTPUT, ">StackBehaviour.cs") or die "Couldn't open StackBehaviour.cs: $!\n";
open (OCOUTPUT, ">OpCodeType.cs") or die "Couldn't open OpCodeType.cs: $!\n";
open (OPOUTPUT, ">OperandType.cs") or die "Couldn't open OperandType.cs: $!\n";

print OUTPUT $license;
print OUTPUT $startHeaderComment;
print OUTPUT "** Class: OpCodes\n";
print OUTPUT "**\n";
print OUTPUT "** Purpose: Exposes all of the IL instructions supported by the runtime.\n";
print OUTPUT $endHeaderComment;

print OUTPUT $usingAndRefEmitNmsp;

print FCOUTPUT $license;
print FCOUTPUT $startHeaderComment;
print FCOUTPUT "** Enumeration: FlowControl\n";
print FCOUTPUT "**\n";
print FCOUTPUT "** Purpose: Exposes FlowControl Attribute of IL.\n";
print FCOUTPUT $endHeaderComment;

print FCOUTPUT $usingAndRefEmitNmsp;
print FCOUTPUT "    public enum FlowControl\n    {\n";

print SOUTPUT $license;
print SOUTPUT $startHeaderComment;
print SOUTPUT "** Enumeration: StackBehaviour\n";
print SOUTPUT "**\n";
print SOUTPUT "** Purpose: Exposes StackBehaviour Attribute of IL.\n";
print SOUTPUT $endHeaderComment;

print SOUTPUT $usingAndRefEmitNmsp;
print SOUTPUT "    public enum StackBehaviour\n    {\n";

print OCOUTPUT $license;
print OCOUTPUT $startHeaderComment;
print OCOUTPUT "** Enumeration: OpCodeType\n";
print OCOUTPUT "**\n";
print OCOUTPUT "** Purpose: Exposes OpCodeType Attribute of IL.\n";
print OCOUTPUT $endHeaderComment;

print OCOUTPUT $usingAndRefEmitNmsp;
print OCOUTPUT "    public enum OpCodeType\n    {\n";

print OPOUTPUT $license;
print OPOUTPUT $startHeaderComment;
print OPOUTPUT "** Enumeration: OperandType\n";
print OPOUTPUT "**\n";
print OPOUTPUT "** Purpose: Exposes OperandType Attribute of IL.\n";
print OPOUTPUT $endHeaderComment;

print OPOUTPUT $usingAndRefEmitNmsp;
print OPOUTPUT "    public enum OperandType\n    {\n";

while (<OPCODE>)
{
    # Process only OPDEF(....) lines
    if (/OPDEF\(\s*/)
    {
	chop;               # Strip off trailing CR
	s/^OPDEF\(\s*//;    # Strip off "OP("
	s/,\s*/,/g;         # Remove whitespace
	s/\).*$//;          # Strip off ")" and everything behind it at end

	# Split the line up into its basic parts
	($enumname, $stringname, $pop, $push, $operand, $type, $size, $s1, $s2, $ctrl) = split(/,/);
	$s1 =~ s/0x//;
	$s1 = hex($s1);
	$s2 =~ s/0x//;
	$s2 = hex($s2);

	if ($size == 0)
	{
		next;
	}

	next if ($enumname =~ /UNUSED/);

	#Remove the prefix
	$enumname=~s/CEE_//g;

	#Convert name to our casing convention
	$enumname=~tr/A-Z/a-z/;
	$enumname=~s/^(.)/\u$1/g;
	$enumname=~s/_(.)/_\u$1/g;

	#Convert pop to our casing convention
	$pop=~tr/A-Z/a-z/;
	$pop=~s/^(.)/\u$1/g;
	$pop=~s/_(.)/_\u$1/g;

	#Convert push to our casing convention
	$push=~tr/A-Z/a-z/;
	$push=~s/^(.)/\u$1/g;
	$push=~s/_(.)/_\u$1/g;

	#Convert operand to our casing convention
	#$operand=~tr/A-Z/a-z/;
	#$operand=~s/^(.)/\u$1/g;
	#$operand=~s/_(.)/_\u$1/g;

	#Remove the I prefix on type
	$type=~s/I//g;

	#Convert Type to our casing convention
	$type=~tr/A-Z/a-z/;
	$type=~s/^(.)/\u$1/g;
	$type=~s/_(.)/_\u$1/g;

	#Convert ctrl to our casing convention
	$ctrl=~tr/A-Z/a-z/;
	$ctrl=~s/^(.)/\u$1/g;
	$ctrl=~s/_(.)/_\u$1/g;

	# Make a list of the flow Control type

	# Make a list of the opcodes and their values
	if ($opcodes{$enumname})
	{
	}
	elsif ($size == 1)
	{
		$opcodes{$enumname} = $s2;
	}
	elsif ($size == 2)
	{
        $opcodes{$enumname} = ($s2 + 256 * $s1);
	}

	#Make a list of the instructions which only take one-byte arguments
	if ($enumname =~ /^.*_S$/) {
	    #but exclude the deprecated expressions (sometimes spelled "depricated")
	    if (!($enumname=~/^Depr.cated.*/)) {
		my $caseStatement = sprintf("        case %-20s: \n", $enumname);
		push(@singleByteArg, $caseStatement);
	    }
	}

	#make a list of the control Flow Types
	if ($controlFlow{$ctrl})
	{
		#printf("DUPE Control Flow\n");
	}
	else
	{
		$controlFlow{$ctrl} = $ctrlflowcount;
		$ctrlflowcount++;
	}

	$ctrlflowcount	= 0;
	#make a list of the StackBehaviour Types
	$pop=~s/\+/_/g;
	if ($stackbehav{$pop})
	{
		#printf("DUPE stack behaviour pop\n");
	}
	else
	{
		$stackbehav{$pop} = $ctrlflowcount;
		$ctrlflowcount++;
	}

	#make a list of the StackBehaviour Types
	$push=~s/\+/_/g;
	if ($stackbehav{$push})
	{
		#printf("DUPE stack behaviour push\n");
	}
	else
	{
		$stackbehav{$push} = $ctrlflowcount;
		$ctrlflowcount++;
	}
	#make a list of operand types
	if ($operandtype{$operand})
	{
		#printf("DUPE operand type\n");
	}
	else
	{
		$operandtype{$operand} = $ctrlflowcount;
		$ctrlflowcount++;
	}


	#make a list of opcode types
	if ($opcodetype{$type})
	{
		#printf("DUPE opcode type\n");
	}
	else
	{
		$opcodetype{$type} = $ctrlflowcount;
		$ctrlflowcount++;
	}

    my $opcodeName = $enumname;

	# Tailcall OpCode enum name does not comply with convention
	# that all enum names are exactly the same as names in opcode.def
	# file less leading CEE_ and changed casing convention
	$enumname = substr $enumname, 0, 4 unless $enumname !~ m/Tailcall$/;

	# If string name ends with dot OpCode enum name ends with underscore
	$enumname .= "_" unless $stringname !~ m/\."$/;

    printf(" OpCode name:%20s,\t\tEnum label:%20s,\t\tString name:%20s\n", $opcodeName, $enumname, $stringname);
	if ($stringname eq "arglist")
	{
		print "This is arglist----------\n";
	}

    my $lineEnum;
	if ($size == 1)
	{
	    $lineEnum = sprintf("        %s = 0x%.2x,\n", $enumname, $s2);
		$opcodeEnum{$s2} = $lineEnum;
	}
	elsif ($size == 2)
	{
		$lineEnum = sprintf("        %s = 0x%.4x,\n", $enumname, $s2 + 256 * $s1);
		$opcodeEnum{$s2 + 256 * $s1} = $lineEnum;
	}

	my $line;
	$line = sprintf("        public static readonly OpCode %s = new OpCode(OpCodeValues.%s,\n", $opcodeName, $enumname);
	$line .= sprintf("            ((int)OperandType.%s) |\n", $operand);
	$line .= sprintf("            ((int)FlowControl.%s << OpCode.FlowControlShift) |\n", $ctrl);
	$line .= sprintf("            ((int)OpCodeType.%s << OpCode.OpCodeTypeShift) |\n", $type);
	$line .= sprintf("            ((int)StackBehaviour.%s << OpCode.StackBehaviourPopShift) |\n", $pop);
	$line .= sprintf("            ((int)StackBehaviour.%s << OpCode.StackBehaviourPushShift) |\n", $push);

	$popstate = 0;
	if($pop eq "Pop0" || $pop eq "Varpop")
	{
		$popstate = 0;
	}
	elsif ($pop eq "Pop1" || $pop eq "Popi" || $pop eq "Popref")
	{
		$popstate = $popstate -1;
	}
	elsif ($pop eq "Pop1_pop1" || $pop eq "Popi_pop1" || $pop eq "Popi_popi" || $pop eq "Popi_popi8" || $pop eq "Popi_popr4" || $pop eq "Popi_popr8" || $pop eq "Popref_pop1" || $pop eq "Popref_popi")
	{
		$popstate = $popstate -2;
	}
	elsif ($pop eq "Popi_popi_popi" || $pop eq "Popref_popi_popi" || $pop eq "Popref_popi_popi8" || $pop eq "Popref_popi_popr4" || $pop eq "Popref_popi_popr8" || $pop eq "Popref_popi_popref" || $pop eq "Popref_popi_pop1")
	{
		$popstate = $popstate -3;
	}

	if ($push eq "Push1" || $push eq "Pushi" ||$push eq "Pushi8" ||$push eq "Pushr4" ||$push eq "Pushr8" ||$push eq "Pushref")
	{
		$popstate = $popstate + 1;
	}
	elsif($push eq "Push1_push1")
	{
		$popstate = $popstate + 2;
	}

	$line .= sprintf("            (%s << OpCode.SizeShift) |\n", $size);
	if ($ctrl =~ m/Return/ || $ctrl =~ m/^Branch/ || $ctrl =~ m/^Throw/ || $enumname =~ m/Jmp/){
		$line .= sprintf("            OpCode.EndsUncondJmpBlkFlag |\n", $size);
	}
	$line .= sprintf("            (%d << OpCode.StackChangeShift)\n", $popstate);
	$line .= sprintf("        );\n\n");

	if ($size == 1)
	{
	    if ($oneByte{$s2})
		{
			printf("Error opcode 0x%x  already defined!\n", $s2);
			print "   Old = $oneByte{$s2}";
			print "   New = $line";
			$ret = -1;
	    }
	    $oneByte{$s2} = $line;
	}
	elsif ($size == 2)
	{
	    if ($twoByte{$s2})
		{
			printf("Error opcode 0x%x%x  already defined!\n", $s1, $s2);
			print "   Old = $oneByte{$s2}";
			print "   New = $line";
			$ret = -1;
	    }

	    $twoByte{$s2 + 256 * $s1} = $line;
	}
	else
	{
	    $line .= "\n";
	    push(@deprecated, $line);
		printf("deprecated code!\n");
	}
	$count++;
	}
}

# Generate the Flow Control enum
$ctrlflowcount = 0;
foreach $key (sort {$a cmp $b} keys (%controlFlow))
{
	print FCOUTPUT "        $key";
	print FCOUTPUT " = $ctrlflowcount,\n";
	$ctrlflowcount++;
	if ($key =~ m/Next/){
	    print FCOUTPUT $obsoleteAttr;
		print FCOUTPUT "        Phi";
		print FCOUTPUT " = $ctrlflowcount,\n";
		$ctrlflowcount++;
	}
}
#end the flowcontrol enum
print FCOUTPUT "    }\n}\n";

# Generate the StackBehaviour enum
$ctrlflowcount = 0;
foreach $key (sort {$a cmp $b} keys (%stackbehav))
{
	if ($key !~ m/Popref_popi_pop1/){
		print SOUTPUT "        $key";
		print SOUTPUT " = $ctrlflowcount,\n";
		$ctrlflowcount++;
	}
}
print SOUTPUT "        Popref_popi_pop1 = $ctrlflowcount,\n";
#end the StackBehaviour enum
print SOUTPUT "    }\n}\n";

# Generate OpCodeType enum
$ctrlflowcount = 0;
foreach $key (sort {$a cmp $b} keys (%opcodetype))
{
	if ($ctrlflowcount == 0){
	    print OCOUTPUT $obsoleteAttr;
		print OCOUTPUT "        Annotation = 0,\n";
		$ctrlflowcount++;
	}
	print OCOUTPUT "        $key";
	print OCOUTPUT " = $ctrlflowcount,\n";
	$ctrlflowcount++;
}
# end the OpCodeType enum
print OCOUTPUT "    }\n}\n";

# Generate OperandType enum
$ctrlflowcount = 0;
foreach $key (sort {$a cmp $b} keys (%operandtype))
{
	print OPOUTPUT "        $key";
	print OPOUTPUT " = $ctrlflowcount,\n";
	$ctrlflowcount++;
	if ($key =~ m/InlineNone/){
	    print OPOUTPUT $obsoleteAttr;
		print OPOUTPUT "        InlinePhi = 6,\n";
		$ctrlflowcount++;
	}
	if ($key =~ m/^InlineR$/){
		$ctrlflowcount++;
	}
}
#end the OperandType enum
print OPOUTPUT "    }\n}\n";

# Generate OpCodeValues internal enum
print OUTPUT "    ///<summary>\n";
print OUTPUT "    /// Internal enum OpCodeValues for opcode values.\n";
print OUTPUT "    ///</summary>\n";
print OUTPUT "    ///<remarks>\n";
print OUTPUT "    /// Note that the value names are used to construct publicly visible\n";
print OUTPUT "    /// ilasm-compatible opcode names, so their exact form is important!\n";
print OUTPUT "    ///</remarks>\n";
print OUTPUT "    internal enum OpCodeValues\n";
print OUTPUT "    {\n";

foreach $opcodeValue (sort {$a <=> $b} keys(%opcodeEnum)) {
	print OUTPUT $opcodeEnum{$opcodeValue};
}

# End generating OpCodeValues internal enum
print OUTPUT "    }\n\n";


# Generate public OpCodes class
print OUTPUT "    /// <summary>\n";
print OUTPUT "    ///    <para>\n";
print OUTPUT "    ///       The IL instruction opcodes supported by the runtime.\n";
print OUTPUT "    ///       The Specification of IL Instruction describes each Opcode.\n";
print OUTPUT "    ///    </para>\n";
print OUTPUT "    /// </summary>\n";
print OUTPUT "    /// <seealso topic='IL Instruction Set Specification'/>\n";
print OUTPUT "    public class OpCodes\n";
print OUTPUT "    {\n\n";;
print OUTPUT "        private OpCodes()\n        {\n        }\n\n";

my $opcode;
my $lastOp = -1;
foreach $opcode (sort {$a <=> $b} keys(%oneByte)) {
    printf("***** GAP %d instrs ****\n", $opcode - $lastOp) if ($lastOp + 1 != $opcode && $lastOp > 0);
    print OUTPUT $oneByte{$opcode};
    $lastOp = $opcode;
}

$lastOp = -1;
foreach $opcode (sort {$a <=> $b} keys(%twoByte)) {
    printf("***** GAP %d instrs ****\n", $opcode - $lastOp) if ($lastOp + 1 != $opcode && $lastOp > 0);
    print OUTPUT $twoByte{$opcode};
    $lastOp = $opcode;
}

print OUTPUT "\n";;
print OUTPUT "        public static bool TakesSingleByteArgument(OpCode inst)\n";
print OUTPUT "        {\n";
print OUTPUT "            switch (inst.OperandType)\n";
print OUTPUT "            {\n";
print OUTPUT "                case OperandType.ShortInlineBrTarget:\n";
print OUTPUT "                case OperandType.ShortInlineI:\n";
print OUTPUT "                case OperandType.ShortInlineVar:\n";
print OUTPUT "                    return true;\n";
print OUTPUT "            }\n";
print OUTPUT "            return false;\n";
print OUTPUT "        }\n";

# End Generate public OpCodes class and close namespace
print OUTPUT "    }\n}\n";

exit($ret);
