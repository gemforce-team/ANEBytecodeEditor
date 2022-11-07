package com.cff.anebe.ir.multinames
{
    import com.cff.anebe.ir.ASMultiname;

    /**
     * Builds an ASMultiname that represents an RTQNameL
     * @return Built ASMultiname
     */
    public function RTQNameL():ASMultiname
    {
        return new ASMultiname(ASMultiname.TYPE_RTQNAMEL);
    }
}
