package com.cff.anebe.ir.multinames
{
    import com.cff.anebe.ir.ASMultiname;
    import com.cff.anebe.ir.ASNamespace;

    /**
     * Builds an ASMultiname that represents a Multiname
     * @param nsSet Set of namespaces associated with the Multiname
     * @param name Name of the Multiname
     * @return Built ASMultiname
     */
    public function Multiname(name:String, nsSet:Vector.<ASNamespace>):ASMultiname
    {
        return new ASMultiname(ASMultiname.TYPE_MULTINAME, name, null, nsSet);
    }
}
