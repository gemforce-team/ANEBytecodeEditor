package com.cff.anebe.ir.multinames
{
    import com.cff.anebe.ir.ASMultiname;

    /**
     * Builds an ASMultiname that represents a TypeName (as of writing, only ever used for Vector)
     * @param typename Name of type being parameterized
     * @param params Parameters to the type
     * @return Built ASMultiname
     */
    public function TypeName(typename:ASMultiname, params:Vector.<ASMultiname>):ASMultiname
    {
        return new ASMultiname(ASMultiname.TYPE_TYPENAME, null, null, null, typename, params);
    }
}
