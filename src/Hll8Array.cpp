/*
 * Copyright 2018, Yahoo! Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include <cstring>

#include "Hll8Array.hpp"

namespace sketches {

Hll8Iterator::Hll8Iterator(Hll8Array& hllArray, const int lengthPairs)
  : HllPairIterator(lengthPairs),
    hllArray(hllArray)
{}

Hll8Iterator::~Hll8Iterator() { }

int Hll8Iterator::value() {
  // we know it must be an Hll8Array so the cast is safe
  return ((Hll8Array) hllArray).hllByteArr[index] & VAL_MASK_6;
}

Hll8Array::Hll8Array(const int lgConfigK) :
    HllArray(lgConfigK, HLL_8) {
  const int numBytes = hll8ArrBytes(lgConfigK);
  hllByteArr = new uint8_t[numBytes];
  std::fill(hllByteArr, hllByteArr + numBytes, 0);
}

Hll8Array::Hll8Array(Hll8Array& that) :
  HllArray(that)
{
  const int numBytes = hll8ArrBytes(lgConfigK);
  hllByteArr = new uint8_t[numBytes];
  std::copy(that.hllByteArr, that.hllByteArr + numBytes, hllByteArr);
}

Hll8Array::~Hll8Array() {
  // hllByteArr deleted in parent
}

Hll8Array* Hll8Array::copy() {
  return new Hll8Array(*this);
}

PairIterator* Hll8Array::getIterator() {
  return new Hll8Iterator(*this, 1 << lgConfigK);
}

int Hll8Array::getSlot(const int slotNo) {
  return (int) hllByteArr[slotNo] & VAL_MASK_6;
}

void Hll8Array::putSlot(const int slotNo, const int value) {
  hllByteArr[slotNo] = value & VAL_MASK_6;
}

int Hll8Array::getHllByteArrBytes() {
  return hll8ArrBytes(lgConfigK);
}

}

