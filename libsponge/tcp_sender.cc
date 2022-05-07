#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , cur_RTO{retx_timeout} {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    uint16_t valid_window;

    if (!syn_) {  // send SYN
        TCPSegment seg;
        seg.header().syn = true;
        syn_ = true;
        send_segment(seg);
        return;
    }

    if (!syn_acked || fin_)
        return;  // It is SYN SENT state or FIN already was sent

    if (rwnd == 0) {  // if the window size is zero, we consider it one.
        valid_window = ackno_ + 1 - _next_seqno;
    } else if (ackno_ + rwnd > _next_seqno) {  // (window size) - (size of bytes already sent)
        valid_window = ackno_ + rwnd - _next_seqno;
    } else {
        valid_window = 0;
    }

    while (valid_window) {
        TCPSegment seg;

        size_t stream_size = _stream.buffer_size();
        if (_stream.buffer_empty() && !_stream.eof())
            break;

        size_t payload_len = min({TCPConfig::MAX_PAYLOAD_SIZE, static_cast<size_t>(valid_window), stream_size});
        seg.payload() = Buffer{_stream.read(payload_len)};

        if (payload_len == stream_size && _stream.eof() && static_cast<size_t>(valid_window) > payload_len) {
            seg.header().fin = true;
            fin_ = true;
        }

        send_segment(seg);
        valid_window -= static_cast<uint64_t>(seg.length_in_sequence_space());
        if (fin_)
            break;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t old_ackno = ackno_;
    ackno_ = unwrap(ackno, _isn, _next_seqno);
    rwnd = window_size;

    if (ackno_ > _next_seqno || ackno_ <= old_ackno)
        return;

    unsigned int queue_size = outstanding_segments.size();
    for (unsigned int i = 0; i < queue_size; i++) {
        TCPSegment &front_seg = outstanding_segments.front();
        uint64_t seg_len = static_cast<uint64_t>(front_seg.length_in_sequence_space());

        if (ackno_ < unwrap(front_seg.header().seqno, _isn, _next_seqno) + seg_len)
            break;
        if (front_seg.header().syn)
            syn_acked = true;  // It becomes SYN ACKED state

        _bytes_in_flight -= seg_len;
        outstanding_segments.pop();
    }

    if (queue_size > outstanding_segments.size()) {
        cur_RTO = _initial_retransmission_timeout;
        if_timer_running = true;
        time_elapsed = 0;
        consecutive_retransmissions_ = 0;
    }
    if (outstanding_segments.size() == 0 && if_timer_running) {
        if_timer_running = false;
    }

    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    time_elapsed += ms_since_last_tick;

    if (if_timer_running && time_elapsed >= cur_RTO && !outstanding_segments.empty()) {
        _segments_out.push(outstanding_segments.front());
        if (rwnd) {
            consecutive_retransmissions_++;
            cur_RTO <<= 1;
        }
        time_elapsed = 0;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return consecutive_retransmissions_; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
}

// private
void TCPSender::send_segment(TCPSegment &seg) {
    seg.header().seqno = wrap(_next_seqno, _isn);
    _next_seqno += static_cast<uint64_t>(seg.length_in_sequence_space());
    _bytes_in_flight += static_cast<uint64_t>(seg.length_in_sequence_space());
    _segments_out.push(seg);
    outstanding_segments.push(seg);
    if (!if_timer_running) {
        if_timer_running = true;
        time_elapsed = 0;
    }
}
