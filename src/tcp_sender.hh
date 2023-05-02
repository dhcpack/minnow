#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <queue>

class TCPTimer
{
private:
  bool running_;
  // 初始重传时间
  const uint64_t initial_RTO_ms_;
  // 当前重传时间
  uint64_t curr_RTO_ms_;
  // 当前时间
  uint64_t time_;
  // 超时重传次数
  uint64_t retransmission_cnt_;

public:
  explicit TCPTimer( uint64_t initial_RTO_ms );
  bool running();
  void start();
  void reset_start();
  void double_start( bool double_RTO );
  void close();
  bool tick( uint64_t ms_since_last_tick );
  uint64_t consecutive_retransmissions() const;
};

class TCPSender
{
  Wrap32 isn_;
  // uint64_t initial_RTO_ms_;

  TCPTimer timer_;
  // 已经成功接收的序号
  // std::optional<Wrap32> ackno {};
  uint64_t ackno_ = 0;
  // 已经成功发送的序号
  uint64_t next_seqno_ = 0;
  // 窗口大小
  uint16_t window_size_ = 1;
  // 等待发送的信息
  std::queue<TCPSenderMessage> messages_ {};
  // 发送但未确认的信息
  std::queue<TCPSenderMessage> outstanding_messages_ {};
  // // 连续重传数量
  // uint64_t consecutive_retransmissions_ = 0;
  // 发送但未接收的序号数量
  uint64_t sequence_numbers_in_flight_ = 0;

  bool syn_sent_ = false;
  bool fin_sent_ = false;

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  // ISN: Initial Sequence Number
  // RTO: Retransmission TimeOut
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};
