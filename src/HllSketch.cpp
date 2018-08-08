/*
 * Copyright 2018, Yahoo! Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>

#include "HllUtil.hpp"
#include "CouponList.hpp"
#include "HllArray.hpp"
#include "HllSketch.hpp"

using std::endl;

namespace sketches {

HllSketch::HllSketch(const int lgConfigK, const TgtHllType tgtHllType) {
  hllSketchImpl = new CouponList(sketches::checkLgK(lgConfigK), tgtHllType, LIST);
}

HllSketch::~HllSketch() {
  delete hllSketchImpl;
}

HllSketch::HllSketch(const HllSketch& that) {
  hllSketchImpl = that.hllSketchImpl->copy();
}

HllSketch::HllSketch(HllSketchImpl* that) {
  hllSketchImpl = that;
}

void HllSketch::reset() {
  hllSketchImpl = hllSketchImpl->reset();
}

void HllSketch::couponUpdate(int coupon) {
  if (coupon == sketches::EMPTY) { return; }
  HllSketchImpl* result = this->hllSketchImpl->couponUpdate(coupon);
  if (result != this->hllSketchImpl) {
    delete this->hllSketchImpl;
    this->hllSketchImpl = result;
  }
}

void dump_sketch(HllSketch& sketch, const bool all) {
  //std::ostringstream oss;
  //sketch.to_string(oss, true, true, true, all);
  sketch.to_string(std::cout, true, true, true, all);
  //fprintf(stdout, "%s\n", oss.str().c_str());
}

std::ostream& operator<<(std::ostream& os, HllSketch& sketch) {
  return sketch.to_string(os, true, true, false, false);
}

std::ostream& HllSketch::to_string(std::ostream& os, const bool summary, const bool detail, const bool auxDetail, const bool all) {
  if (summary) {
    os << "### HLL SKETCH SUMMARY: " << endl
       << "  Log Config K   : " << getLgConfigK() << endl
       << "  Hll Target     : " << type_as_string() << endl
       << "  Current Mode   : " << mode_as_string() << endl
       << "  LB             : " << getLowerBound(1) << endl
       << "  Estimate       : " << getEstimate() << endl
       << "  UB             : " << getUpperBound(1) << endl
       << "  OutOfOrder flag: " << isOutOfOrderFlag() << endl;
    if (getCurrentMode() == HLL) {
      HllArray* hllArray = (HllArray*) hllSketchImpl;
      os << "  CurMin       : " << hllArray->getCurMin() << endl
         << "  NumAtCurMin  : " << hllArray->getNumAtCurMin() << endl
         << "  HipAccum     : " << hllArray->getHipAccum() << endl
         << "  KxQ0         : " << hllArray->getKxQ0() << endl
         << "  KxQ1         : " << hllArray->getKxQ1() << endl;
    } else {
      os << "  Coupon count : " << std::to_string(((AbstractCoupons*) hllSketchImpl)->getCouponCount()) << endl;
    }
  }

  if (detail) {
    os << "### HLL SKETCH DATA DETAIL: " << endl;
    PairIterator* pitr = getIterator();
    os << pitr->getHeader() << endl;
    if (all) {
      while (pitr->nextAll()) {
        os << pitr->getString() << endl;
      }
    } else {
      while (pitr->nextValid()) {
        os << pitr->getString() << endl;
      }
    }
    delete pitr;
  }
  if (auxDetail) {
    if ((getCurrentMode() == HLL) && (getTgtHllType() == HLL_4)) {
      HllArray* hllArray = (HllArray*) hllSketchImpl;
      PairIterator* auxItr = hllArray->getAuxIterator();
      if (auxItr != NULL) {
        os << "### HLL SKETCH AUX DETAIL: " << std::endl
           << auxItr->getHeader() << std::endl;
        if (all) {
          while (auxItr->nextAll()) {
            os << auxItr->getString() << std::endl;
          }
        } else {
          while (auxItr->nextAll()) {
            os << auxItr->getString() << std::endl;
          }
        }
      }
    }
  }

  return os;
}

double HllSketch::getEstimate() {
  return hllSketchImpl->getEstimate();
}

double HllSketch::getCompositeEstimate() {
  return hllSketchImpl->getCompositeEstimate();
}

double HllSketch::getLowerBound(int numStdDev) {
  return hllSketchImpl->getLowerBound(numStdDev);
}

double HllSketch::getUpperBound(int numStdDev) {
  return hllSketchImpl->getUpperBound(numStdDev);
}

CurMode HllSketch::getCurrentMode() {
  return hllSketchImpl->getCurrentMode();
}

int HllSketch::getLgConfigK() {
  return hllSketchImpl->getLgConfigK();
}

TgtHllType HllSketch::getTgtHllType() {
  return hllSketchImpl->getTgtHllType();
}

bool HllSketch::isOutOfOrderFlag() {
  return hllSketchImpl->isOutOfOrderFlag();
}

int HllSketch::getUpdatableSerializationBytes() {
  return hllSketchImpl->getUpdatableSerializationBytes();
}

int HllSketch::getCompactSerializationBytes() {
  return hllSketchImpl->getCompactSerializationBytes();
}

bool HllSketch::isCompact() {
  return hllSketchImpl->isCompact();
}

bool HllSketch::isEmpty() {
  return hllSketchImpl->isEmpty();
}

PairIterator* HllSketch::getIterator() {
  return hllSketchImpl->getIterator();
}

std::string HllSketch::type_as_string() {
  switch (hllSketchImpl->getTgtHllType()) {
    case TgtHllType::HLL_4:
      return std::string("HLL_4");
    case TgtHllType::HLL_8:
      return std::string("HLL_8");
    default:
      throw std::runtime_error("Sketch state error: Invalid TgtHllType");
  }
}

std::string HllSketch::mode_as_string() {
  switch (hllSketchImpl->getCurrentMode()) {
    case LIST:
      return std::string("LIST");
    case SET:
      return std::string("SET");
    case HLL:
      return std::string("HLL");
    default:
      throw std::runtime_error("Sketch state error: Invalid CurMode");
  }
}

}
