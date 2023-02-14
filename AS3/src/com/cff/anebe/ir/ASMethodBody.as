package com.cff.anebe.ir
{
    /**
     * A representation of a compiled AS3 method. Any ASMethod with one of these was originally compiled from ActionScript and is not a native method.
     * @author Chris
     */
    public class ASMethodBody
    {
        /** The maximum stack depth this function allows */
        public var maxStack:uint;

        /** The number of local variables this function has */
        public var localCount:uint;

        /** The scope depth at which this function starts */
        public var initScopeDepth:uint;

        /** The maximum scope depth this function can reach */
        public var maxScopeDepth:uint;

        /** The list of instructions that make up the code of this body */
        public var instructions:Vector.<ASInstruction>;

        /** The list of try-catch blocks in this body */
        public var exceptions:Vector.<ASException>;

        /** The list of traits in this body */
        public var traits:Vector.<ASTrait>;

        /** The list of errors that occurred while assembling this body */
        public var errors:Vector.<ASError>;

        /**
         * Builds an InstructionStream to modify this method body's instructions.
         * @param reverse Whether to start the stream in reverse or forwards mode
         * @return The built stream
         */
        public function streamInstructions(reverse:Boolean = false):InstructionStream
        {
            return new InstructionStream(instructions, reverse);
        }

        /**
         * Edits all instructions that jump to a given instruction to jump to a different instruction
         * @param originalTarget The original jump target
         * @param newTarget The jump target to replace the original with
         */
        public function redirectJumps(originalTarget:ASInstruction, newTarget:ASInstruction):void
        {
            for each (var instr:ASInstruction in instructions)
            {
                if (instr.isJump() && instr.args[0] == originalTarget)
                {
                    instr.args[0] = newTarget;
                }
                if (instr.opcode == ASInstruction.OP_lookupswitch)
                {
                    var switchTargets:Vector.<ASInstruction> = instr.args[1];
                    for (var i:int = 0; i < switchTargets.length; i++)
                    {
                        if (switchTargets[i] == originalTarget)
                        {
                            switchTargets[i] = newTarget;
                        }
                    }
                }
            }
        }

        /**
         * Creates an ASMethodBody from scratch
         * @param maxStack Maximum stack depth that can be reached
         * @param localCount Number of local variables
         * @param initScopeDepth Starting scope depth
         * @param maxScopeDepth Maximum scope depth
         * @param instructions List of instructions
         * @param exceptions List of try-catch blocks
         * @param traits List of traits
         * @param errors List of errors (if you're building this, should probably be empty)
         */
        public function ASMethodBody(maxStack:uint = 0, localCount:uint = 0, initScopeDepth:uint = 0, maxScopeDepth:uint = 0, instructions:Vector.<ASInstruction> = null, exceptions:Vector.<ASException> = null, traits:Vector.<ASTrait> = null, errors:Vector.<ASError> = null)
        {
            this.maxStack = maxStack;
            this.localCount = localCount;
            this.initScopeDepth = initScopeDepth;
            this.maxScopeDepth = maxScopeDepth;
            if (instructions == null)
            {
                this.instructions = new <ASInstruction>[];
            }
            else
            {
                this.instructions = instructions;
            }
            if (exceptions == null)
            {
                this.exceptions = new <ASException>[];
            }
            else
            {
                this.exceptions = exceptions;
            }
            if (traits == null)
            {
                this.traits = new <ASTrait>[];
            }
            else
            {
                this.traits = traits;
            }
            if (errors == null)
            {
                this.errors = new <ASError>[];
            }
            else
            {
                this.errors = errors;
            }
        }
    }
}
