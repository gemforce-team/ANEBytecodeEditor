package com.cff.anebe.ir.multinames
{
    import com.cff.anebe.ir.ASMultiname;
    import com.cff.anebe.ir.ASNamespace;

    /**
     * Builds an ASMultiname that represents a MultinameA
     * @param nsSet Set of namespaces associated with the MultinameA
     * @param name Name of the MultinameA
     * @return Built ASMultiname
     */
    public function MultinameA(name:String, nsSet:Vector.<ASNamespace>):ASMultiname
    {
        return new ASMultiname(ASMultiname.TYPE_MULTINAMEA, name, null, nsSet);
    }
}
