#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/*  TCPTimer  */
TCPTimer::TCPTimer( uint64_t initial_RTO_ms )
  : running_( false )
  , initial_RTO_ms_( initial_RTO_ms )
  , curr_RTO_ms_( initial_RTO_ms )
  , time_( 0 )
  , retransmission_cnt_( 0 )
{}

bool TCPTimer::running()
{
  return running_;
}

void TCPTimer::start()
{
  running_ = true;
  curr_RTO_ms_ = initial_RTO_ms_;
  time_ = 0;
  retransmission_cnt_ = 0;
}

void TCPTimer::restart( bool double_RTO )
{
  running_ = true;
  if ( double_RTO ) {
    curr_RTO_ms_ *= 2;
  }
  time_ = 0;
  retransmission_cnt_++;
}

void TCPTimer::close()
{
  running_ = false;
  curr_RTO_ms_ = initial_RTO_ms_;
  time_ = 0;
  retransmission_cnt_ = 0;
}

bool TCPTimer::tick( uint64_t ms_since_last_tick )
{
  if ( !running_ ) {
    return false;
  }
  if ( time_ + ms_since_last_tick >= curr_RTO_ms_ ) {
    return true;
  }
  time_ += ms_since_last_tick;
  return false;
}

uint64_t TCPTimer::consecutive_retransmissions() const
{
  return retransmission_cnt_;
}

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), timer_( TCPTimer( initial_RTO_ms ) )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return sequence_numbers_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return timer_.consecutive_retransmissions();
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if ( messages_.empty() ) {
    return {};
  }
  TCPSenderMessage message = messages_.front();
  messages_.pop();
  return message;
}

void TCPSender::push( Reader& outbound_stream )
{
  // Your code here.
  uint64_t space = ackno_ + ( window_size_ == 0 ? 1 : window_size_ ) - sendno_;
  while ( space > 0 && !fin_sent_ ) {
    TCPSenderMessage message;
    // add SYN
    if ( !sendno_ ) {
      message.SYN = true;
      syn_sent_ = true;
      space--;
    }
    // add payload
    message.seqno = Wrap32::wrap( sendno_, isn_ );
    read( outbound_stream, min( space, TCPConfig::MAX_PAYLOAD_SIZE ), message.payload );
    space -= message.payload.size();
    // add FIN
    if ( outbound_stream.is_finished() && space > 0 ) {
      message.FIN = true;
      fin_sent_ = true;
      space--;
    }
    size_t len = message.sequence_length();
    if ( len == 0 ) {
      return;
    }
    messages_.push( message );
    outstanding_messages_.push( message );
    if ( !timer_.running() ) {
      timer_.start();
    }

    sendno_ += len;
    sequence_numbers_in_flight_ += len;
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  TCPSenderMessage message;
  message.seqno = Wrap32::wrap( sendno_, isn_ );
  return message;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  if ( msg.ackno.has_value() ) {
    ackno_ = msg.ackno.value().unwrap( isn_, sendno_ );
  }
  window_size_ = msg.window_size;

  bool refresh = false;
  while ( outstanding_messages_.size() ) {
    TCPSenderMessage& message = outstanding_messages_.front();
    uint64_t seqno = message.seqno.unwrap( isn_, sendno_ );
    if ( seqno + message.sequence_length() > ackno_ ) {
      break;
    }
    sequence_numbers_in_flight_ -= message.sequence_length();
    outstanding_messages_.pop();
    refresh = true;
  }

  if ( refresh ) {
    if ( outstanding_messages_.size() ) {
      timer_.restart( false );
    } else {
      timer_.close();
    }
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  if ( timer_.tick( ms_since_last_tick ) ) {
    messages_.push( outstanding_messages_.front() );
    timer_.restart( window_size_ != 0 );
  }
}
