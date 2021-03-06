
#ifndef SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
#define SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH

#include "byte_stream.hh"

#include <cstdint>
#include <string>
//#include <list>
#include <deque>
//! \brief A class that assembles a series of excerpts from a byte stream (possibly out of order,
//! possibly overlapping) into an in-order byte stream.
class StreamReassembler {
  private:
    // Writing var names as asked in .cc file
    // Realized that I kept some names after reading cs144 repo which had some functions which are probably from later
    // assignments Your code here -- add private members as necessary.
    bool _eof;  // Reqd in one of the functions. Might delete it if unnecessary
    // size_t eof_idx;       // Keep track of eof idx. If we reach that in the defragment function, _eof becomes true
    size_t _unass_bytes;           // Unassembled bytes
    size_t _base_index;            // Need to store the base index
    std::deque<size_t> _trackmap;  // Trackmap helps us to correspond bits in cases when they are not contiguous. It
                                   // moves parallely with buffer
    std::deque<char> _buffer;
    ByteStream _output;  //!< The reassembled in-order byte stream
    size_t _capacity;    //!< The maximum number of bytes
                         //    void defragment(const size_t index);
    void defragment();

  public:
    //! \brief Construct a `StreamReassembler` that will store up to `capacity` bytes.
    //! \note This capacity limits both the bytes that have been reassembled,
    //! and those that have not yet been reassembled.
    StreamReassembler(const size_t capacity);

    //! \brief Receive a substring and write any newly contiguous bytes into the stream.
    //!
    //! The StreamReassembler will stay within the memory limits of the `capacity`.
    //! Bytes that would exceed the capacity are silently discarded.
    //!
    //! \param data the substring
    //! \param index indicates the index (place in sequence) of the first byte in `data`
    //! \param eof the last byte of `data` will be the last byte in the entire stream
    void push_substring(const std::string &data, const uint64_t index, const bool eof);

    //! \name Access the reassembled byte stream
    //!@{
    const ByteStream &stream_out() const { return _output; }
    ByteStream &stream_out() { return _output; }
    //!@}

    //! The number of bytes in the substrings stored but not yet reassembled
    //!
    //! \note If the byte at a particular index has been pushed more than once, it
    //! should only be counted once for the purpose of this function.
    size_t unassembled_bytes() const;

    //! \brief Is the internal state empty (other than the output stream)?
    //! \returns `true` if no substrings are waiting to be assembled
    bool empty() const;
};

#endif  // SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
