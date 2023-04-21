#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return zero_point + static_cast<uint32_t>( n );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  uint32_t offset = this->raw_value_ - wrap( checkpoint, zero_point ).raw_value_;
  uint64_t res = checkpoint + offset;
  /*
    如果offset大于1<<32的一半，则离checkpoint更近的应该是checkpoint前面的元素 res向前一段(2 ^ 32)；
    需要注意的是如果res小于2^32，则不能向前。
  */
  if ( offset >= ( 1U << 31 ) && res >= 1UL << 32 ) {
    res -= ( 1UL << 32 );
  }
  return res;
}
