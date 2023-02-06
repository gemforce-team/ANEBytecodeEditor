package com.cff.anebe
{
    import flash.utils.ByteArray;
    import flash.utils.CompressionAlgorithm;

    /**
     * Internal utilities
     * @author Chris
     */

    internal class Utils
    {
        public static function decompressSWF(replaceSWF:ByteArray):ByteArray
        {
            replaceSWF.position = 0;
            var decompressor:ByteArray = new ByteArray();
            switch (replaceSWF.readUTFBytes(1))
            {
                case "F":
                    replaceSWF.position = 0;
                    break;
                case "C":
                    replaceSWF.position = 8;
                    replaceSWF.readBytes(decompressor);
                    replaceSWF.position = 8;
                    decompressor.uncompress(CompressionAlgorithm.ZLIB);
                    replaceSWF.writeBytes(decompressor);
                    replaceSWF.position = 0;
                    replaceSWF.writeUTFBytes("F");
                    replaceSWF.position = 0;
                    decompressor = null;
                    break;
                case "Z":
                    replaceSWF.position = 8;
                    replaceSWF.readBytes(decompressor, 8);
                    replaceSWF.position = 8;
                    decompressor.uncompress(CompressionAlgorithm.LZMA);
                    replaceSWF.writeBytes(decompressor);
                    replaceSWF.position = 0;
                    replaceSWF.writeUTFBytes("F");
                    replaceSWF.position = 0;
                    decompressor = null;
                    break;
                default:
                    throw new Error("Unrecognized compression scheme for SWF");
            }

            return replaceSWF;
        }
    }
}
