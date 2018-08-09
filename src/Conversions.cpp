/*
 * Copyright 2018, Yahoo! Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "Conversions.hpp"
#include "HllArray.hpp"

namespace sketches {

Hll4Array* Conversions::convertToHll4(HllArray& srcHllArr) {
  const int lgConfigK = srcHllArr.getLgConfigK();
  Hll4Array* hll4Array = new Hll4Array(lgConfigK);
  hll4Array->putOutOfOrderFlag(srcHllArr.isOutOfOrderFlag());

  // 1st pass: compute starting curMin and numAtCurMin
  int pairVals = curMinAndNum(srcHllArr);
  int curMin = getValue(pairVals);
  int numAtCurMin = getLow26(pairVals);

  // 2nd pass: must know curMin.
  // Populate KxQ registers, build AuxHashMap if needed
  PairIterator* itr = srcHllArr.getIterator();
  // nothing allocated, may be null
  AuxHashMap* auxHashMap = srcHllArr.getAuxHashMap();

  while (itr->nextValid()) {
    const int slotNo = itr->getIndex();
    const int actualValue = itr->getValue();
    HllArray::hipAndKxQIncrementalUpdate(*hll4Array, 0, actualValue);
    if (actualValue >= (curMin + 15)) {
      hll4Array->putSlot(slotNo, AUX_TOKEN);
      if (auxHashMap == NULL) {
        auxHashMap = new AuxHashMap(LG_AUX_ARR_INTS[lgConfigK], lgConfigK);
        hll4Array->putAuxHashMap(auxHashMap);
      }
      auxHashMap->mustAdd(slotNo, actualValue);
    } else {
      hll4Array->putSlot(slotNo, actualValue - curMin);
    }
  }
  
  delete itr;

  hll4Array->putCurMin(curMin);
  hll4Array->putNumAtCurMin(numAtCurMin);
  hll4Array->putHipAccum(srcHllArr.getHipAccum());

  return hll4Array;
}

int Conversions::curMinAndNum(HllArray& hllArr) {
  int curMin = 64;
  int numAtCurMin = 0;
  PairIterator* itr = hllArr.getIterator();
  while (itr->nextAll()) {
    int v = itr->getValue();
    if (v < curMin) {
      curMin = v;
      numAtCurMin = 1;
    } else if (v == curMin) {
      ++numAtCurMin;
    }
  }
  delete itr;

  return pair(numAtCurMin, curMin);
}

Hll8Array* Conversions::convertToHll8(HllArray& srcHllArr) {
  const int lgConfigK = srcHllArr.getLgConfigK();
  Hll8Array* hll8Array = new Hll8Array(lgConfigK);
  hll8Array->putOutOfOrderFlag(srcHllArr.isOutOfOrderFlag());

  int numZeros = 1 << lgConfigK;
  PairIterator* itr = srcHllArr.getIterator();
  while (itr->nextAll()) {
    if (itr->getValue() != EMPTY) {
      --numZeros;
      hll8Array->couponUpdate(itr->getPair());
    }
  }
  delete itr;

  hll8Array->putNumAtCurMin(numZeros);
  hll8Array->putHipAccum(srcHllArr.getHipAccum());
  return hll8Array;
}

}