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
#ifndef SdPfsConfig_h
#define SdPfsConfig_h
#include <stdint.h>
/**
 * Set USE_SEPARATE_PFS_CACHE nonzero to use a second 512 byte cache
 * for FAT table entries.  Improves performance for large writes that
 * are not a multiple of 512 bytes.
 */
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
 * To use multiple SD cards set USE_MULTIPLE_CARDS nonzero.
 *
 * Using multiple cards costs 400 - 500  bytes of flash.
 *
 * Each card requires about 550 bytes of SRAM so use of a Mega is recommended.
 */
#define USE_MULTIPLE_CARDS 0
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

// define software SPI pins so Mega can use unmodified 168/328 shields
/** Default Software SPI chip select pin */
uint8_t const SOFT_SPI_CS_PIN = 10;
/** Software SPI Master Out Slave In pin */
uint8_t const SOFT_SPI_MOSI_PIN = 11;
/** Software SPI Master In Slave Out pin */
uint8_t const SOFT_SPI_MISO_PIN = 12;
/** Software SPI Clock pin */
uint8_t const SOFT_SPI_SCK_PIN = 13;

#endif  //SdPfsConfig_h