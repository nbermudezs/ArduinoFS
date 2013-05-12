#include <SdVolume.h>
// macro for debug
#define DBG_FAIL_MACRO  //  Serial.print(__FILE__);Serial.println(__LINE__)

#if !USE_MULTIPLE_CARDS
// raw block cache

cache_t  SdVolume::cacheBuffer_;       // 512 byte cache for Sd2Card
uint32_t SdVolume::cacheBlockNumber_;  // current block number
uint8_t  SdVolume::cacheStatus_;       // status of cache block
uint32_t SdVolume::cachePfsOffset_;    // offset for mirrored PFS
Sd2Card* SdVolume::sdCard_;            // pointer to SD card object
#endif  // USE_MULTIPLE_CARDS
//------------------------------

#if !USE_SEPARATE_PFS_CACHE
cache_t* SdVolume::cacheFetch(uint32_t blockNumber, uint8_t options) {
  if (cacheBlockNumber_ != blockNumber) {
    if (!cacheSync()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (!(options & CACHE_OPTION_NO_READ)) {
      if (!sdCard_->readBlock(blockNumber, cacheBuffer_.data)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    cacheStatus_ = 0;
    cacheBlockNumber_ = blockNumber;
  }
  cacheStatus_ |= options & CACHE_STATUS_MASK;
  return &cacheBuffer_;

 fail:
  return 0;
}
#endif // USE_SEPARATE_PFS_CACHE
//------------------------------------------------------------------------------
/** Initialize a FAT volume.
 *
 * \param[in] dev The SD card where the volume is located.
 *
 * \param[in] part The partition to be used.  Legal values for \a part are
 * 1-4 to use the corresponding partition on a device formatted with
 * a MBR, Master Boot Record, or zero if the device is formatted as
 * a super floppy with the FAT boot sector in block zero.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.  Reasons for
 * failure include not finding a valid partition, not finding a valid
 * FAT file system in the specified partition or an I/O error.
 */
bool SdVolume::init(Sd2Card* dev, uint8_t part) {
  uint32_t totalBlocks;
  uint32_t volumeStartBlock = 0;
  pfs_boot_t* pbs;
  cache_t* pc;
  sdCard_ = dev;
  allocSearchStart_ = 2;
  cacheStatus_ = 0;  // cacheSync() will write block if true
  cacheBlockNumber_ = 0XFFFFFFFF;
  // if part == 0 assume super floppy with FAT boot sector in block zero
  // if part > 0 assume mbr volume with partition table
  if (part) {
    if (part > 4) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    pc = cacheFetch(volumeStartBlock, CACHE_FOR_READ);
    if (!pc) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    part_t* p = &pc->mbr.part[part-1];
    if ((p->boot & 0X7F) !=0  || //porque es 0X7F??? de donde salio
      p->totalSectors < 100 ||
      p->firstSector == 0) {
      // not a valid partition
      DBG_FAIL_MACRO;
      goto fail;
    }
    volumeStartBlock = p->firstSector;
  }
  pc = cacheFetch(volumeStartBlock, CACHE_FOR_READ);
  if (!pc) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  pbs = &(pc->pbs);
  if (pbs->bytesPerSector != 512) {
       // not valid FAT volume
      DBG_FAIL_MACRO;
      goto fail;
  }
  pfsCount_ = pbs->pfsCount;
  blocksPerCluster_ = 1;

  sectorsPerPfs_ = pbs->sectorsPerPfs;

  if (pfsCount_ > 0) cachePfsOffset_ = sectorsPerPfs;
  pfsStartBlock_ = volumeStartBlock + pbs->reservedSectorCount;

  rootDirEntryMax_ = pbs->rootDirEntryMax;

  // directory start for PFS
  rootDirStart_ = pfsStartBlock_ + pbs->pfsCount * sectorsPerPfs_;

  // data start for PFS
  dataStartBlock_ = rootDirStart_ + ((sizeof(dir_t) * pbs->rootDirEntryCount + 511)/512);
  
  // total data blocks
  clusterCount_ = totalBlocks - (dataStartBlock_ - volumeStartBlock);

  // divide by cluster size to get cluster count
  clusterCount_ >>= clusterSizeShift_;

  rootDirStart_ = pbs->fat32RootCluster;

  return true;

 fail:
  return false;
}