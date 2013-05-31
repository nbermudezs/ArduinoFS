#ifndef SdPfsConfig_h
#define SdPfsConfig_h
#include <stdint.h>

#ifdef __arm__
#define USE_SEPARATE_PFS_CACHE 1
#else  // __arm__ Arduino Uno es AVR asi que no usara otra cache!
#define USE_SEPARATE_PFS_CACHE 0	
#endif  // __arm__
//------------------------------------------------------------------------------
/**
 * Don't use mult-block read/write on small AVR boards
 */
#if defined(RAMEND) && RAMEND < 3000
#define USE_MULTI_BLOCK_SD_IO 0
#else
#define USE_MULTI_BLOCK_SD_IO 1
#endif
//------------------------------------------------------------------------------
/**
 *  If set to 1 use just read methods for SD on Arduino, else check for USE_MEDIUM_API
 */
#define USE_SMALL_API 1
 /**
 *  If set to 1 use just read+write methods for SD on Arduino, else FULL API is used
 */
#define USE_MEDIUM_API 0
/**
 *  Force use of Arduino Standard SPI library if USE_ARDUINO_SPI_LIBRARY
 * is nonzero.
 */
#define USE_ARDUINO_SPI_LIBRARY 0
//------------------------------------------------------------------------------
/**
 * To enable SD card CRC checking set USE_SD_CRC nonzero.
 *
 * Set USE_SD_CRC to 1 to use a smaller slower CRC-CCITT function.
 *
 * Set USE_SD_CRC to 2 to used a larger faster table driven CRC-CCITT function.
 */
#define USE_SD_CRC 0
//------------------------------------------------------------------------------
/**
 * Set DESTRUCTOR_CLOSES_FILE nonzero to close a file in its destructor.
 *
 * Causes use of lots of heap in ARM.
 */
#define DESTRUCTOR_CLOSES_FILE 0
//------------------------------------------------------------------------------
/**
 * For AVR
 *
 * Set nonzero to use Serial (the HardwareSerial class) for error messages
 * and output from print functions like ls().
 *
 * If USE_SERIAL_FOR_STD_OUT is zero, a small non-interrupt driven class
 * is used to output messages to serial port zero.  This allows an alternate
 * Serial library like SerialPort to be used with SdFat.
 *
 * You can redirect stdOut with SdFat::setStdOut(Print* stream) and
 * get the current stream with SdFat::stdOut().
 */
#define USE_SERIAL_FOR_STD_OUT 0
//------------------------------------------------------------------------------
/**
 * Call flush for endl if ENDL_CALLS_FLUSH is nonzero
 *
 * The standard for iostreams is to call flush.  This is very costly for
 * SdFat.  Each call to flush causes 2048 bytes of I/O to the SD.
 *
 * SdFat has a single 512 byte buffer for SD I/O so it must write the current
 * data block to the SD, read the directory block from the SD, update the
 * directory entry, write the directory block to the SD and read the data
 * block back into the buffer.
 *
 * The SD flash memory controller is not designed for this many rewrites
 * so performance may be reduced by more than a factor of 100.
 *
 * If ENDL_CALLS_FLUSH is zero, you must call flush and/or close to force
 * all data to be written to the SD.
 */
#define ENDL_CALLS_FLUSH 0
/**
 * SPI init rate for SD initialization commands. Must be 10 (F_CPU/64)
 * or greater
 */
#define SPI_SD_INIT_RATE 11
 /**
 * Set USE_SOFTWARE_SPI nonzero to always use software SPI on AVR.
 */
#define USE_SOFTWARE_SPI 0
 /**
 * Set USE_PFS_BITMAP nonzero to load to memory the bitmap region.
 * Not recommended on embeded arch.
 */
#define USE_PFS_BITMAP 0

#define ENABLED_READ_ONLY 0 //luego lo cambio, esto es solo para pruebas

#define USING_APP 0

// define software SPI pins so Mega can use unmodified 168/328 shields
/** Default Software SPI chip select pin */
uint8_t const SOFT_SPI_CS_PIN = 10;
/** Software SPI Master Out Slave In pin */
uint8_t const SOFT_SPI_MOSI_PIN = 11;
/** Software SPI Master In Slave Out pin */
uint8_t const SOFT_SPI_MISO_PIN = 12;
/** Software SPI Clock pin */
uint8_t const SOFT_SPI_SCK_PIN = 13;

// values for type_
/** This file has not been opened. */
uint8_t const PFS_FILE_TYPE_CLOSED = 0;
/** A normal file */
uint8_t const PFS_FILE_TYPE_NORMAL = 1;
/** A FAT12 or FAT16 root directory */
uint8_t const PFS_FILE_TYPE_ROOT_FIXED = 2;
/** A FAT32 root directory */
uint8_t const PFS_FILE_TYPE_ROOT32 = 3;
/** A subdirectory file*/
uint8_t const PFS_FILE_TYPE_SUBDIR = 4;
/** Test value for directory type */
uint8_t const PFS_FILE_TYPE_MIN_DIR = PFS_FILE_TYPE_ROOT_FIXED;

/** ls() flag to print file size */
uint8_t const LS_SIZE = 2;
/** ls() flag for recursive list of subdirectories */
uint8_t const LS_R = 4;

//open flags
/** open() oflag for reading */
uint8_t const O_READ = 0X01;
/** open() oflag - same as O_IN */
uint8_t const O_RDONLY = O_READ;
/** open() oflag for write */
uint8_t const O_WRITE = 0X02;
/** open() oflag - same as O_WRITE */
uint8_t const O_WRONLY = O_WRITE;
/** open() oflag for reading and writing */
uint8_t const O_RDWR = (O_READ | O_WRITE);
/** open() oflag mask for access modes */
uint8_t const O_ACCMODE = (O_READ | O_WRITE);
/** create the file if nonexistent */
uint8_t const O_CREAT = 0X40;

//seek constants
uint8_t const SEEK_BEG_ = 1;
uint8_t const SEEK_CUR_ = 2;
uint8_t const SEEK_END_ = 4;

#endif  //SdPfsConfig_h