/*
 * Copyright 2018, Yahoo! Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#pragma once

#include <cassert>
#include <cmath>
#include <exception>
#include <string>
#include <sstream>

namespace sketches {

enum CurMode { LIST = 0, SET, HLL };

// preamble stuff
// Coupon List
const int LIST_INT_ARR_START = 8;
const int LIST_PREINTS = 2;
// Coupon Hash Set
const int HASH_SET_COUNT_INT             = 8;
const int HASH_SET_INT_ARR_START         = 12;
const int HASH_SET_PREINTS         = 3;
// HLL
const int HLL_PREINTS = 10;
const int HLL_BYTE_ARR_START = 40;


// other HllUtil stuff
const int KEY_BITS_26 = 26;
const int VAL_BITS_6 = 6;
const int KEY_MASK_26 = (1 << KEY_BITS_26) - 1;
const int VAL_MASK_6 = (1 << VAL_BITS_6) - 1;
const int EMPTY = 0;
const int MIN_LOG_K = 4;
const int MAX_LOG_K = 21;

const double HLL_HIP_RSE_FACTOR = sqrt(log(2.0)); //.8325546
const double HLL_NON_HIP_RSE_FACTOR = sqrt((3.0 * log(2.0)) - 1.0); //1.03896
const double COUPON_RSE_FACTOR = .409; //at transition point not the asymptote

const double COUPON_RSE = COUPON_RSE_FACTOR / (1 << 13);

const int LG_INIT_LIST_SIZE = 3;
const int LG_INIT_SET_SIZE = 5;
const int RESIZE_NUMER = 3;
const int RESIZE_DENOM = 4;

const int loNibbleMask = 0x0f;
const int hiNibbleMask = 0xf0;
const int AUX_TOKEN = 0xf;

/**
 * Log2 table sizes for exceptions based on lgK from 0 to 26.
 * However, only lgK from 4 to 21 are used.
 */
const int LG_AUX_ARR_INTS[] = {
    0, 2, 2, 2, 2, 2, 2, 3, 3, 3,   // 0 - 9
    4, 4, 5, 5, 6, 7, 8, 9, 10, 11, // 10-19
    12, 13, 14, 15, 16, 17, 18      // 20-26
    };

inline int checkLgK(const int lgK) {
  if ((lgK >= MIN_LOG_K) && (lgK <= MAX_LOG_K)) { return lgK; }
  std::stringstream ss;
  ss << "Invalid value of k: " << lgK;
  throw std::invalid_argument(ss.str());
}

inline void checkMemSize(const uint64_t minBytes, const uint64_t capBytes) {
  if (capBytes < minBytes) {
    std::stringstream ss;
    ss << "Given destination array is not large enough: " << capBytes;
    throw std::invalid_argument(ss.str());
  }
}

inline void checkNumStdDev(const int numStdDev) {
  if ((numStdDev < 1) || (numStdDev > 3)) {
    throw std::invalid_argument("NumStdDev may not be less than 1 or greater than 3.");
  }
}

inline int pair(const int slotNo, const int value) {
  return (value << KEY_BITS_26) | (slotNo & KEY_MASK_26);
}

inline int getLow26(const int coupon) { return coupon & KEY_MASK_26; }

inline int getValue(const int coupon) { return coupon >> KEY_BITS_26; }

inline double invPow2(const int e) {
  union {
    long long longVal;
    double doubleVal;
  } conv;
  conv.longVal = (1023L - e) << 52;
  return conv.doubleVal;
}

}
