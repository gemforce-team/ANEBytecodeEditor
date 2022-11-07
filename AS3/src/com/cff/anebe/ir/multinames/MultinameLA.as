package com.cff.anebe.ir.multinames
{
    import com.cff.anebe.ir.ASMultiname;
    import com.cff.anebe.ir.ASNamespace;

    /**
     * Builds an ASMultiname that represents a MultinameLA
     * @param nsSet Set of namespaces associated with the MultinameLA
     * @return Built ASMultiname
     */
    public function MultinameLA(nsSet:Vector.<ASNamespace>):ASMultiname
    {
        return new ASMultiname(ASMultiname.TYPE_MULTINAMELA, null, null, nsSet);
    }
}
