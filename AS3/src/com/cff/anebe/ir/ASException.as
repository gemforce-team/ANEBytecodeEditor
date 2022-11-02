package com.cff.anebe.ir
{
    /**
     * A record of where in a method body exceptions are caught and where execution will jump to if they are
     * @author Chris
     */
    public class ASException
    {
        /** The instruction where the exception catch range starts */
        public var from:ASInstruction;

        /** The instruction where the exception catch range stops */
        public var to:ASInstruction;

        /** The target instruction for the catch block */
        public var target:ASInstruction;

        /** The type of the exception caught by this handler. Null means the any type. */
        public var exceptionType:ASMultiname;

        /** The name of the variable the exception will be assigned to for this catch block. Null means no name. */
        public var exceptionName:ASMultiname;

        /**
         * Builds an ASException. Null labels are interpreted as instruction 0, offset 0
         * @param from The instruction where the exception catch range starts
         * @param to The instruction where the exception catch range stops
         * @param target The target instruction for the catch block
         * @param exceptionType The type of the exception caught by this handler. Null is interpreted as the any type
         * @param exceptionName The name of the variable the exception will be assigned to for this catch block. Null is interpreted as no name
         */
        public function ASException(from:ASInstruction, to:ASInstruction, target:ASInstruction, exceptionType:ASMultiname = null, exceptionName:ASMultiname = null)
        {
            this.from = from;
            this.to = to;
            this.target = target;
            this.exceptionType = exceptionType;
            this.exceptionName = exceptionName;
        }
    }
}
