package com.cff.anebe.ir.traits
{
    import com.cff.anebe.ir.ASMetadata;
    import com.cff.anebe.ir.ASMethod;
    import com.cff.anebe.ir.ASMultiname;
    import com.cff.anebe.ir.ASTrait;

    /**
     * Builds an ASTrait that represents a method trait.
     * @param name Name of the trait
     * @param method Method the trait contains
     * @param slotId Slot ID of the trait
     * @param attributes Attributes on the trait
     * @param metadata Metadata on the trait
     * @return The built ASTrait
     */
    public function TraitMethod(name:ASMultiname, method:ASMethod, slotId:uint = 0, attributes:Vector.<String> = null, metadata:Vector.<ASMetadata> = null):ASTrait
    {
        return new ASTrait(ASTrait.KIND_METHOD, attributes, slotId, name, null, metadata, null, method);
    }
}
