package com.cff.anebe.ir.multinames
{
    import com.cff.anebe.ir.ASMultiname;
    import com.cff.anebe.ir.ASNamespace;

    /**
     * Builds an ASMultiname that represents a MultinameL
     * @param nsSet Set of namespaces associated with the MultinameL
     * @return Built ASMultiname
     */
    public function MultinameL(nsSet:Vector.<ASNamespace>):ASMultiname
    {
        return new ASMultiname(ASMultiname.TYPE_MULTINAMEL, null, null, nsSet);
    }
}
