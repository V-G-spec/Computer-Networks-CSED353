#include "tcp_receiver_best.hh"

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader &tcp_header = seg.header();

    // tracking syn/fin flag
    if (tcp_header.syn && !_is_received_syn) {
        _is_received_syn = true;
        _isn = tcp_header.seqno;
    }

    if (!_is_received_syn)
        return;

    _is_received_fin |= tcp_header.fin;

    // pushing payload into reassembler
    const WrappingInt32 relative_str_seqno(tcp_header.seqno + tcp_header.syn);
    const string &str = seg.payload().copy();
    const size_t absolute_str_seqno = unwrap(relative_str_seqno, _isn, _get_unassm_base());

    _reassembler.push_substring(str, absolute_str_seqno - 1, tcp_header.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_is_received_syn)
        return std::nullopt;

    const size_t auxiliary_term = _is_received_syn + (_is_received_fin && _reassembler.empty());
    const size_t absolute_seqno = auxiliary_term + _get_unassm_base();
    return wrap(absolute_seqno, _isn);
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
