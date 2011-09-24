using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;

namespace MemoryMap
{
    class MemoryDataXmlParser
    {
        public MemoryDataXmlParser()
        {
            MemoryData = new List<MemoryRegion>(1000);
            stream = null;
            reader = null;
        }

        ~MemoryDataXmlParser()
        {
            if (reader != null)
            {
                reader.Close();
            }
            if (stream != null)
            {
                stream.Close();
            }
        }

        public void ParseXml(StringBuilder Win32AppOutput)
        {
            try
            {
                stream = new StringReader(Win32AppOutput.ToString());
                reader = new XmlTextReader(stream);


                reader.MoveToContent();
                while (reader.ReadState == ReadState.Interactive && reader.ReadToFollowing("item"))
                {
                    MemoryRegion region = new MemoryRegion();
                    XmlReader SubReader = reader.ReadSubtree();
                    ParseRegion(ref region, ref SubReader);
                    MemoryData.Add(region);
                }
            }
            catch (Exception ex)
            {
                throw new Exception("Cannot parse memory data XML.", ex);
            }
        }

        private void ParseRegion(ref MemoryRegion region, ref XmlReader reader)
        {
            reader.ReadToDescendant("m_region_base_addr");
            region.RegionAddress = (UInt64)reader.ReadElementContentAs(typeof(UInt64), null);
            reader.ReadToNextSibling("m_region_protection");
            region.RegionProtection = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            reader.ReadToNextSibling("region_size");
            region.RegionSize = (UInt64)reader.ReadElementContentAs(typeof(UInt64), null);
            reader.ReadToNextSibling("m_storage_type");
            region.RegionStorageType = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            reader.ReadToNextSibling("m_region_blocks");
            region.NumberOfBlocks = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            reader.ReadToNextSibling("m_guard_blocks");
            region.NumberOfGuardBlocks = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            reader.ReadToNextSibling("m_is_stack");
            region.IsAStack = (Boolean)reader.ReadElementContentAs(typeof(Boolean), null);
            reader.ReadToFollowing("m_blocks");
            XmlReader SubReader = reader.ReadSubtree();
            ParseBlocks(ref region.MemoryBlocks, ref SubReader);
        }

        private void ParseBlocks(ref List<MemoryBlock> list, ref XmlReader reader)
        {
            reader.ReadToFollowing("count");
            UInt32 ListLength = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            if (ListLength == 0)
                return;
            list = new List<MemoryBlock>();
            while (reader.ReadToFollowing("item"))
            {
                XmlReader SubReader = reader.ReadSubtree();
                ParseBlock(ref list, ref SubReader);
            }
        }

        private void ParseBlock(ref List<MemoryBlock> list, ref XmlReader reader)
        {
            MemoryBlock MemBlock = new MemoryBlock();
            reader.ReadToFollowing("m_block_base_addr");
            MemBlock.BlockAddress = (UInt64)reader.ReadElementContentAs(typeof(UInt64), null);
            reader.ReadToNextSibling("protection");
            MemBlock.BlockProtection = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            reader.ReadToNextSibling("m_size");
            MemBlock.BlockSize = (UInt64)reader.ReadElementContentAs(typeof(UInt64), null);
            reader.ReadToNextSibling("m_storage_type");
            MemBlock.BlockStorageType = (UInt32)reader.ReadElementContentAs(typeof(UInt32), null);
            reader.ReadToNextSibling("m_is_shared");
            MemBlock.IsShared = (Boolean)reader.ReadElementContentAs(typeof(Boolean), null);
            reader.ReadToNextSibling("m_map_file_name");
            MemBlock.MappedFileName = (String)reader.ReadElementContentAs(typeof(String), null);
            list.Add(MemBlock);
        }

        public List<MemoryRegion> MemoryData;

        private StringReader stream;
        private XmlReader reader;
    }
}
