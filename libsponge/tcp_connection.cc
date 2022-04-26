#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity();}

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight();}

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes();}

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_rcvd; }

void TCPConnection::real_send(bool syn){
    if (syn || _syn_sent) {
	_syn_sent = true;
	//_sender.fill_window();
	while(!_sender.segments_out().empty()){
	    TCPSegment segment = _sender.segments_out().front();
	    _sender.segments_out().pop();
	    optional<WrappingInt32> ackno = _receiver.ackno();
	    if (ackno.has_value()) {
        	segment.header().ack = true;
        	segment.header().ackno = ackno.value();
		segment.header().win = static_cast<uint16_t>(_receiver.window_size());
    	    }
	    if (_rst_send_reqd) segment.header().rst = _rst_send_reqd;
	    //segment.header().win = _receiver.window_size();
	    _segments_out.push(segment);
	}
	// Clean shutdown
	if (_receiver.stream_out().input_ended() && _sender.stream_in().eof() == false) _linger_after_streams_finish = false;
	//if (_receiver.unassembled_bytes() == 0 && _receiver.stream_out().input_ended() && _sender.stream_in().eof() == false) _linger_after_streams_finish = false;
	if ((_receiver.stream_out().input_ended() && _sender.stream_in().eof() && _sender.bytes_in_flight() == 0) && (_linger_after_streams_finish == false || _time_since_last_segment_rcvd >= 10*_cfg.rt_timeout)) {
	    _active = false;
	}
    }
}


void TCPConnection::unclean_shutdown(bool rst){
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _active = false;
    _linger_after_streams_finish = false;
    if (rst){
	_rst_send_reqd = true;
	if (!_sender.segments_out().size()){
	    _sender.send_empty_segment();
	}
	real_send(false);
    }
}


void TCPConnection::segment_received(const TCPSegment &seg) {
    if (_active){
	_time_since_last_segment_rcvd = 0;
	if (seg.header().rst) { //If rst is set already
	    _sender.stream_in().set_error();
	    _receiver.stream_out().set_error();
	    _active = false;
	    return;
	}

	_receiver.segment_received(seg); //Give seg to receiver
	
	if (seg.header().ack && _sender.next_seqno_absolute()>0) {
	    _sender.ack_received(seg.header().ackno, seg.header().win); //Not in closed state
	    _sender.fill_window();
	    real_send(false);
	}
	if (_receiver.unassembled_bytes() == 0 && _receiver.stream_out().input_ended() && (_sender.stream_in().eof()==false)) {
	    _linger_after_streams_finish = false;
	} //check if there is a need to linger
	
	if (seg.header().syn && _sender.next_seqno_absolute() == 0) {
            connect();  // Send syn
            return;
        }
	
	if (seg.header().ack) {
            _sender.ack_received(seg.header().ackno, seg.header().win);
	    _sender.fill_window();
            real_send(false);
	    return;
        }

	// Send empty segment
        if (seg.length_in_sequence_space() > 0 && _receiver.ackno().has_value() && _sender.segments_out().empty())
            _sender.send_empty_segment();
	
	real_send(false);
    }
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    if (data.size()==0) return 0;
    size_t tmp = _sender.stream_in().write(data);
    _sender.fill_window();
    real_send(false);
    return tmp;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    if (_active) {
	_time_since_last_segment_rcvd += ms_since_last_tick;
	_sender.tick(ms_since_last_tick); //tick sender for retransmit
	if (_sender.segments_out().size() > 0) {
	    if (_sender.consecutive_retransmissions() > _cfg.MAX_RETX_ATTEMPTS) {
		unclean_shutdown(true);
	    }
	    _sender.fill_window();
	    real_send(false);
	}
    }
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    real_send(false);
}

void TCPConnection::connect() {
    _sender.fill_window();
    real_send(true);
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
	    unclean_shutdown(true);
            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
