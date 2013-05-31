#include <SdVolume.h>
// macro for debug
#define DBG_FAIL_MACRO  //  Serial.print(__FILE__);Serial.println(__LINE__)

cache_t  SdVolume::cacheBuffer_;       // 512 byte cache for Sd2Card
uint32_t SdVolume::cacheBlockNumber_;  // current block number
uint8_t  SdVolume::cacheStatus_;       // status of cache block
uint32_t SdVolume::cachePfsOffset_;    // offset for mirrored PFS
#if USE_SEPARATE_PFS_CACHE
cache_t  SdVolume::cachePfsBuffer_;       // 512 byte cache for FAT
uint32_t SdVolume::cachePfsBlockNumber_;  // current Fat block number
uint8_t  SdVolume::cachePfsStatus_;       // status of cache Fatblock
#endif  // USE_SEPARATE_FAT_CACHE
Sd2Card* SdVolume::sdCard_;            // pointer to SD card object
//------------------------------

bool SdVolume::pfsGet(uint32_t cluster, uint32_t* value) {
  uint32_t lba;
  cache_t* pc;
  // error if reserved cluster of beyond FAT
  if (cluster < 2  || cluster > (clusterCount_ + 1)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  lba = pfsStartBlock_ + (cluster >> 7);
  pc = cacheFetchPfs(lba, CACHE_FOR_READ);
  if (!pc) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  *value = pc->fat32[cluster & 0X7F] & PFSMASK;
  
  return true;

 fail:
  return false;
}

bool SdVolume::pfsPut(uint32_t cluster, uint32_t value) {
  #if ENABLED_READ_ONLY
  return true;
  #else
  uint32_t lba;
  cache_t* pc;
  if (cluster < 2 || cluster > (clusterCount_ + 1)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  lba = pfsStartBlock_ + (cluster >> 7);

  pc = cacheFetchPfs(lba, CACHE_FOR_WRITE);
  if (!pc) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  pc->fat32[cluster & 0X7F] = value;
  
  return true;

 fail:
  return false;
  #endif  //ENABLED_READ_ONLY
}

bool SdVolume::allocContiguous(uint32_t count, uint32_t* curCluster) {
  // start of group
  uint32_t bgnCluster;
  // end of group
  uint32_t endCluster;
  // last cluster of FAT
  uint32_t fatEnd = clusterCount_ + 1;

  // flag to save place to start next search
  bool setStart;

  // set search start cluster
  if (*curCluster) {
    // try to make file contiguous
    bgnCluster = *curCluster + 1;

    // don't save new start location
    setStart = false;
  } else {
    // start at likely place for free cluster
    bgnCluster = allocSearchStart_;

    // save next search start if one cluster
    setStart = count == 1;
  }
  // end of group
  endCluster = bgnCluster;

  // search the FAT for free clusters
  for (uint32_t n = 0;; n++, endCluster++) {
    // can't find space checked all clusters
    if (n >= clusterCount_) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // past end - start from beginning of FAT
    if (endCluster > fatEnd) {
      bgnCluster = endCluster = 2;
    }
    uint32_t f;
    if (!pfsGet(endCluster, &f)) {
      DBG_FAIL_MACRO;
      goto fail;
    }

    if (f != 0) {
      // cluster in use try next cluster as bgnCluster
      bgnCluster = endCluster + 1;
    } else if ((endCluster - bgnCluster + 1) == count) {
      // done - found space
      break;
    }
  }
  // mark end of chain
  if (!pfsPutEOC(endCluster)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // link clusters
  while (endCluster > bgnCluster) {
    if (!pfsPut(endCluster - 1, endCluster)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    endCluster--;
  }
  if (*curCluster != 0) {
    // connect chains
    if (!pfsPut(*curCluster, bgnCluster)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // return first cluster number to caller
  *curCluster = bgnCluster;

  // remember possible next free cluster
  if (setStart) allocSearchStart_ = bgnCluster + 1;

  return true;

 fail:
  return false;
}

uint32_t SdVolume::clusterStartBlock(uint32_t cluster) const {
  return dataStartBlock_ + ((cluster - 2)*blocksPerCluster_);
}

// free a cluster chain
bool SdVolume::freeChain(uint32_t cluster) {
  #if ENABLED_READ_ONLY
  return true;
  #else
  uint32_t next;

  // clear free cluster location
  allocSearchStart_ = 2;

  do {
    if (!pfsGet(cluster, &next)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // free cluster
    if (!pfsPut(cluster, 0)) {
      DBG_FAIL_MACRO;
      goto fail;
    }

    cluster = next;
  } while (!isEOC(cluster));

  return true;

 fail:
  return false;
  #endif
}

int32_t SdVolume::freeClusterCount() {
  uint32_t free = 0;
  uint32_t lba;
  uint32_t todo = clusterCount_ + 2;
  uint16_t n;

  lba = pfsStartBlock_;
  while (todo) {
    cache_t* pc = cacheFetchPfs(lba++, CACHE_FOR_READ);
    if (!pc) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    n = 128;
    if (todo < n) n = todo;
    for (uint16_t i = 0; i < n; i++) {
      if (pc->fat32[i] == 0) free++;
    }    
    todo -= n;
  }
  return free;

 fail:
  return -1;
}

#if !USE_SEPARATE_PFS_CACHE

cache_t* SdVolume::cacheFetchPfs(uint32_t blockNumber, uint8_t options) {
  return cacheFetch(blockNumber, options | CACHE_STATUS_PFS_BLOCK);
}

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

bool SdVolume::cacheSync() {
  #if ENABLED_READ_ONLY
  return true;
  #else
  if (cacheStatus_ & CACHE_STATUS_DIRTY) {
    if (!sdCard_->writeBlock(cacheBlockNumber_, cacheBuffer_.data)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    cacheStatus_ &= ~CACHE_STATUS_DIRTY;
  }
  return true;

 fail:
  return false;
 #endif  // ENABLED_READ_ONLY
}

bool SdVolume::cacheWriteData() {
  return cacheSync();
}

#else

cache_t* SdVolume::cacheFetch(uint32_t blockNumber, uint8_t options) {
  return cacheFetchData(blockNumber, options);
}

cache_t* SdVolume::cacheFetchData(uint32_t blockNumber, uint8_t options) {
  if (cacheBlockNumber_ != blockNumber) {
    if (!cacheWriteData()) {
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

cache_t* SdVolume::cacheFetchPfs(uint32_t blockNumber, uint8_t options) {
  if (cachePfsBlockNumber_ != blockNumber) {
    if (!cacheWritePfs()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (!(options & CACHE_OPTION_NO_READ)) {
      if (!sdCard_->readBlock(blockNumber, cachePfsBuffer_.data)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    cachePfsStatus_ = 0;
    cachePfsBlockNumber_ = blockNumber;
  }
  cachePfsStatus_ |= options & CACHE_STATUS_MASK;
  return &cachePfsBuffer_;

 fail:
  return 0;
}

bool SdVolume::cacheWriteData() {
  #if ENABLED_READ_ONLY
  return true;
  #else
  if (cacheStatus_ & CACHE_STATUS_DIRTY) {
    if (!sdCard_->writeBlock(cacheBlockNumber_, cacheBuffer_.data)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    cacheStatus_ &= ~CACHE_STATUS_DIRTY;
  }
  return true;

 fail:
  return false;
  #endif  //ENABLED_READ_ONLY
}

bool SdVolume::cacheWritePfs() {
  #if ENABLED_READ_ONLY
  return true;
  #else
  if (cachePfsStatus_ & CACHE_STATUS_DIRTY) {
    if (!sdCard_->writeBlock(cachePfsBlockNumber_, cachePfsBuffer_.data)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // mirror second FAT
    if (cachePfsOffset_) {
      uint32_t lbn = cachePfsBlockNumber_ + cachePfsOffset_;
      if (!sdCard_->writeBlock(lbn, cachePfsBuffer_.data)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    cachePfsStatus_ &= ~CACHE_STATUS_DIRTY;
  }
  return true;

 fail:
  return false;
  #endif  // ENABLED_READ_ONLY
}

bool SdVolume::cacheSync() {
  return cacheWriteData() && cacheWritePfs();
}

#endif // USE_SEPARATE_PFS_CACHE

void SdVolume::cacheInvalidate() {
  #if !ENABLED_READ_ONLY
    cacheBlockNumber_ = 0XFFFFFFFF;
    cacheStatus_ = 0;
  #endif
}

bool SdVolume::init(Sd2Card* dev, uint8_t part) {
  uint32_t totalBlocks;
  uint32_t volumeStartBlock = 0;
  pfs_boot_t* pbs;
  cache_t* pc;
  sdCard_ = dev;
  allocSearchStart_ = 2;
  cacheStatus_ = 0; 
  cacheBlockNumber_ = 0XFFFFFFFF;
#if USE_SEPARATE_PFS_CACHE
  cachePfsStatus_ = 0;  
  cachePfsBlockNumber_ = 0XFFFFFFFF;
#endif  // USE_SEPARATE_PFS_CACHE
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
    if ((p->boot & 0X7F) !=0  || p->totalSectors < 100 || p->firstSector == 0) {
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
      DBG_FAIL_MACRO;
      goto fail;
  }
  pfsCount_ = 1;
  blocksPerCluster_ = pbs->sectorsPerCluster;
  
  // determine shift that is same as multiply by blocksPerCluster_
  clusterSizeShift_ = 0;
  while (blocksPerCluster_ != (1 << clusterSizeShift_)) {
    // error if not power of 2
    if (clusterSizeShift_++ > 7) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }

  sectorsPerPfs_ = 1;

  if (pfsCount_ > 0) cachePfsOffset_ = sectorsPerPfs_;
  pfsStartBlock_ = volumeStartBlock + 1;

  // directory start for PFS
  rootDirStart_ = pfsStartBlock_ + pfsCount_ * sectorsPerPfs_;

  // data start for PFS
  dataStartBlock_ = rootDirStart_ + ((32 * pbs->rootDirEntryCount + 511)/512);
  
  totalBlocks = pbs->totalSectors;
  
  // total data blocks
  clusterCount_ = totalBlocks - (dataStartBlock_ - volumeStartBlock);

  // divide by cluster size to get cluster count
  clusterCount_ >>= clusterSizeShift_;
  rootDirEntryCount_ = pbs->rootDirEntryCount;

  return true;

 fail:
  return false;
}