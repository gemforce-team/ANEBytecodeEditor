package com.cff.anebe.ir.multinames
{
    import com.cff.anebe.ir.ASMultiname;

    /**
     * Builds an ASMultiname that represents an RTQNameLA
     * @return Built ASMultiname
     */
    public function RTQNameLA():ASMultiname
    {
        return new ASMultiname(ASMultiname.TYPE_RTQNAMELA);
    }
}
