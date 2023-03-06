#include <iostream>
#include <fstream>
#include <map>

using namespace std;

namespace wawtrack {

using bucket_t = map<void*, int>;

static bucket_t bucket;

void version(void) {
  cout << "tracker version 0.1.0" << endl;
}

void load(void *ptr) {
  bucket[ptr]--;
}

void store(void *ptr) {
  bucket[ptr]++;
}

void dump(void) {
  ofstream dump("wawtracker_dump.txt");

  for (auto &[ptr, count] : bucket) {
    if (count != 0) {
      dump << ptr << "\t" << count << endl;
    }
  }
}

class Printer {
public:
  Printer() = default;

  ~Printer() {
    dump();
  }
};

static Printer printer;

} // namespace wawtrack
