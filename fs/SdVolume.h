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
#ifndef SdVolume_h
#define SdVolume_h
/**
 * \file
 * \brief SdVolume class
 */
#include <SdPfsConfig.h>
#include <Sd2Card.h>
#include <SdPfsStructs.h>

//==============================================================================
// SdVolume class
/**
 * \brief Cache for an SD data block
 */
union cache_t {
           /** Used to access cached file data blocks. */
  uint8_t  data[512];
           /** Used to access cached directory entries. */
  dir_t    dir[8];
           /** Used to access a cached Master Boot Record. */
  mbr_t    mbr;
           /** Used to access to a cached PFS boot sector. */
  pfs_boot_t pbs;
           /** Used to access to a cached PFS FSINFO sector. */
  pfs_info_t fsinfo;
};
//------------------------------------------------------------------------------
/**
 * \class SdVolume
 * \brief Access FAT16 and FAT32 volumes on SD and SDHC cards.
 */
class SdVolume {
 public:
  /** Create an instance of SdVolume */
  SdVolume(){}
  /** Clear the cache and returns a pointer to the cache.  Used by the WaveRP
   * recorder to do raw write to the SD card.  Not for normal apps.
   * \return A pointer to the cache buffer or zero if an error occurs.
   */
  cache_t* cacheClear() {
    if (!cacheSync()) return 0;
    cacheBlockNumber_ = 0XFFFFFFFF;
    return &cacheBuffer_;
  }
  /** Initialize a PFS volume.  Try partition one first then try super
   * floppy format.
   *
   * \param[in] dev The Sd2Card where the volume is located.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.  Reasons for
   * failure include not finding a valid partition, not finding a valid
   * FAT file system or an I/O error.
   */
  bool init(Sd2Card* dev) { return init(dev, 1) ? true : init(dev, 0);}
  bool init(Sd2Card* dev, uint8_t part);

  // inline functions that return volume info
  /** \return The volume's cluster size in blocks. */
  uint8_t blocksPerCluster() const {return blocksPerCluster_;}
  /** \return The total number of clusters in the volume. */
  uint32_t clusterCount() const {return clusterCount_;}
  /** \return The shift count required to multiply by blocksPerCluster. */
  uint8_t clusterSizeShift() const {return clusterSizeShift_;}
  /** \return The logical block number for the start of file data. */
  uint32_t dataStartBlock() const {return dataStartBlock_;}
  /** \return The logical block number for the start of the first FAT. */
  uint32_t fatStartBlock() const {return fatStartBlock_;}
  /** \return The FAT type of the volume. Values are 12, 16 or 32. */
  uint8_t fatType() const {return fatType_;}
  int32_t freeClusterCount();
  /** \return The number of entries in the root directory for FAT16 volumes. */
  uint32_t rootDirEntryCount() const {return rootDirEntryCount_;}
  /** \return The logical block number for the start of the root directory
       on FAT16 volumes or the first cluster number on FAT32 volumes. */
  uint32_t rootDirStart() const {return rootDirStart_;}

private:

  uint8_t pfsCount_;            // number of PFSs on volume
  uint16_t rootDirEntryMax_;  // number of entries in PFS root dir
  uint32_t sectorsPerPfs_;      // PFS size in blocks
  uint32_t fatStartBlock_;      // start block for first PFS


#if USE_MULTIPLE_CARDS
  cache_t cacheBuffer_;        // 512 byte cache for device blocks
  uint32_t cacheBlockNumber_;  // Logical number of block in the cache
  uint32_t cachePfsOffset_;    // offset for mirrored PFS
  Sd2Card* sdCard_;            // Sd2Card object for cache
  uint8_t cacheStatus_;        // status of cache block
#else  // USE_MULTIPLE_CARDS
  static cache_t cacheBuffer_;        // 512 byte cache for device blocks
  static uint32_t cacheBlockNumber_;  // Logical number of block in the cache
  static uint32_t cachePfsOffset_;    // offset for mirrored PFS
  static uint8_t cacheStatus_;        // status of cache block
  static Sd2Card* sdCard_;            // Sd2Card object for cache
#endif  // USE_MULTIPLE_CARDS

// block caches
// use of static functions save a bit of flash - maybe not worth complexity
//
  static const uint8_t CACHE_STATUS_DIRTY = 1;
  static const uint8_t CACHE_STATUS_PFS_BLOCK = 2;
  static const uint8_t CACHE_STATUS_MASK
     = CACHE_STATUS_DIRTY | CACHE_STATUS_PFS_BLOCK;
  static const uint8_t CACHE_OPTION_NO_READ = 4;
  // value for option argument in cacheFetch to indicate read from cache
  static uint8_t const CACHE_FOR_READ = 0;
  // value for option argument in cacheFetch to indicate write to cache
  static uint8_t const CACHE_FOR_WRITE = CACHE_STATUS_DIRTY;
  // reserve cache block with no read
  static uint8_t const CACHE_RESERVE_FOR_WRITE
     = CACHE_STATUS_DIRTY | CACHE_OPTION_NO_READ;
};

 #endif // SdVolume_h