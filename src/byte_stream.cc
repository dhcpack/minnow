#include <iostream>
#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  // Your code here.
  uint64_t sz = min( available_capacity(), static_cast<uint64_t>( data.size() ) );
  byte_pushed_ += sz;
  for ( int i = 0; static_cast<uint64_t>(i) < sz; i++ ) {
    buffer_.push_back( data[i] );
  }
  // (void)data;
}

void Writer::close()
{
  // Your code here.
  closed_ = true;
}

void Writer::set_error()
{
  // Your code here.
  error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - ( byte_pushed_ - byte_poped_ );
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return byte_pushed_;
}

string_view Reader::peek() const
{
  // Your code here.
  /*
    string_view是一个字符串“视图”，是一个字符串指针和长度的集合，字符串指针必须指向内存中确实存在的数据
    使用 &buffer_.front() 队列中第一个字符的指针。这是一个指向有效内存的指针，并且与 buffer_ 的内容相关联，因此
    std::string_view 可以正确地引用它。
  */
  return std::string_view( &buffer_.front(), 1 );
}

bool Reader::is_finished() const
{
  // Your code here.
  return closed_ && byte_pushed_ == byte_poped_;
}

bool Reader::has_error() const
{
  // Your code here.
  return error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  uint64_t cnt = min( len, byte_pushed_ - byte_poped_ );
  byte_poped_ += cnt;
  buffer_.erase( buffer_.begin(), buffer_.begin() + cnt );
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return  byte_pushed_ - byte_poped_;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return byte_poped_;
}
