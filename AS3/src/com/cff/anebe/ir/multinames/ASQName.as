package com.cff.anebe.ir.multinames
{
    import com.cff.anebe.ir.ASMultiname;
    import com.cff.anebe.ir.ASNamespace;

    /**
     * Builds an ASMultiname that represents a QName
     * @param ns Namespace of the QName
     * @param name Name of the QName
     * @return Built ASMultiname
     */
    public function ASQName(ns:ASNamespace, name:String):ASMultiname
    {
        return new ASMultiname(ASMultiname.TYPE_QNAME, name, ns);
    }
}
