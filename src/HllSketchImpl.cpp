/*
 * Copyright 2018, Yahoo! Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "HllSketchImpl.hpp"

namespace sketches {

HllSketchImpl::HllSketchImpl(const int lgConfigK, const TgtHllType tgtHllType, const CurMode curMode)
  : lgConfigK(lgConfigK),
    tgtHllType(tgtHllType),
    curMode(curMode)
{}

HllSketchImpl::~HllSketchImpl() {}

TgtHllType HllSketchImpl::getTgtHllType() {
  return tgtHllType;
}

int HllSketchImpl::getLgConfigK() {
  return lgConfigK;
}

CurMode HllSketchImpl::getCurrentMode() {
  return curMode;
}


}
