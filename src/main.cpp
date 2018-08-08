/*
 * Copyright 2018, Yahoo! Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */


#include <cstdio>

#include "MurmurHash3.h"
#include "HllSketch.hpp"

using namespace sketches;

int main() {
  HllSketch* sketch = new HllSketch(4, sketches::TgtHllType::HLL_4);

  sketch->update("value1");
  sketch->update((uint64_t) 2);
  sketch->update(3.0);
  for (int i = 3; i < 10000; ++i) {
    std::cout << i << std::endl;
    sketch->update((uint64_t) i);
  }

  //cout << sketch;
  std::ostringstream oss;
  sketch->to_string(oss, true, true, false, true);
  std::cout << oss.str();

  std::cout << "Done" << std::endl;

  delete sketch;

  return 0;
}
