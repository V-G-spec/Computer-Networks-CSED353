#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Implementation of a TCP sender

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _retransmission_timer(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    // Regard window_size as 1 if its value is 0
    const size_t window_size = _window_size > 0 ? _window_size : 1;
    // Segments already sent must be considered in available window size
    size_t available_window_size = window_size - (_next_seqno - _absolute_ackno);

    while (!_fin_segment_sent && available_window_size > 0) {
        size_t sequence_length = 0;
        TCPSegment segment_to_send;

        // SYN case
        if (_next_seqno == 0) {
            segment_to_send.header().syn = true;
            sequence_length++;
        }
        // Set Payload
        // consider MAX_PAYLOAD_SIZE and current _stream size for the upperbound of payload size
        const size_t read_length_limit =
            TCPConfig::MAX_PAYLOAD_SIZE > _stream.buffer_size() ? _stream.buffer_size() : TCPConfig::MAX_PAYLOAD_SIZE;
        // Also, Considering the size (# of seqno) of TCPSegment must not be greater than available_window_size
        // Here, sequence_legnth is either 0 or 1, therefore it is guaranteed that stream_read_length >= 0
        size_t stream_read_length = read_length_limit + sequence_length > available_window_size
                                        ? available_window_size - sequence_length
                                        : read_length_limit;
        sequence_length += stream_read_length;
        segment_to_send.payload() = Buffer{_stream.read(stream_read_length)};  // payload read from _stream
        // FIN case
        if (_stream.eof() && sequence_length < available_window_size) {
            segment_to_send.header().fin = true;
            sequence_length++;
            _fin_segment_sent = true;
        }
        // Set Seqno : wrap function call necessary based on _isn
        segment_to_send.header().seqno = wrap(_next_seqno, _isn);

        if (sequence_length == 0)  // avoid infinite-loop
            break;

        // send the TCP segement
        _segments_out.push(segment_to_send);
        _retransmission_timer.start_timer();  // timer must be in running state
        // store a copy of the TCP Segment
        _outstanding_queue.push(segment_to_send);
        _bytes_in_flight += sequence_length;

        available_window_size -= sequence_length;
        _next_seqno += sequence_length;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    // checkpoint is the index of the last assembled byte
    uint64_t checkpoint = _next_seqno == 0 ? 0 : _next_seqno - 1;
    const uint64_t absolute_ackno = unwrap(ackno, _isn, checkpoint);
    _window_size = window_size;  // window_size could be increased
    // Considering the case where ackno from ACK has invalid value
    // or the case where there's no new reassembled segment in TCPReceiver
    if (absolute_ackno > _next_seqno || absolute_ackno <= _absolute_ackno)
        return;

    checkpoint = absolute_ackno == 0 ? 0 : absolute_ackno - 1;
    // pop all the ACK'ed segments from _outstanding_queue
    while (!_outstanding_queue.empty() && unwrap(_outstanding_queue.front().header().seqno, _isn, checkpoint) +
                                                  _outstanding_queue.front().length_in_sequence_space() <=
                                              absolute_ackno) {
        _bytes_in_flight -= _outstanding_queue.front().length_in_sequence_space();
        _outstanding_queue.pop();
    }

    _retransmission_timer.init_retransmission_timeout(_initial_retransmission_timeout);
    // if there's any remaining outstanding segments
    if (!_outstanding_queue.empty()) {
        _retransmission_timer.reset_timer();
        _retransmission_timer.start_timer();
    } else {
        _retransmission_timer.stop_timer();
    }

    _num_consec_retransmission = 0;
    _absolute_ackno = absolute_ackno;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    _retransmission_timer.tick_timer(ms_since_last_tick);  // increase the time by ms_since_last_tick if timer's running
    if (_retransmission_timer.timer_expired()) {
        // !_outstanding_queue.empty() should be guaranteed as invariant
        _segments_out.push(_outstanding_queue.front());  // retransmission
        if (_window_size > 0) {
            _num_consec_retransmission++;
            _retransmission_timer.exponential_backoff();  // Double RTO
        }
        _retransmission_timer.reset_timer();
        _retransmission_timer.start_timer();
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _num_consec_retransmission; }

void TCPSender::send_empty_segment() {
    TCPSegment segment_to_send;
    segment_to_send.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(segment_to_send);
    _retransmission_timer.start_timer();  // timer must be in running state
}

// RetransmissionTimer's method
TCPSender::RetransmissionTimer::RetransmissionTimer(const size_t initial_retransmission_timeout)
    : _retransmission_timeout(initial_retransmission_timeout) {}

void TCPSender::RetransmissionTimer::tick_timer(const size_t time_elapsed) {
    if (_running)
        _current_time += time_elapsed;
}

void TCPSender::RetransmissionTimer::reset_timer() { _current_time = 0; }

void TCPSender::RetransmissionTimer::start_timer() { _running = true; }

void TCPSender::RetransmissionTimer::stop_timer() { _running = false; }

void TCPSender::RetransmissionTimer::init_retransmission_timeout(const size_t initial_retransmission_timeout) {
    _retransmission_timeout = initial_retransmission_timeout;
}

void TCPSender::RetransmissionTimer::exponential_backoff() { _retransmission_timeout *= 2; }
// Retransmission timer is expired only if it was previously started and running.
bool TCPSender::RetransmissionTimer::timer_expired() { return _running && _current_time >= _retransmission_timeout; }