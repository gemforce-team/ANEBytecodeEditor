package com.cff.anebe.ir
{
    /**
     * An error reported by the assembler while assembling a method that wasn't severe enough to fully break the representation of a method.
     * @author Chris
     */
    public class ASError
    {
        /** The instruction at which this error occurred */
        public var loc:ASLabel;

        /** The message that the assembler gave for this error */
        public var message:String;

        public function ASError(loc:ASLabel = null, message:String = "")
        {
            if (loc != null)
            {
                this.loc = loc;
            }
            else
            {
                this.loc = new ASLabel(0, 0);
            }

            this.message = message;
        }
    }
}
