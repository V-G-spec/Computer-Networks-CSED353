#include "wrapping_integers.hh"
//#include<iostream>
// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    //DUMMY_CODE(n, isn);
    //return WrappingInt32{0};
    return WrappingInt32((isn.raw_value() + uint32_t(n))%(1l<<32));
}

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
    //DUMMY_CODE(n, isn, checkpoint);
    //if (n-isn<0) uint64_t tmp = uint64_t(n-isn+(1ull<<32)); //INT_MAX but I am not sure if we are allowed to import math
    //else uint64_t tmp = uint64_t(n-isn);
    uint64_t tmp = n.raw_value()-wrap(checkpoint, isn).raw_value();
    uint64_t absSn = tmp+checkpoint;
    if ((tmp>= (1l<<31)) && (absSn>=(1l<<32))) absSn-=1l<<32;
    return absSn;
}
