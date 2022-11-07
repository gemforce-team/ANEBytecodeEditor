package com.cff.anebe.ir.traits
{
    import com.cff.anebe.ir.ASMetadata;
    import com.cff.anebe.ir.ASMultiname;
    import com.cff.anebe.ir.ASTrait;
    import com.cff.anebe.ir.ASValue;

    /**
     * Builds an ASTrait that represents a slot trait.
     * @param name Name of the trait
     * @param typename Type the trait contains
     * @param value Initial value of the trait
     * @param slotId Slot ID of the trait
     * @param attributes Attributes on the trait
     * @param metadata Metadata on the trait
     * @return The built ASTrait
     */
    public function TraitSlot(name:ASMultiname, typename:ASMultiname, value:ASValue = null, slotId:uint = 0, attributes:Vector.<String> = null, metadata:Vector.<ASMetadata> = null):ASTrait
    {
        return new ASTrait(ASTrait.KIND_SLOT, attributes, slotId, name, typename, metadata, value);
    }
}
