package com.cff.anebe.ir.multinames
{
    import com.cff.anebe.ir.ASMultiname;

    /**
     * Builds an ASMultiname that represents an RTQName
     * @param name Name of the RTQName
     * @return Built ASMultiname
     */
    public function RTQName(name:String):ASMultiname
    {
        return new ASMultiname(ASMultiname.TYPE_RTQNAME, name);
    }
}
