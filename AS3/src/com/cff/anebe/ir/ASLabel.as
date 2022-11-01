package com.cff.anebe.ir
{
    /**
     * A location of an instruction within an ASMethodBody
     * @author Chris
     */

    public class ASLabel
    {
        /** The instruction index of the first instruction after this label */
        public var instrIndex:uint;

        /** The offset from the label the instrIndex refers to */
        public var offset:int;

        /**
         * Builds an ASLabel from scratch
         * @param idx The instruction index of the first instruction after this label
         * @param ofs The instruction index offset from the actual label
         */
        public function ASLabel(idx:uint = 0, ofs:int = 0)
        {
            instrIndex = idx;
            offset = ofs;
        }
    }
}
