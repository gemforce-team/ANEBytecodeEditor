package com.cff.anebe.ir.multinames
{
    import com.cff.anebe.ir.ASMultiname;

    /**
     * Builds an ASMultiname that represents an RTQNameA
     * @param name Name of the RTQNameA
     * @return Built ASMultiname
     */
    public function RTQNameA(name:String):ASMultiname
    {
        return new ASMultiname(ASMultiname.TYPE_RTQNAMEA, name);
    }
}
