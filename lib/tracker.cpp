#include <iostream>
#include <map>

using namespace std;

namespace wawtrack {

using bucket_t = map<void*, int>;

static bucket_t bucket;

void version(void) {
  cout << "tracker version 0.1.0" << endl;
}

void load(void *ptr) {
  cout << "tracker load " << ptr << endl;
}

void store(void *ptr) {
  cout << "tracker store " << ptr << endl;
  bucket[ptr]++;
}

void dump(void) {
  cout << "tracker dump" << endl;
  for (auto &[ptr, count] : bucket) {
    cout << "\t" << ptr << "\t" << count << endl;
  }
}

class Printer {
public:
  Printer() { cout << "Printer()" << endl; } 

  ~Printer() {
    dump();
  }
};

static Printer printer;

} // namespace wawtrack
