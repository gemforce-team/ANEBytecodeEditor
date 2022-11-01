package com.cff.anebe.ir
{
    /**
     * A record of where in a method body exceptions are caught and where execution will jump to if they are
     * @author Chris
     */
    public class ASException
    {
        /** The instruction where the exception catch range starts */
        public var from:ASLabel;

        /** The instruction where the exception catch range stops */
        public var to:ASLabel;

        /** The target instruction for the catch block */
        public var target:ASLabel;

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
        public function ASException(from:ASLabel = null, to:ASLabel = null, target:ASLabel = null, exceptionType:ASMultiname = null, exceptionName:ASMultiname = null)
        {
            if (from != null)
            {
                this.from = from;
            }
            else
            {
                this.from = new ASLabel(0, 0);
            }

            if (to != null)
            {
                this.to = to;
            }
            else
            {
                this.to = new ASLabel(0, 0);
            }

            if (target != null)
            {
                this.target = target;
            }
            else
            {
                this.target = new ASLabel(0, 0);
            }

            this.exceptionType = exceptionType;
            this.exceptionName = exceptionName;
        }
    }
}
