/*
 * Copyright 2018, Yahoo! Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#pragma once

#include "BaseHllSketch.hpp"
#include "PairIterator.hpp"
#include "HllSketchImpl.hpp"

#include <memory>
#include <iostream>

namespace sketches {

class HllSketchImpl;

class HllSketch : public BaseHllSketch {
  public:
    explicit HllSketch(const int lgConfigK);
    explicit HllSketch(const int lgConfigK, const TgtHllType tgtHllType);
    ~HllSketch();

    HllSketch* copy();
    HllSketch* copyAs(const TgtHllType tgtHllType);

    void reset();

    std::ostream& to_string(std::ostream& os, const bool summary,
                            const bool detail, const bool auxDetail, const bool all);

    double getEstimate();
    double getCompositeEstimate();
    double getLowerBound(int numStdDev);
    double getUpperBound(int numStdDev);

    int getLgConfigK();
    TgtHllType getTgtHllType();
    bool isOutOfOrderFlag();

    bool isCompact();
    bool isEmpty();

    int getUpdatableSerializationBytes();
    int getCompactSerializationBytes();

  protected:
    HllSketchImpl* hllSketchImpl;

    virtual std::unique_ptr<PairIterator> getIterator();

    CurMode getCurrentMode();

    // copy constructors
    HllSketch(const HllSketch& that);
    HllSketch(HllSketchImpl* that);

    virtual void couponUpdate(int coupon);

    std::string type_as_string();
    std::string mode_as_string();
};

std::ostream& operator<<(std::ostream& os, HllSketch& sketch);

void dump_sketch(HllSketch& sketch, const bool all);

}
