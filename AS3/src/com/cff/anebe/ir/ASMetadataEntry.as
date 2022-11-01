package com.cff.anebe.ir
{
    /**
     * An entry in an ASMetadata
     * @author Chris
     */
    public class ASMetadataEntry
    {
        /** The key for this metadata entry */
        public var key:String;

        /** The value for this metadata entry */
        public var value:String;

        /**
         * Builds a metadata entry from scratch
         * @param key The key for the entry
         * @param value The value for the entry
         */
        public function ASMetadataEntry(key:String = "", value:String = "")
        {
            this.key = key;
            this.value = value;
        }
    }
}
