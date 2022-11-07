package com.cff.anebe.ir.namespaces
{
    import com.cff.anebe.ir.ASNamespace;

    /**
     * Builds an ASNamespace that represents a ProtectedNamespace
     * @param name Name of the namespace
     * @param id If there are multiple of the same namespace name and type, disambiguates between them; should almost always be zero
     * @return Built ASNamespace
     */
    public function ProtectedNamespace(name:String, id:int = 0):ASNamespace
    {
        return new ASNamespace(ASNamespace.TYPE_PROTECTED, name, id);
    }
}
