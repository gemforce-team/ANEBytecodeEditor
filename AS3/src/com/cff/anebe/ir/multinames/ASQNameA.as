package com.cff.anebe.ir.multinames
{
    import com.cff.anebe.ir.ASMultiname;
    import com.cff.anebe.ir.ASNamespace;

    /**
     * Builds an ASMultiname that represents a QNameA
     * @param ns Namespace of the QNameA
     * @param name Name of the QNameA
     * @return Built ASMultiname
     */
    public function ASQNameA(ns:ASNamespace, name:String):ASMultiname
    {
        return new ASMultiname(ASMultiname.TYPE_QNAMEA, name, ns);
    }
}
