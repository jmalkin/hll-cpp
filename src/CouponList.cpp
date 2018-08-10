/*
 * Copyright 2018, Yahoo! Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "CouponList.hpp"
#include "CouponHashSet.hpp"
#include "HllUtil.hpp"
#include "IntArrayPairIterator.hpp"
#include "HllArray.hpp"

#include <iostream>
#include <cstring>
#include <algorithm>

namespace sketches {

CouponList::CouponList(const int lgConfigK, const TgtHllType tgtHllType, const CurMode curMode)
  : AbstractCoupons(lgConfigK, tgtHllType, curMode) {
    if (curMode == LIST) {
      lgCouponArrInts = sketches::LG_INIT_LIST_SIZE;
      oooFlag = false;
    } else { // curMode == SET
      lgCouponArrInts = sketches::LG_INIT_SET_SIZE;
      oooFlag = true;
    }
    const int arrayLen = 1 << lgCouponArrInts;
    couponIntArr = new int[arrayLen];
    std::fill(couponIntArr, couponIntArr + arrayLen, 0);
    couponCount = 0;
}

CouponList::CouponList(const CouponList& that)
  : AbstractCoupons(that.lgConfigK, that.tgtHllType, that.curMode),
    lgCouponArrInts(that.lgCouponArrInts),
    couponCount(that.couponCount),
    oooFlag(that.oooFlag) {

  const int numItems = 1 << lgCouponArrInts;
  couponIntArr = new int[numItems];
  std::copy(that.couponIntArr, that.couponIntArr + numItems, couponIntArr);
}

CouponList::CouponList(const CouponList& that, const TgtHllType tgtHllType)
  : AbstractCoupons(that.lgConfigK, tgtHllType, that.curMode),
    lgCouponArrInts(that.lgCouponArrInts),
    couponCount(that.couponCount),
    oooFlag(that.oooFlag) {

  const int numItems = 1 << lgCouponArrInts;
  couponIntArr = new int[numItems];
  std::copy(that.couponIntArr, that.couponIntArr + numItems, couponIntArr);
}

CouponList::~CouponList() {
  delete couponIntArr;
}

CouponList* CouponList::copy() {
  return new CouponList(*this);
}

CouponList* CouponList::copyAs(const TgtHllType tgtHllType) {
  return new CouponList(*this, tgtHllType);
}

HllSketchImpl* CouponList::couponUpdate(int coupon) {
  const int len = 1 << lgCouponArrInts;
  for (int i = 0; i < len; ++i) { // search for empty slot
    const int couponAtIdx = couponIntArr[i];
    if (couponAtIdx == sketches::EMPTY) {
      couponIntArr[i] = coupon; // the actual update
      ++couponCount;
      if (couponCount >= len) { // array full
        if (lgConfigK < 8) {
          return promoteHeapListOrSetToHll(*this); // oooFlag = false
        }
        return promoteHeapListToSet(*this); // oooFlag = true;
      }
      return this;
    }
    // cell not empty
    if (couponAtIdx == coupon) {
      return this; // duplicate
    }
    // cell not empty and not a duplicate, continue
  }
  throw std::runtime_error("Array invalid: no empties and no duplicates");
}

int CouponList::getCouponCount() {
  return couponCount;
}

int CouponList::getCompactSerializationBytes() {
  return getMemDataStart() + (couponCount << 2);
}

int CouponList::getMemDataStart() {
  return sketches::LIST_INT_ARR_START;
}

int CouponList::getPreInts() {
  return sketches::LIST_PREINTS;
}

bool CouponList::isCompact() { return false; }

bool CouponList::isOutOfOrderFlag() { return oooFlag; }

void CouponList::putOutOfOrderFlag(bool oooFlag) {
  this->oooFlag = oooFlag;
}

CouponList* CouponList::reset() {
  return new CouponList(lgConfigK, tgtHllType, LIST);
}

int CouponList::getLgCouponArrInts() {
  return lgCouponArrInts;
}

int* CouponList::getCouponIntArr() {
  return couponIntArr;
}

std::unique_ptr<PairIterator> CouponList::getIterator() {
  PairIterator* itr = new IntArrayPairIterator(couponIntArr, 1 << lgCouponArrInts, lgConfigK);
  return std::move(std::unique_ptr<PairIterator>(itr));
}

HllSketchImpl* CouponList::promoteHeapListToSet(CouponList& list) {
  const int couponCount = list.couponCount;
  const int* arr = list.couponIntArr;
  CouponHashSet* chSet = new CouponHashSet(list.lgConfigK, list.tgtHllType);
  for (int i = 0; i < couponCount; ++i) {
    chSet->couponUpdate(arr[i]);
  }
  chSet->putOutOfOrderFlag(true);

  return chSet;
}

HllSketchImpl* CouponList::promoteHeapListOrSetToHll(CouponList& src) {
  HllArray* tgtHllArr = HllArray::newHll(src.lgConfigK, src.tgtHllType);
  std::unique_ptr<PairIterator> srcItr = src.getIterator();
  tgtHllArr->putKxQ0(1 << src.lgConfigK);
  while (srcItr->nextValid()) {
    tgtHllArr->couponUpdate(srcItr->getPair());
    tgtHllArr->putHipAccum(src.getEstimate());
  }
  tgtHllArr->putOutOfOrderFlag(false);
  return tgtHllArr;
}


}


