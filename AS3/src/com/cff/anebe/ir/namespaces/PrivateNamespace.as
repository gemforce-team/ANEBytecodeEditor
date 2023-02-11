package com.cff.anebe.ir.namespaces
{
    import com.cff.anebe.ir.ASNamespace;

    /**
     * Builds an ASNamespace that represents an PrivateNamespace
     * @param name Name of the namespace
     * @param disambiguator If there are multiple of the same namespace name and type, disambiguates between them; should almost always be null
     * @return Built ASNamespace
     */
    public function PrivateNamespace(name:String, disambiguator:String = null):ASNamespace
    {
        return new ASNamespace(ASNamespace.TYPE_PRIVATE, name, disambiguator);
    }
}
