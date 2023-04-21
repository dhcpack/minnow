#include "reassembler.hh"
#include <iostream>

using namespace std;

int lower_bound_p( vector<pair<uint64_t, string>>& v, uint64_t num )
{
  if ( v.size() == 0 || num > v.back().first ) {
    return v.size();
  }
  int l = 0, r = v.size() - 1;
  while ( l < r ) {
    int mid = ( l + r ) >> 1;
    if ( num <= v[mid].first ) {
      r = mid;
    } else {
      l = mid + 1;
    }
  }
  return l;
}

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  if ( is_last_substring ) {
    last_index_ = first_index + data.size();
  }
  uint64_t unacc = next_index_ + output.available_capacity();
  // [next_index_, unacc)
  // [first_index, first_index + data.size())
  if ( first_index + data.size() <= next_index_ || first_index >= unacc ) {
    if ( next_index_ == last_index_ ) {
      output.close();
    }
    return;
  }
  if ( first_index < next_index_ ) {
    data = data.substr( next_index_ - first_index );
    first_index = next_index_;
  }
  if ( first_index + data.size() > unacc ) {
    data = data.substr( 0, unacc - first_index );
  }

  int pos = lower_bound_p( pendings_, first_index );
  pendings_.insert( pendings_.begin() + pos, { first_index, data } );
  bytes_pending_ += data.size();
  int id = max( 0, pos - 1 );
  while ( true ) {
    if ( id + 1 >= (int)pendings_.size() ) {
      break;
    }
    uint64_t r1 = pendings_[id].first + pendings_[id].second.size();
    uint64_t l2 = pendings_[id + 1].first, r2 = pendings_[id + 1].first + pendings_[id + 1].second.size();
    std::cout << pendings_[id].first << " " << r1 << "--" << l2 << " " << r2 << std::endl;
    if ( r1 >= l2 ) { // overlap
      if ( r1 >= r2 ) {
        bytes_pending_ -= pendings_[id + 1].second.size();
        pendings_.erase( pendings_.begin() + id + 1 );
      } else {
        bytes_pending_ -= ( r1 - l2 );
        pendings_[id].second += pendings_[id + 1].second.substr( r1 - l2 );
        pendings_.erase( pendings_.begin() + id + 1 );
      }
    } else if ( id == pos - 1 ) {
      id++;
    } else {
      break;
    }
  }

  while ( pendings_.size() && pendings_[0].first == next_index_ ) {
    next_index_ = first_index + pendings_[0].second.size();
    output.push( pendings_[0].second );
    bytes_pending_ -= pendings_.front().second.size();
    pendings_.erase( pendings_.begin() );
  }

  if ( next_index_ == last_index_ ) {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return bytes_pending_;
}
