package com.cff.anebe.ir
{
    /**
     * An error reported by the assembler while assembling a method that wasn't severe enough to fully break the representation of a method.
     * @author Chris
     */
    public class ASError
    {
        /** The instruction at which this error occurred */
        public var loc:ASInstruction;

        /** The message that the assembler gave for this error */
        public var message:String;

        public function ASError(loc:ASInstruction, message:String = "")
        {
            this.loc = loc;
            this.message = message;
        }
    }
}
