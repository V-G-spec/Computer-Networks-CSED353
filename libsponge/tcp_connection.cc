#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _ms_since_last_incoming_seg; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    _ms_since_last_incoming_seg = 0;

    if (seg.header().rst) {
        inbound_stream().set_error();
        _sender.stream_in().set_error();
        _active = false;
        return;
    }

    _receiver.segment_received(seg);

    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
        while (!_sender.segments_out().empty()) {
            send_segment(false);
        }
    }

    if (seg.length_in_sequence_space() > 0) {
        // at least one segment is sent in reply, to reflect an update in the ackno and window size.
        _sender.fill_window();
        if (_sender.segments_out().empty()) {
            _sender.send_empty_segment();
        }
        send_segment(false);
    } else if (_receiver.ackno().has_value() && (seg.length_in_sequence_space() == 0) &&
               (seg.header().seqno == _receiver.ackno().value() - 1)) {
        _sender.send_empty_segment();
        send_segment(false);
    }

    if (!_sender.stream_in().eof() && _receiver.unassembled_bytes() == 0 && inbound_stream().input_ended()) {
        _linger_after_streams_finish = false;
    } else if (!_linger_after_streams_finish && _receiver.unassembled_bytes() == 0 && inbound_stream().input_ended() &&
               _sender.stream_in().eof() && _sender.bytes_in_flight() == 0) {
        _active = false;
    }
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    // Write data to the outbound byte stream, and send it over TCP if possible
    size_t actually_written = _sender.stream_in().write(data);
    _sender.fill_window();
    while (!_sender.segments_out().empty()) {
        send_segment(false);
    }
    return actually_written;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _ms_since_last_incoming_seg += ms_since_last_tick;

    _sender.tick(ms_since_last_tick);
    while (!_sender.segments_out().empty()) {
        if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
            send_segment(true);

            inbound_stream().set_error();
            _sender.stream_in().set_error();
            _active = false;
            return;
        } else {
            send_segment(false);
        }
    }

    if (_receiver.unassembled_bytes() == 0 && inbound_stream().input_ended() && _sender.stream_in().eof() &&
        _sender.bytes_in_flight() == 0) {
        if (_ms_since_last_incoming_seg >= 10 * _cfg.rt_timeout || !_linger_after_streams_finish) {
            _active = false;
        }
    }
}

void TCPConnection::end_input_stream() {
    // Shut down the outbound byte stream (still allows reading incoming data)
    _sender.stream_in().end_input();
    _sender.fill_window();
    while (!_sender.segments_out().empty()) {
        send_segment(false);
    }
}

void TCPConnection::connect() {
    // sending a SYN segment
    _sender.fill_window();
    while (!_sender.segments_out().empty()) {
        send_segment(false);
    }
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            _sender.send_empty_segment();
            send_segment(true);

            inbound_stream().set_error();
            _sender.stream_in().set_error();
            _active = false;
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

// private
void TCPConnection::send_segment(bool rst) {
    TCPSegment to_send = _sender.segments_out().front();
    if (rst) {
        to_send.header().rst = true;
    }

    if (_receiver.ackno().has_value()) {
        to_send.header().ack = true;
        to_send.header().ackno = _receiver.ackno().value();
    }

    to_send.header().win = static_cast<uint16_t>(_receiver.window_size());
    _segments_out.push(to_send);
    _sender.segments_out().pop();
}