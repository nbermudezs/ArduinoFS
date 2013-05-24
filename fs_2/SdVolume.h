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

#include <SdPfsConfig.h>
#include <Sd2Card.h>
#include <SdPfsStructs.h>

union cache_t {
           /** Used to access cached file data blocks. */
  uint8_t  data[512];
           /** Used to access cached directory entries. */
  dir_t    dir[8];
           /** Used to access cached FAT32 entries. */
  uint32_t fat32[128];
           /** Used to access a cached Master Boot Record. */
  mbr_t    mbr;
           /** Used to access to a cached PFS boot sector. */
  pfs_boot_t pbs;
           /** Used to access to a cached PFS FSINFO sector. */
  pfs_info_t fsinfo;
};

class SdVolume {
 public:
  SdVolume() :allocSearchStart_(2){}

  cache_t* cacheClear() {
    if (!cacheSync()) return 0;
    cacheBlockNumber_ = 0XFFFFFFFF;
    return &cacheBuffer_;
  }

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
  /** \return The logical block number for the start of the first PFS. */
  uint32_t pfsStartBlock() const {return pfsStartBlock_;}
  int32_t freeClusterCount();
  /** \return The number of entries in the root directory for FAT16 volumes. */
  uint32_t rootDirEntryCount() const {return rootDirEntryCount_;}
  /** \return The logical block number for the start of the root directory
       on FAT16 volumes or the first cluster number on FAT32 volumes. */
  uint32_t rootDirStart() const {return rootDirStart_;}

  Sd2Card* sdCard() {return sdCard_;}

  bool dbgPfs(uint32_t n, uint32_t* v) {return pfsGet(n, v);}

private:
  friend class SdBaseFile;      // Allow SdBaseFile access to SdVolume private data.

  uint8_t pfsCount_;            // number of PFSs on volume
  uint16_t rootDirEntryMax_;    // maximum number of entries in PFS root dir
  uint16_t rootDirEntryCount_;    // number of entries in PFS root dir
  uint32_t sectorsPerPfs_;      // PFS size in blocks
  uint32_t pfsStartBlock_;      // start block for first PFS
  uint8_t blocksPerCluster_;    // cluster size in blocks
  uint8_t clusterSizeShift_;    // shift to convert cluster count to block count (always 0 for us)
  uint32_t rootDirStart_;       // root start block for PFS
  uint32_t dataStartBlock_;     // first data block number
  uint32_t clusterCount_;       // clusters in one PFS
  uint32_t allocSearchStart_;   // start cluster for alloc search


  static cache_t cacheBuffer_;        // 512 byte cache for device blocks
  static uint32_t cacheBlockNumber_;  // Logical number of block in the cache
  static uint32_t cachePfsOffset_;    // offset for mirrored PFS
  static uint8_t cacheStatus_;        // status of cache block
  static Sd2Card* sdCard_;            // Sd2Card object for cache
  
  
  static bool cacheSync();
  static cache_t* cacheFetchPfs(uint32_t blockNumber, uint8_t options);
  static cache_t* cacheFetch(uint32_t blockNumber, uint8_t options);
  static void cacheInvalidate();
  static bool cacheWriteData();
  static bool cacheWritePfs();

  bool pfsGet(uint32_t cluster, uint32_t* value);
  bool pfsPut(uint32_t cluster, uint32_t value);
  bool pfsPutEOC(uint32_t cluster) {return pfsPut(cluster, 0x0FFFFFFF);}
  uint32_t clusterStartBlock(uint32_t cluster) const;
  uint8_t blockOfCluster(uint32_t position) const {return (position >> 9) & (blocksPerCluster_ - 1);}
  bool freeChain(uint32_t cluster);
  bool isEOC(uint32_t cluster) const {return  cluster >= PFSEOC_MIN;}
  cache_t *cacheAddress() {return &cacheBuffer_;}
  uint32_t cacheBlockNumber() {return cacheBlockNumber_;}

  bool readBlock(uint32_t block, uint8_t* dst) {
    return sdCard_->readBlock(block, dst);}
  bool writeBlock(uint32_t block, const uint8_t* dst) {
    return sdCard_->writeBlock(block, dst);
  }

// use of static functions save a bit of flash - maybe not worth complexity
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