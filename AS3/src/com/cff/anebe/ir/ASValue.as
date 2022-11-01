package com.cff.anebe.ir
{
    /**
     * A value stored in either an ASMethod's options or a trait slot/const.
     * May contain one of int, uint, Number, String, Boolean, null, undefined, or ASNamespace
     */
    public class ASValue
    {
        private var _value:*;
        private var _type:String;

        /** The actual value stored */
        public function get value():*
        {
            return _value;
        }

        /** The actual value stored */
        public function set value(v:*):void
        {
            if (v == null)
            {
                _type = "null";
                _value = null;
            }
            else if (v is ASNamespace)
            {
                _type = "Namespace";
                _value = v;
            }
            else if (v is int)
            {
                _type = "int";
                _value = v;
            }
            else if (v is uint)
            {
                _type = "uint";
                _value = v;
            }
            else if (v is Number)
            {
                _type = "Number";
                _value = v;
            }
            else if (v is String)
            {
                _type = "String";
                _value = v;
            }
            else if (v is Boolean)
            {
                _type = (v as Boolean) ? "True" : "False";
                _value = v;
            }
            else
                throw new ArgumentError("Invalid type given to an ASValue");
        }

        /** Used internally */
        public function get type():String
        {
            return _type;
        }

        public function ASValue(value:* = null)
        {
            this.value = value;
        }
    }
}
