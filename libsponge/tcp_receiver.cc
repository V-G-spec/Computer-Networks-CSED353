#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader _header = seg.header();
    const WrappingInt32 seqno = _header.seqno;  // Hoping I am doing overloading correctly. Diff names for same value is
                                                // harder to understand for me
    if (_header.syn == true && _synRec == true)  // Cannot receive another
        return;
    if (_header.syn == false &&
        _synRec == false)  // I doubt this will ever be the case, but best to make a code that handles everything
        return;
    if (_header.syn == true) {  // Automatically means _synRec is false
        _synRec = true;
        _isn = seqno;
        _ackno = _isn + 1;
        if (_header.fin == true)
            _finRec = true;
        _reassembler.push_substring(seg.payload().copy(), 0, _finRec);
        return;
    }
    if (_synRec == true && _header.fin == true) {  // Have received both (In context, received fin)
        _finRec = true;
    }

    uint64_t idx =
        unwrap(seqno, _isn, _reassembler.stream_out().bytes_written());  // abs seq calculated using checkpoint
    if (_synRec == true)
        idx -= 1;
    _reassembler.push_substring(seg.payload().copy(), idx, _finRec);

    //_reassembler.stream_out().end_input(); // One test case failed on using this. I believe this has to be handled
    // separately which I do not have time for. Making note of it so that in future if this gets stuck, I know where to
    // start
    if (_reassembler.empty() == true && _finRec == true)
        _reassembler.stream_out().end_input();
    return;
    // DUMMY_CODE(seg);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_synRec == false)
        return nullopt;
    if (_ackno ==
        WrappingInt32{0})  // Made this change after first couple of test cases made it look like 0-null was the problem
        return nullopt;    // NULL doesn't work
    // return _ackno;
    // return _synRec==false?nullopt:_ackno;
    // Now I make 2 cases, one is where we have finished all the bytes. In this case, the fin sign will occupy 1 byte
    if (_reassembler.empty() == true && _finRec == true)
        return wrap(_reassembler.stream_out().bytes_written() + 2, _isn);
    // In the second case, either of the 2 conditions don't hold, and thus we do not need an extra byte.
    return wrap(_reassembler.stream_out().bytes_written() + 1, _isn);  //
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
