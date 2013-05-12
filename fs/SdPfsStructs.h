/* Arduino PFS Library
 * Copyright (C) 2013 by Enrique Urbina, Moises Martinez and Néstor Bermúdez
 *
 * This file is part of the Arduino PFS Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino PFS Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/*
 *Creditos: https://github.com/frasermac/sdfatlib
 */
#ifndef SdPfsStructs_h
#define SdPfsStructs_h

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
           /** 32-bit unsigned holding this file's size in bytes. */
  uint32_t fileSize;
           /** 32-bit unsigned holding the block count used for this file. */
  uint32_t blockCount;
           /** 32-bit unsigned pointers (double indirection) if data for a file
            *  direct data blocks if dir_t for a directory.
            */
  uint32_t indirectLv2[9];
           /** 32-bit unsigned pointer to a third level indirect block if data for a file
            *  first level indirection if directory.
            */
  uint32_t indirectLv3;
  
  uint8_t  type;
}__attribute__((packed));

typedef struct directoryEntry dir_t;
//------------------------------------------------------------------------------

struct PFS_boot{
           /**
           * Maximum number of directories to be created on the root directory
           */
  uint32_t rootDirEntryMax;
             /**
           * Size of the bitmap in bytes
           */
  uint32_t bitmapSize;
           /**
           * The size of a hardware sector. Valid decimal values for this
           * field are 512, 1024, 2048, and 4096. For most disks used in
           * the United States, the value of this field is 512.
           */
  uint16_t bytesPerSector;
            /**
           * Sector number of FSINFO structure in the reserved area of the
           * PFS volume. Usually 1.
           */
  uint16_t pfsInfo;
          /**
           * If nonzero, indicates the sector number in the reserved area
           * of the volume of a copy of the boot record. Usually 6.
           * No value other than 6 is recommended.
           */
  uint16_t pfsBackBootBlock;
            /**
           * The number of copies of the PFS on the volume.
           * The value of this field is always 2.
           */
  uint8_t  pfsCount;
         /**
           * Count of sectors occupied by one PFS on PFS volumes.
           */
  uint32_t sectorsPerPfs;
            /**
           * The number of sectors preceding the start of the first PFS,
           * including the boot sector. The value of this field is always 1.
           */
  uint16_t reservedSectorCount;
           /** PFS for our project!*/
  char     fileSystemType[6];
           /** must be 0X55 */
  uint8_t  bootSectorSig0;
           /** must be 0XAA */
  uint8_t  bootSectorSig1;

  uint8_t  padding[3];
}__attribute__((packed));

typedef struct PFS_boot pfs_boot_t;

struct PFS_info{
          /**
           * Contains the last known free cluster count on the volume.
           * If the value is 0xFFFFFFFF, then the free count is unknown
           * and must be computed. Any other value can be used, but is
           * not necessarily correct. It should be range checked at least
           * to make sure it is <= volume cluster count.
           */
  uint32_t freeCount;
          /**
           * This is a hint for the FAT driver. It indicates the cluster
           * number at which the driver should start looking for free clusters.
           * If the value is 0xFFFFFFFF, then there is no hint and the driver
           * should start looking at cluster 2.
           */
  uint32_t nextFree;
}__attribute__((packed));

typedef struct PFS_info pfs_info_t;


//------------------------------------------------------------------------------
// Definitions for directory entries
//
/** Type name for directoryEntry */
typedef struct directoryEntry dir_t;

/** escape for name[0] = 0XE5 */
uint8_t const DIR_NAME_0XE5 = 0X05;
/** name[0] value for entry that is free after being "deleted" */
uint8_t const DIR_NAME_DELETED = 0XE5;
/** name[0] value for entry that is free and no allocated entries follow */
uint8_t const DIR_NAME_FREE = 0X00;
/** file is read-only */
uint8_t const DIR_ATT_READ_ONLY = 0X01;
/** File should hidden in directory listings */
uint8_t const DIR_ATT_HIDDEN = 0X02;
/** Entry is for a system file */
uint8_t const DIR_ATT_SYSTEM = 0X04;
/** Directory entry contains the volume label */
uint8_t const DIR_ATT_VOLUME_ID = 0X08;
/** Entry is for a directory */
uint8_t const DIR_ATT_DIRECTORY = 0X10;
/** Old DOS archive bit for backup support */
uint8_t const DIR_ATT_ARCHIVE = 0X20;
/** Test value for long name entry.  Test is
  (d->attributes & DIR_ATT_LONG_NAME_MASK) == DIR_ATT_LONG_NAME. */
uint8_t const DIR_ATT_LONG_NAME = 0X0F;
/** Test mask for long name entry */
uint8_t const DIR_ATT_LONG_NAME_MASK = 0X3F;
/** defined attribute bits */
uint8_t const DIR_ATT_DEFINED_BITS = 0X3F;
/** Test mask for allowing entry to contain subdirectories*/
uint8_t const DIR_ATT_ALLOW_SUBDIRS = 0X80;
/** Test mask for allowing entry to contain files*/
uint8_t const DIR_ATT_ALLOW_FILES = 0X40;
/** Directory entry is part of a long name
 * \param[in] dir Pointer to a directory entry.
 *
 * \return true if the entry is for part of a long name else false.
 */
static inline uint8_t DIR_IS_LONG_NAME(const dir_t* dir) {
  return (dir->attributes & DIR_ATT_LONG_NAME_MASK) == DIR_ATT_LONG_NAME;
}
/** Mask for file/subdirectory tests */
uint8_t const DIR_ATT_FILE_TYPE_MASK = (DIR_ATT_VOLUME_ID | DIR_ATT_DIRECTORY);
/** Directory entry is for a file
 * \param[in] dir Pointer to a directory entry.
 *
 * \return true if the entry is for a normal file else false.
 */
static inline uint8_t DIR_IS_FILE(const dir_t* dir) {
  return (dir->attributes & DIR_ATT_FILE_TYPE_MASK) == 0;
}
/** Directory entry is for a subdirectory
 * \param[in] dir Pointer to a directory entry.
 *
 * \return true if the entry is for a subdirectory else false.
 */
static inline uint8_t DIR_IS_SUBDIR(const dir_t* dir) {
  return (dir->attributes & DIR_ATT_FILE_TYPE_MASK) == DIR_ATT_DIRECTORY;
}
/** Directory entry is for a file or subdirectory
 * \param[in] dir Pointer to a directory entry.
 *
 * \return true if the entry is for a normal file or subdirectory else false.
 */
static inline uint8_t DIR_IS_FILE_OR_SUBDIR(const dir_t* dir) {
  return (dir->attributes & DIR_ATT_VOLUME_ID) == 0;
}
/** Directory entry allows subdirectories on it (ONLY VALID FOR ROOT DIRECTORY!)
 * \param[in] dir Pointer to a directory entry.
 *
 * \return true if the entry allows creating subdirs else false.
 */
static inline uint8_t DIR_ALLOW_SUBDIRS(const dir_t* dir) {
  return (dir->attributes & DIR_ALLOW_SUBDIRS) != 0;
}
/** Directory entry allows files on it (level 1 directory)
 * \param[in] dir Pointer to a directory entry.
 *
 * \return true if the entry allows creating files else false.
 */
static inline uint8_t DIR_ALLOW_FILES(const dir_t* dir) {
  return (dir->attributes & DIR_ATT_ALLOW_FILES) != 0;
}

#endif  //SdPfsStructs_h