/*
 * Copyright 2018, Yahoo! Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "BaseHllSketch.hpp"
#include "HllSketch.hpp"

namespace datasketches {

/**
 * This performs union operations for HLL sketches. This union operator is configured with a
 * <i>lgMaxK</i> instead of the normal <i>lgConfigK</i>.
 *
 * <p>This union operator does permit the unioning of sketches with different values of
 * <i>lgConfigK</i>.  The user should be aware that the resulting accuracy of a sketch returned
 * at the end of the unioning process will be a function of the smallest of <i>lgMaxK</i> and
 * <i>lgConfigK</i> that the union operator has seen.
 *
 * <p>This union operator also permits unioning of any of the three different target HllSketch
 * types.
 *
 * <p>Although the API for this union operator parallels many of the methods of the
 * <i>HllSketch</i>, the behavior of the union operator has some fundamental differences.
 *
 * <p>First, the user cannot specify the {@link TgtHllType} as an input parameter.
 * Instead, it is specified for the sketch returned with {@link #getResult(TgtHllType)}.
 *
 * <p>Second, the internal effective value of log-base-2 of <i>K</i> for the union operation can
 * change dynamically based on the smallest <i>lgConfigK</i> that the union operation has seen.
 *
 * @author Lee Rhodes
 * @author Kevin Lang
 */
class HllUnion : public BaseHllSketch {
  public:
    explicit HllUnion(const int lgMaxK);
    explicit HllUnion(HllSketch& sketch);

    virtual ~HllUnion();

    double getEstimate();
    double getCompositeEstimate();
    double getLowerBound(const int numStdDev);
    double getUpperBound(const int numStdDev);

    int getCompactSerializationBytes();
    int getUpdatableSerializationBytes();
    int getLgConfigK();

    CurMode getCurMode();
    TgtHllType getTgtHllType();
    bool isCompact();
    bool isEmpty();
    bool isOutOfOrderFlag();

    void reset();

    HllSketch* getResult();
    HllSketch* getResult(TgtHllType tgtHllType);

    std::ostream& to_string(std::ostream& os, const bool summary,
                            const bool detail, const bool auxDetail, const bool all);

    void update(HllSketch& sketch);
    void update(HllSketch* sketch);

    static int getMaxSerializationBytes(const int lgK);


  protected:
    void couponUpdate(const int coupon);

   /**
    * Union the given source and destination sketches. This static method examines the state of
    * the current internal gadget and the incoming sketch and determines the optimum way to
    * perform the union. This may involve swapping, down-sampling, transforming, and / or
    * copying one of the arguments and may completely replace the internals of the union.
    *
    * @param incomingImpl the given incoming sketch, which may not be modified.
    * @param gadgetImpl the given gadget sketch, which must have a target of HLL_8 and may be
    * modified.
    * @param lgMaxK the maximum value of log2 K for this union.
    * //@return the union of the two sketches in the form of the internal HllSketchImpl, which for
    * //the union is always in HLL_8 form.
    */
    void unionImpl(HllSketchImpl* incomingImpl, const int lgMaxK);

    static HllSketchImpl* copyOrDownsampleHll(HllSketchImpl* srcImpl, const int tgtLgK);

    // calls couponUpdate on sketch, freeing the old sketch upon changes in CurMode
    static HllSketchImpl* leakFreeCouponUpdate(HllSketchImpl* impl, const int coupon);

    const int lgMaxK;
    HllSketch* gadget;
};

}
