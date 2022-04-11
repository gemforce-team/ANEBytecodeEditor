#pragma once

#include "enums/TagType.hpp"
#include "utils/StringException.hpp"
#include <bit>
#include <cassert>
#include <filesystem>
#include <stdint.h>
#include <vector>

namespace SWF
{
    // Note: does not handle compression. The SWF data passed in must be pre-decompressed.
    // Note: must be compiled for little-endian architectures
    class SWFFile
    {
    public:
        struct Tag
        {
            const uint8_t* data      = nullptr;
            const uint8_t* patchData = nullptr;
            size_t patchLength       = 0;
            size_t patchOffset       = 0;
            uint32_t length          = 0;
            TagType type             = TagType::End;
            bool forceLongLength     = false;
        };
        struct Header
        {
            char magic[3];
            uint8_t version;
            uint32_t fileLength;
        };
        static_assert(sizeof(Header) == 8);
        static_assert(std::endian::native == std::endian::little);

        size_t numAbcTags() const { return abcTags.size(); }

        std::pair<const uint8_t*, size_t> abcData(size_t abcTagIndex = 0) const
        {
            const Tag& tag         = abcTag(abcTagIndex);
            const uint8_t* abcData = tag.data;
            abcData += 4; // skip flags
            while (*abcData++)
                ; // skip name
            return {abcData, tag.length - (std::distance<const uint8_t*>(tag.data, abcData))};
        }

        // Note: does not own this data pointer. Data pointer must be valid for the time this
        // SWFFile is being used.
        void replaceABCData(const uint8_t* data, size_t length, size_t index = 0)
        {
            Tag& tag        = abcTag(index);
            tag.patchData   = data;
            tag.patchLength = length;

            const uint8_t* abcData = tag.data;
            abcData += 4; // skip flags
            while (*abcData++)
                ; // skip name

            tag.patchOffset = std::distance<const uint8_t*>(tag.data, abcData);
        }

        size_t getFullSize() const
        {
            size_t ret =
                sizeof(Header) + frameSizeData.size() + sizeof(frameRate) + sizeof(frameCount);
            for (const Tag& t : tags)
            {
                ret += 2; // Type and small size
                if (t.patchData != nullptr)
                {
                    if (t.patchLength + t.patchOffset >= 0x3F || t.forceLongLength)
                    {
                        ret += 4;
                    }
                    ret += t.patchLength + t.patchOffset;
                }
                else
                {
                    if (t.length >= 0x3F || t.forceLongLength)
                    {
                        ret += 4;
                    }
                    ret += t.length;
                }
            }

            return ret;
        }

        // Storage must be large enough to fit getFullSize() bytes
        void writeTo(uint8_t* out) const
        {
            const uint32_t maxSize = getFullSize();
            size_t currentPos      = 0;
            auto writeData         = [out, maxSize, &currentPos](size_t writeSize, const auto* in) {
                if (writeSize == 0)
                    return;
                if (currentPos + writeSize > maxSize)
                    throw StringException("SWF file write went out of bounds");
                std::copy(reinterpret_cast<const uint8_t*>(in),
                    reinterpret_cast<const uint8_t*>(in) + writeSize,
                    reinterpret_cast<uint8_t*>(out) + currentPos);
                currentPos += writeSize;
            };

            writeData(3, header.magic);
            writeData(1, &header.version);
            writeData(4, &maxSize);

            writeData(frameSizeData.size(), frameSizeData.data());

            writeData(2, &frameRate);
            writeData(2, &frameCount);

            auto writeTag = [&writeData, &currentPos](const Tag& tag) {
                uint16_t rawTagData = uint16_t(tag.type) << 6;
                if (tag.patchData != nullptr)
                {
                    const uint32_t finalLength = tag.patchLength + tag.patchOffset;
                    if (finalLength >= 0x3F)
                    {
                        rawTagData |= 0x3F;
                        writeData(2, &rawTagData);
                        writeData(4, &finalLength);
                    }
                    else
                    {
                        rawTagData |= uint8_t(finalLength);
                        writeData(2, &rawTagData);
                    }
                    writeData(tag.patchOffset, tag.data);
                    writeData(tag.patchLength, tag.patchData);
                }
                else
                {
                    if (tag.length >= 0x3F || tag.forceLongLength)
                    {
                        rawTagData |= 0x3F;
                        writeData(2, &rawTagData);
                        writeData(4, &tag.length);
                    }
                    else
                    {
                        rawTagData |= uint8_t(tag.length);
                        writeData(2, &rawTagData);
                    }
                    writeData(tag.length, tag.data);
                }
            };

            for (const auto& tag : tags)
            {
                writeTag(tag);
            }
        }

        // SWF must already be decompressed
        SWFFile(std::vector<uint8_t>&& _data) : data(std::move(_data))
        {
            size_t currentPos = 0;
            auto readData     = [this, &currentPos](size_t readSize, auto* out) {
                if (currentPos + readSize > data.size())
                    throw StringException("SWF file read went out of bounds");
                std::copy(data.begin() + currentPos, data.begin() + currentPos + readSize,
                    reinterpret_cast<uint8_t*>(out));
                currentPos += readSize;
            };
            readData(3, header.magic);
            readData(1, &header.version);
            readData(4, &header.fileLength);

            if (header.fileLength != data.size())
                throw StringException(
                    "Header length is larger than the given size. This should not happen!");

            uint8_t rectSize;
            readData(1, &rectSize);
            uint32_t nbits  = rectSize >> 3;
            uint32_t nbytes = ((5 + 4 * nbits) + 7) / 8;
            currentPos--;
            frameSizeData.resize(nbytes);
            readData(nbytes, frameSizeData.data());

            readData(2, &frameRate);
            readData(2, &frameCount);

            auto readTag = [this, &readData, &currentPos] {
                Tag ret;
                uint16_t rawTagData;
                readData(2, &rawTagData);
                ret.type   = TagType(rawTagData >> 6);
                ret.length = rawTagData & 0x3F;
                if (ret.length == 0x3F)
                {
                    readData(4, &ret.length);
                    if (ret.length < 0x3F)
                    {
                        ret.forceLongLength = true;
                    }
                }
                if (currentPos + ret.length > data.size())
                    throw StringException("SWF tag read went out of bounds");
                ret.data = data.data() + currentPos;
                currentPos += ret.length;
                return ret;
            };

            while (currentPos < data.size())
            {
                Tag tag = readTag();
                if (tag.type == TagType::DoABC2)
                {
                    abcTags.emplace_back(tags.size());
                }
                tags.emplace_back(std::move(tag));
            }
        }

    private:
        const Tag& abcTag(size_t index = 0) const
        {
            if (index >= abcTags.size())
            {
                throw StringException("ABC Tag " + std::to_string(index + 1) +
                                      " does not exist (there are " +
                                      std::to_string(abcTags.size()) + ")");
            }
            return tags[abcTags[index]];
        }

        Tag& abcTag(size_t index = 0)
        {
            if (index >= abcTags.size())
            {
                throw StringException("ABC Tag " + std::to_string(index + 1) +
                                      " does not exist (there are " +
                                      std::to_string(abcTags.size()) + ")");
            }
            return tags[abcTags[index]];
        }

        std::vector<uint8_t> data;

        Header header;
        uint16_t frameRate, frameCount;
        std::vector<uint8_t> frameSizeData;
        std::vector<Tag> tags;
        std::vector<size_t> abcTags;
    };
}
