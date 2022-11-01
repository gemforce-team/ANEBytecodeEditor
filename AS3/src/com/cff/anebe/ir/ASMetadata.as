package com.cff.anebe.ir
{
    /**
     * A piece of extra information about an ASTrait
     * @author Chris
     */
    public class ASMetadata
    {
        /** The name of this metadata field */
        public var name:String;

        /** The data contained in this field */
        public var data:Vector.<ASMetadataEntry>;

        /**
         * Constructs a metadata field from scratch
         * @param name The name for this field
         * @param data The data contained in this field
         */
        public function ASMetadata(name:String, data:Vector.<ASMetadataEntry>)
        {
            this.name = name;
            if (data == null)
            {
                this.data = new <ASMetadataEntry>[];
            }
            else
            {
                this.data = data;
            }
        }
    }
}
