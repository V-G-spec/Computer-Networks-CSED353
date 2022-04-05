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
    , _alarm(retx_timeout)
    , _syn(false)
    , _fin(false)
    , _abseqno(0)   // might be an issue!!
    , _win_size(0)  // Might be an issue! Be careful!
    , _RTO(retx_timeout)
    , _rec_ack(0)
    , _cons_retrans(0)
    , _in_flight(0) {}

uint64_t TCPSender::bytes_in_flight() const { return {_in_flight}; }

void TCPSender::fill_window() {
    if (_fin == true)
        return;
    if (_syn == false) {
        TCPSegment seg;
        seg.header().seqno = wrap(_next_seqno, _isn);
        seg.header().syn = true;
        _syn = true;
        _segments_out.push(seg);
        if (seg.length_in_sequence_space() > 0) {
            _wait_seg.push(seg);
            _abseqno = _next_seqno;
            _next_seqno += 1ull;
            _in_flight += 1ull;
            if (_alarm.started() == false) {
                // Alarm _alarm(_RTO);
                _alarm.start(_RTO);
            }
        }
    }
    size_t len = 0;
    if (_abseqno - _next_seqno > 0)
        len = _abseqno - _next_seqno;
    // else size_t len = 0;
    // size_t len = max(_abseqno - _next_seqno, 0);
    if (_win_size == 0 && len == 0)
        len = 1;
    while (len > 0) {
        TCPSegment seg;
        seg.header().seqno = wrap(_next_seqno, _isn);

        size_t len2 = min(len, _stream.buffer_size());
        len2 = min(len2, TCPConfig::MAX_PAYLOAD_SIZE);
        seg.payload() = Buffer(_stream.read(len2));
        len -= len2;

        // may have to send fin
        if (_stream.eof() && _stream.buffer_empty() && len) {
            len -= 1;
            seg.header().fin = _fin = true;
        }
        _next_seqno += seg.length_in_sequence_space();
        _in_flight += seg.length_in_sequence_space();
        if (seg.length_in_sequence_space()) {
            _segments_out.push(seg);
            _wait_seg.push(seg);
            if (_alarm.started() == false) {
                // Alarm _alarm(_RTO);
                _alarm.start(_RTO);
            }
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t abs_ackno = unwrap(ackno, _isn, _rec_ack);
    if (abs_ackno > _next_seqno)
        return;
    _win_size = window_size;
    // if (_fin==true &&
    if (abs_ackno <= _rec_ack)
        return;
    else {
        _rec_ack = abs_ackno;
        _RTO = _initial_retransmission_timeout;
        _cons_retrans = 0;
        _in_flight -= (abs_ackno - _abseqno);
        while ((_wait_seg.empty() == false) &&
               (ackno.raw_value() >=
                _wait_seg.front().length_in_sequence_space() + _wait_seg.front().header().seqno.raw_value())) {
            _wait_seg.pop();
        }
        if (_wait_seg.empty())
            _alarm.stop();
        _abseqno = max(_abseqno, abs_ackno + _win_size);
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    _alarm.tick(ms_since_last_tick);
    if (_alarm.limit_reached() == true) {  // Retransmit the earliest (lowest sequence number) segment that hasn’t been
                                           // fully acknowledged by the TCP receiver
        _segments_out.push(_wait_seg.front());
        if (_win_size != 0) {
            ++_cons_retrans;
            _RTO *= 2;
            // Alarm _alarm(_RTO);
            _alarm.start(_RTO);
        }
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _cons_retrans; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
}
