/*
 * Copyright 2018, Yahoo! Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */


#include <cstdio>

#include "HllSketch.hpp"
#include "Union.hpp"

int main() {
/*
  sketches::HllSketch* sketch = new sketches::HllSketch(4, sketches::TgtHllType::HLL_4);

  sketch->update("value1");
  sketch->update((uint64_t) 2);
  sketch->update(3.0);
  for (int i = 3; i < 10000; ++i) {
    std::cout << i << std::endl;
    sketch->update((uint64_t) i);
  }

  sketch->to_string(std::cout, true, true, false, true);

  sketches::HllSketch* newSketch = sketch->copyAs(sketches::TgtHllType::HLL_8);
  newSketch->to_string(std::cout, true, true, false, true);

  std::cout << "Done" << std::endl;

  delete sketch;
  delete newSketch;
*/
  sketches::HllSketch* s1 = new sketches::HllSketch(8, sketches::TgtHllType::HLL_8);
  sketches::HllSketch* s2 = new sketches::HllSketch(8, sketches::TgtHllType::HLL_8);

  int n = 10000;
  for (int i = 0; i < n; ++i) {
    s1->update((uint64_t) i);
    s2->update((uint64_t) i + (n / 2));
  }

  sketches::Union* hllUnion = new sketches::Union(8);
  hllUnion->update(s1);
  hllUnion->update(s2);

  hllUnion->to_string(std::cout, true, true, false, true);

  delete s1;
  delete s2;
  delete hllUnion;

  return 0;
}
