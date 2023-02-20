package com.cff.anebe
{
    /**
     * An internally used class for errors that occur at the AS3-C++ border
     */
    public class NestedError extends Error
    {
        public function NestedError(message:String, originalError:Error)
        {
            super(message + "\nNested error: " + originalError.message, originalError.errorID);
        }
    }
}
