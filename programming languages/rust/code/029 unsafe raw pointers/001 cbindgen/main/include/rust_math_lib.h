#include <cstdarg>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>

struct Accumulator;

struct Vector3 {
  double x;
  double y;
  double z;
};

extern "C" {

int32_t rust_add(int32_t a, int32_t b);

double vector3_length(Vector3 v);

Vector3 vector3_add(Vector3 a, Vector3 b);

Accumulator *accumulator_new();

void accumulator_add(Accumulator *ptr, double value);

double accumulator_total(const Accumulator *ptr);

void accumulator_free(Accumulator *ptr);

}  // extern "C"
