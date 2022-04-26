#include "wrapping_integers.hh"

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) { return WrappingInt32{isn + n}; }

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    const uint64_t relative_seqno = static_cast<uint32_t>(n - isn);

    if (checkpoint <= relative_seqno)
        return relative_seqno;

    const uint64_t ring_cnt = (checkpoint - relative_seqno) >> 32;
    const uint64_t lower_bound = relative_seqno + (ring_cnt << 32);
    const uint64_t upper_bound = relative_seqno + ((ring_cnt + 1) << 32);
    return (checkpoint - lower_bound) > (upper_bound - checkpoint) ? upper_bound : lower_bound;
}
