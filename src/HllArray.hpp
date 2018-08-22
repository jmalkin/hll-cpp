/*
 * Copyright 2018, Yahoo! Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#pragma once

#include "HllSketchImpl.hpp"
#include "AuxHashMap.hpp"

namespace datasketches {

class HllArray : public HllSketchImpl {
  public:
    explicit HllArray(const int lgConfigK, const TgtHllType tgtHllType);
    explicit HllArray(HllArray& that);

    static HllArray* newHll(const int lgConfigK, const TgtHllType tgtHllType);

    virtual ~HllArray();

    virtual HllArray* copy() = 0;
    virtual HllArray* copyAs(const TgtHllType tgtHllType);

    virtual HllSketchImpl* couponUpdate(const int coupon);

    virtual double getEstimate();
    virtual double getCompositeEstimate();
    virtual double getLowerBound(const int numStdDev);
    virtual double getUpperBound(const int numStdDev);

    virtual HllSketchImpl* reset();

    void addToHipAccum(double delta);

    void decNumAtCurMin();

    virtual CurMode getCurMode();

    int getCurMin();
    int getNumAtCurMin();
    double getHipAccum();

    virtual int getHllByteArrBytes() = 0;

    virtual std::unique_ptr<PairIterator> getIterator() = 0;

    virtual std::unique_ptr<PairIterator> getAuxIterator();

    virtual int getUpdatableSerializationBytes();
    virtual int getCompactSerializationBytes();

    virtual bool isOutOfOrderFlag();
    virtual bool isEmpty();
    virtual bool isCompact();

    virtual void putOutOfOrderFlag(const bool flag);

    double getKxQ0();
    double getKxQ1();

    virtual int getMemDataStart();
    virtual int getPreInts();

    virtual void putSlot(const int slotNo, const int value) = 0;
    virtual int getSlot(int slotNo) = 0;

    void putCurMin(const int curMin);
    void putHipAccum(const double hipAccum);
    void putKxQ0(const double kxq0);
    void putKxQ1(const double kxq1);
    void putNumAtCurMin(const int numAtCurMin);

    static int hll4ArrBytes(const int lgConfigK);
    //static int hll6ArrBytes(const int lgConfigK);
    static int hll8ArrBytes(const int lgConfigK);

  protected:
    // TODO: does this need to be static?
    static void hipAndKxQIncrementalUpdate(HllArray& host, const int oldValue, const int newValue);
    double getHllBitMapEstimate(const int lgConfigK, const int curMin, const int numAtCurMin);
    double getHllRawEstimate(const int lgConfigK, const double kxqSum);
    virtual AuxHashMap* getAuxHashMap();

    double hipAccum;
    double kxq0;
    double kxq1;
    uint8_t* hllByteArr; //init by sub-classes
    int curMin; //always zero for Hll6 and Hll8, only used / tracked by Hll4Array
    int numAtCurMin; //interpreted as num zeros when curMin == 0
    bool oooFlag; //Out-Of-Order Flag

    friend class Conversions;
};


}
