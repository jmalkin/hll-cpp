/*
 * Copyright 2018, Yahoo! Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "Union.hpp"

#include "HllSketchImpl.hpp"
#include "HllArray.hpp"
#include "HllUtil.hpp"

namespace sketches {

Union::Union(const int lgMaxK)
  : lgMaxK(checkLgK(lgMaxK)) {
  gadget = new HllSketch(lgMaxK, TgtHllType::HLL_8);
}

Union::Union(HllSketch& sketch)
  : lgMaxK(sketch.getLgConfigK()) {
  TgtHllType tgtHllType = sketch.getTgtHllType();
  if (tgtHllType != TgtHllType::HLL_8) {
    throw std::invalid_argument("Union can only wrap HLL_8 sketches");
  }
  gadget = &sketch;
}

Union::~Union() {
  if (gadget != nullptr) {
    delete gadget;
  }
}

HllSketch* Union::getResult() {
  return gadget->copyAs(TgtHllType::HLL_4);
}

HllSketch* Union::getResult(TgtHllType tgtHllType) {
  return gadget->copyAs(tgtHllType);
}

void Union::update(HllSketch* sketch) {
  unionImpl(sketch->hllSketchImpl, lgMaxK);
}

void Union::update(HllSketch& sketch) {
  unionImpl(sketch.hllSketchImpl, lgMaxK);
}

void Union::couponUpdate(const int coupon) {
  if (coupon == EMPTY) { return; }
  HllSketchImpl* result = gadget->hllSketchImpl->couponUpdate(coupon);
  if (result != gadget->hllSketchImpl) {
    if (gadget->hllSketchImpl != nullptr) { delete gadget->hllSketchImpl; }
    gadget->hllSketchImpl = result;
  }
}

std::ostream& Union::to_string(std::ostream& os, const bool summary,
                               const bool detail, const bool auxDetail, const bool all) {
  return gadget->to_string(os, summary, detail, auxDetail, all);
}

double Union::getEstimate() {
  return gadget->getEstimate();
}

double Union::getCompositeEstimate() {
  return gadget->getCompositeEstimate();
}

double Union::getLowerBound(const int numStdDev) {
  return gadget->getLowerBound(numStdDev);
}

double Union::getUpperBound(const int numStdDev) {
  return gadget->getUpperBound(numStdDev);
}

int Union::getCompactSerializationBytes() {
  return gadget->getCompactSerializationBytes();
}

int Union::getUpdatableSerializationBytes() {
  return gadget->getUpdatableSerializationBytes();
}

int Union::getLgConfigK() {
  return gadget->getLgConfigK();
}

void Union::reset() {
  gadget->reset();
}

bool Union::isCompact() {
  return gadget->isCompact();
}

bool Union::isEmpty() {
  return gadget->isEmpty();
}

bool Union::isOutOfOrderFlag() {
  return gadget->isOutOfOrderFlag();
}

CurMode Union::getCurMode() {
  return gadget->getCurMode();
}

TgtHllType Union::getTgtHllType() {
  return TgtHllType::HLL_8;
}

int Union::getMaxSerializationBytes(const int lgK) {
  return HllSketch::getMaxUpdatableSerializationBytes(lgK, TgtHllType::HLL_8);
}

HllSketchImpl* Union::copyOrDownsampleHll(HllSketchImpl* srcImpl, const int tgtLgK) {
  assert(srcImpl->getCurMode() == CurMode::HLL);
  HllArray* src = (HllArray*) srcImpl;
  const int srcLgK = src->getLgConfigK();
  if ((srcLgK <= tgtLgK) && (src->getTgtHllType() == TgtHllType::HLL_8)) {
    return src->copy();
  }
  const int minLgK = ((srcLgK < tgtLgK) ? srcLgK : tgtLgK);
  HllArray* tgtHllArr = HllArray::newHll(minLgK, TgtHllType::HLL_8);
  std::unique_ptr<PairIterator> srcItr = src->getIterator();
  while (srcItr->nextValid()) {
    tgtHllArr->couponUpdate(srcItr->getPair());
  }
  //both of these are required for isomorphism
  tgtHllArr->putHipAccum(src->getHipAccum());
  tgtHllArr->putOutOfOrderFlag(src->isOutOfOrderFlag());
  
  return tgtHllArr;
}

inline HllSketchImpl* Union::leakFreeCouponUpdate(HllSketchImpl* impl, const int coupon) {
  HllSketchImpl* result = impl->couponUpdate(coupon);
  if (result != impl) {
    delete impl;
  }
  return result;
}

void Union::unionImpl(HllSketchImpl* incomingImpl, const int lgMaxK) {
  assert(gadget->hllSketchImpl->getTgtHllType() == TgtHllType::HLL_8);
  HllSketchImpl* srcImpl = incomingImpl; //default
  HllSketchImpl* dstImpl = gadget->hllSketchImpl; //default
  if ((incomingImpl == nullptr) || incomingImpl->isEmpty()) {
    return; // gadget->hllSketchImpl;
  }

  const int hi2bits = (gadget->hllSketchImpl->isEmpty()) ? 3 : gadget->hllSketchImpl->getCurMode();
  const int lo2bits = incomingImpl->getCurMode();

  // TODO: track when we need to free the old gadget

  const int sw = (hi2bits << 2) | lo2bits;
  //System.out.println("SW: " + sw);
  switch (sw) {
    case 0: { //src: LIST, gadget: LIST
      std::unique_ptr<PairIterator> srcItr = srcImpl->getIterator(); //LIST
      while (srcItr->nextValid()) {
        dstImpl = leakFreeCouponUpdate(dstImpl, srcItr->getPair()); //assignment required
      }
      //whichever is True wins:
      dstImpl->putOutOfOrderFlag(dstImpl->isOutOfOrderFlag() | srcImpl->isOutOfOrderFlag());
      break;
    }
    case 1: { //src: SET, gadget: LIST
      //consider a swap here
      std::unique_ptr<PairIterator> srcItr = srcImpl->getIterator(); //SET
      while (srcItr->nextValid()) {
        dstImpl = leakFreeCouponUpdate(dstImpl, srcItr->getPair()); //assignment required
      }
      dstImpl->putOutOfOrderFlag(true); //SET oooFlag is always true
       break;
    }
    case 2: { //src: HLL, gadget: LIST
      //swap so that src is gadget-LIST, tgt is HLL
      //use lgMaxK because LIST has effective K of 2^26
      srcImpl = gadget->hllSketchImpl;
      dstImpl = copyOrDownsampleHll(incomingImpl, lgMaxK);
      std::unique_ptr<PairIterator> srcItr = srcImpl->getIterator();
      while (srcItr->nextValid()) {
        dstImpl = leakFreeCouponUpdate(dstImpl, srcItr->getPair()); //assignment required
      }
      //whichever is True wins:
      dstImpl->putOutOfOrderFlag(srcImpl->isOutOfOrderFlag() | dstImpl->isOutOfOrderFlag());
      break;
    }
    case 4: { //src: LIST, gadget: SET
      std::unique_ptr<PairIterator> srcItr = srcImpl->getIterator(); //LIST
      while (srcItr->nextValid()) {
        dstImpl = leakFreeCouponUpdate(dstImpl, srcItr->getPair()); //assignment required
      }
      dstImpl->putOutOfOrderFlag(true); //SET oooFlag is always true
      break;
    }
    case 5: { //src: SET, gadget: SET
      std::unique_ptr<PairIterator> srcItr = srcImpl->getIterator(); //SET
      while (srcItr->nextValid()) {
        dstImpl = leakFreeCouponUpdate(dstImpl, srcItr->getPair()); //assignment required
      }
      dstImpl->putOutOfOrderFlag(true); //SET oooFlag is always true
      break;
    }
    case 6: { //src: HLL, gadget: SET
      //swap so that src is gadget-SET, tgt is HLL
      //use lgMaxK because LIST has effective K of 2^26
      srcImpl = gadget->hllSketchImpl;
      dstImpl = copyOrDownsampleHll(incomingImpl, lgMaxK);
      std::unique_ptr<PairIterator> srcItr = srcImpl->getIterator(); //LIST
      assert(dstImpl->getCurMode() == HLL);
      while (srcItr->nextValid()) {
        dstImpl = leakFreeCouponUpdate(dstImpl, srcItr->getPair()); //assignment required
      }
      dstImpl->putOutOfOrderFlag(true); //merging SET into non-empty HLL -> true
      break;
    }
    case 8: { //src: LIST, gadget: HLL
      assert(dstImpl->getCurMode() == HLL);
      std::unique_ptr<PairIterator> srcItr = srcImpl->getIterator(); //LIST
      while (srcItr->nextValid()) {
        dstImpl = leakFreeCouponUpdate(dstImpl, srcItr->getPair()); //assignment required
      }
      //whichever is True wins:
      dstImpl->putOutOfOrderFlag(dstImpl->isOutOfOrderFlag() | srcImpl->isOutOfOrderFlag());
      break;
    }
    case 9: { //src: SET, gadget: HLL
      assert(dstImpl->getCurMode() == HLL);
      std::unique_ptr<PairIterator> srcItr = srcImpl->getIterator(); //SET
      while (srcItr->nextValid()) {
        dstImpl = leakFreeCouponUpdate(dstImpl, srcItr->getPair()); //assignment required
      }
      dstImpl->putOutOfOrderFlag(true); //merging SET into existing HLL -> true
      break;
    }
    case 10: { //src: HLL, gadget: HLL
      const int srcLgK = srcImpl->getLgConfigK();
      const int dstLgK = dstImpl->getLgConfigK();
      const int minLgK = ((srcLgK < dstLgK) ? srcLgK : dstLgK);
      if ((srcLgK < dstLgK) || (dstImpl->getTgtHllType() != HLL_8)) {
        dstImpl = copyOrDownsampleHll(dstImpl, minLgK); //TODO Fix for off-heap
      }
      std::unique_ptr<PairIterator> srcItr = srcImpl->getIterator(); //HLL
      while (srcItr->nextValid()) {
        dstImpl = leakFreeCouponUpdate(dstImpl, srcItr->getPair()); //assignment required
      }
      dstImpl->putOutOfOrderFlag(true); //union of two HLL modes is always true
      break;
    }
    case 12: { //src: LIST, gadget: empty
      std::unique_ptr<PairIterator> srcItr = srcImpl->getIterator(); //LIST
      while (srcItr->nextValid()) {
        dstImpl = leakFreeCouponUpdate(dstImpl, srcItr->getPair()); //assignment required
      }
      dstImpl->putOutOfOrderFlag(srcImpl->isOutOfOrderFlag()); //whatever source is
      break;
    }
    case 13: { //src: SET, gadget: empty
      std::unique_ptr<PairIterator> srcItr = srcImpl->getIterator(); //SET
      while (srcItr->nextValid()) {
        dstImpl = leakFreeCouponUpdate(dstImpl, srcItr->getPair()); //assignment required
      }
      dstImpl->putOutOfOrderFlag(true); //SET oooFlag is always true
      break;
    }
    case 14: { //src: HLL, gadget: empty
      dstImpl = copyOrDownsampleHll(srcImpl, lgMaxK);
      dstImpl->putOutOfOrderFlag(srcImpl->isOutOfOrderFlag()); //whatever source is.
      break;
    }
  }
  // replace annd free gadget, if necessary
  //return dstImpl;
  if (dstImpl != gadget->hllSketchImpl) {
    delete gadget->hllSketchImpl;
    gadget->hllSketchImpl = dstImpl;
  }
}


}