/**
 * \struct masterBootRecord
 *
 * \brief Master Boot Record
 *
 * The first block of a storage device that is formatted with a MBR.
 */
 //------------------------------------------------------------------------------
/**
 * \struct partitionTable
 * \brief MBR partition table entry
 *
 * A partition table entry for a MBR formatted storage device.
 * The MBR partition table has four entries.
 */
struct partitionTable {
          /**
           * Boot Indicator . Indicates whether the volume is the active
           * partition.  Legal values include: 0X00. Do not use for booting.
           * 0X80 Active partition.
           */
  uint8_t  boot;
          /**
            * Head part of Cylinder-head-sector address of the first block in
            * the partition. Legal values are 0-255. Only used in old PC BIOS.
            */
  uint8_t  beginHead;
          /**
           * Sector part of Cylinder-head-sector address of the first block in
           * the partition. Legal values are 1-63. Only used in old PC BIOS.
           */
  unsigned beginSector : 6;
           /** High bits cylinder for first block in partition. */
  unsigned beginCylinderHigh : 2;
          /**
           * Combine beginCylinderLow with beginCylinderHigh. Legal values
           * are 0-1023.  Only used in old PC BIOS.
           */
  uint8_t  beginCylinderLow;
          /**
           * Partition type. See defines that begin with PART_TYPE_ for
           * some Microsoft partition types.
           */
  uint8_t  type;
          /**
           * head part of cylinder-head-sector address of the last sector in the
           * partition.  Legal values are 0-255. Only used in old PC BIOS.
           */
  uint8_t  endHead;
          /**
           * Sector part of cylinder-head-sector address of the last sector in
           * the partition.  Legal values are 1-63. Only used in old PC BIOS.
           */
  unsigned endSector : 6;
           /** High bits of end cylinder */
  unsigned endCylinderHigh : 2;
          /**
           * Combine endCylinderLow with endCylinderHigh. Legal values
           * are 0-1023.  Only used in old PC BIOS.
           */
  uint8_t  endCylinderLow;
           /** Logical block address of the first block in the partition. */
  uint32_t firstSector;
           /** Length of the partition, in blocks. */
  uint32_t totalSectors;
}__attribute__((packed));
/** Type name for partitionTable */
typedef struct partitionTable part_t;
//------------------------------------------------------------------------------
struct masterBootRecord {
           /** Code Area for master boot program. */
  uint8_t  codeArea[446];
           /** Partition tables. */
  part_t   part[4];
           /** First MBR signature byte. Must be 0X55 */
  uint8_t  mbrSig0;
           /** Second MBR signature byte. Must be 0XAA */
  uint8_t  mbrSig1;
}__attribute__((packed));
/** Type name for masterBootRecord */
typedef struct masterBootRecord mbr_t;
/**
 * \struct directoryEntry
 * \brief FAT short directory entry
 *
 * Short means short 8.3 name, not the entry size.
 *  
 */
struct directoryEntry {
           /** Short 8.3 name.
            *
            * The first eight bytes contain the file name with blank fill.
            * The last three bytes contain the file extension with blank fill.
            */
  uint8_t  name[11];   
          /** Entry attributes.
           *
           * The upper two bits of the attribute byte are reserved and should
           * always be set to 0 when a file is created and never modified or
           * looked at after that.  See defines that begin with DIR_ATT_.
           * UPDATE: We will use bit 0 for allow_subdirs and bit 1 for allow_files
           */
  uint8_t  attributes;
          /**
           * High word of this entry's first cluster number (always 0 for a
           * FAT12 or FAT16 volume).
           */
  uint16_t firstClusterHigh;
           /** Time of last write. File creation is considered a write. */
  uint16_t lastWriteTime;
           /** Date of last write. File creation is considered a write. */
  uint16_t lastWriteDate;
           /** Low word of this entry's first cluster number. */
  uint16_t firstClusterLow;
           /** 32-bit unsigned holding this file's size in bytes. */
  uint32_t fileSize;
           /** 32-bit unsigned holding the block count used for this file. */
  uint32_t blockCount;
           /** 32-bit unsigned pointers (double indirection) */
  uint32_t indirectLv2[10];
           /** 32-bit unsigned pointer to a third level indirect block */
  uint32_t indirectLv3;
}__attribute__((packed));
//------------------------------------------------------------------------------
// Definitions for directory entries
//
/** Type name for directoryEntry */
typedef struct directoryEntry dir_t;