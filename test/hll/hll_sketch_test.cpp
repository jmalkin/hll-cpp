/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#include "src/hll/HllSketch.hpp"
#include "src/hll/HllUnion.hpp"
#include "src/hll/HllUtil.hpp"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cmath>
#include <cstring>

// this is for debug printing of hll_sketch using ostream& operator<<()
/*
namespace std {
  string to_string(const string& str) {
    return str;
  }
}
*/

namespace datasketches {

//static const double RANK_EPS_FOR_K_200 = 0.0133;
//static const double NUMERIC_NOISE_TOLERANCE = 1E-6;

class hll_sketch_test: public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(hll_sketch_test);
  CPPUNIT_TEST(simple_union);
  CPPUNIT_TEST(k_limits);
  //CPPUNIT_TEST(empty);
  CPPUNIT_TEST_SUITE_END();

  void simple_union() {
    HllSketch* s1 = new HllSketch(8, TgtHllType::HLL_8);
    HllSketch* s2 = new HllSketch(8, TgtHllType::HLL_8);

    int n = 10000;
    for (int i = 0; i < n; ++i) {
      s1->update((uint64_t) i);
      s2->update((uint64_t) i + (n / 2));
    }

    HllUnion* hllUnion = new HllUnion(8);
    hllUnion->update(s1);
    hllUnion->update(s2);

    hllUnion->to_string(std::cout, true, true, false, true);

    delete s1;
    delete s2;
    delete hllUnion;
  }

  void k_limits() {
    HllSketch* sketch1 = new HllSketch(HllUtil::MIN_LOG_K, TgtHllType::HLL_8);
    HllSketch* sketch2 = new HllSketch(HllUtil::MAX_LOG_K, TgtHllType::HLL_4);
    delete sketch1;
    delete sketch2;
    HllSketch testSketch(5, TgtHllType::HLL_8);
    CPPUNIT_ASSERT_THROW(new HllSketch(HllUtil::MIN_LOG_K - 1, TgtHllType::HLL_4), std::invalid_argument);
    CPPUNIT_ASSERT_THROW(new HllSketch(HllUtil::MAX_LOG_K + 1, TgtHllType::HLL_8), std::invalid_argument);
  }


};

CPPUNIT_TEST_SUITE_REGISTRATION(hll_sketch_test);

} /* namespace datasketches */
