#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    
    const TCPHeader _header = seg.header();
    const WrappingInt32 seqno = _header.seqno; //Hoping I am doing overloading correctly. Diff names for same value is harder to understand for me
    if (_header.syn==true && _synRec==true) return;
    if (_header.syn==false && _synRec==false) return;
    if (_header.syn==true) {
	_synRec=true;
	_isn = seqno;
	_ackno = _isn+1;
	if (_header.fin==true) _finRec=true;
	_reassembler.push_substring(seg.payload().copy(), 0, _finRec);
	return;
    }
    if (_synRec==true && _header.fin==true) {//Have received both (In context, received fin)
	_finRec = true; //We can write here after computing the idx, but will have to make cases later.
    }

    uint64_t idx = unwrap(seqno, _isn, _reassembler.stream_out().bytes_written()); //abs seq calculated using checkpoint
    if (_synRec==true) idx -=1;
    _reassembler.push_substring(seg.payload().copy(), idx, _finRec);
    return;
    //DUMMY_CODE(seg);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_synRec==false) return WrappingInt32{0};
    return _ackno;
    //return _synRec==false?nullopt:_ackno;
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
