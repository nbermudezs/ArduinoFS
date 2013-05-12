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
#include <SdFat.h>
// macro for debug
#define DBG_FAIL_MACRO  //  Serial.print(__FILE__);Serial.println(__LINE__)
//------------------------------------------------------------------------------
// pointer to cwd directory
SdBaseFile* SdBaseFile::cwd_ = 0;

//------------------------------------------------------------------------------
void SdBaseFile::getpos(PfsPos_t* pos) {
  pos->position = curPosition_;
  pos->cluster = curCluster_;
}

//------------------------------------------------------------------------------
void SdBaseFile::setpos(PfsPos_t* pos) {
  pos->position = curPosition_;
  pos->cluster = curCluster_;
}