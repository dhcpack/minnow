#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  Wrap32 seqno( message.seqno );
  if ( message.SYN ) {  // Omit SYN
    seqno = seqno + 1;
    zero_point_ = seqno;
  }

  if ( zero_point_.has_value() ) {
    reassembler.insert( seqno.unwrap( zero_point_.value(), reassembler.get_next_index() ),
                        message.payload,
                        message.FIN,
                        inbound_stream );
    next_index_ = Wrap32::wrap( reassembler.get_next_index(), zero_point_.value() );
  }

  if ( message.FIN ) {
    finned_ = true;
  }
  if ( finned_ && inbound_stream.is_closed() ) {
    next_index_ = next_index_.value() + 1;  // Omit FIN
  }
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  if ( inbound_stream.available_capacity() >= 65535 ) {
    return { next_index_, 65535 };
  } else {
    return { next_index_, (u_int16_t)inbound_stream.available_capacity() };
  }
}
