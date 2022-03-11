#include "stream_reassembler.hh"

#include <string>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`
using namespace std;

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

StreamReassembler::StreamReassembler(const size_t capacity)
    : _eof(false)
    , _unass_bytes(0)
    , _base_index(0)
    , _trackmap(capacity, 0)
    , _buffer(capacity, ' ')
    , _output(capacity)
    , _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    size_t tmplen = data.length();
    if (eof) {
        _eof = 1;
        // eof_idx = index;
    }
    if (_eof == 1 && tmplen == 0 && _unass_bytes == 0) {
        _output.end_input();
        return;
    }

    // Ignoring invalid idx (Case 1)
    if (index >= _base_index + _capacity) {
        return;
    }
    // Case 2 when offset will be positive
    if (index >= _base_index) {  // Not yet sure about >= or >
        size_t gap = index - _base_index;
        size_t to_fill = min(tmplen, _capacity - gap - _output.buffer_size());  // How many times we will read data
        if (to_fill != tmplen)
            _eof = 0;
        //_unass_bytes += to_fill;
        size_t trackval = _eof == true ? 2 : 1;
        for (size_t i = gap; i < to_fill + gap; i++) {  // Alternatively, can start i from 0 and do _trackmap[i+gap]
            if (_trackmap[i])
                continue;
            _buffer[i] = data[i - gap];
            _trackmap[i] = trackval;
            ++_unass_bytes;
        }
    }
    // else if (index + tmplen <= _base_index) {
    //	if (eof) _eof=1;
    //	if (_eof && _unass_bytes==0) _output.input_ended();
    //  }
    // Case 3 from reg (Negative offset. Rest is same as case 2)
    else if (index + tmplen > _base_index) {
        size_t gap = _base_index - index;
        size_t tofill = min(index + tmplen - _base_index, _capacity - _output.buffer_size());
        if (tofill != index + tmplen - _base_index)
            _eof = false;
        //_unass_bytes+=to_fill;
        size_t trackval = _eof == true ? 2 : 1;
        for (size_t i = 0; i < tofill; i++) {
            if (_trackmap[i])
                continue;
            _buffer[i] = data[i + gap];
            _trackmap[i] = trackval;
            ++_unass_bytes;
        }
    }
    defragment();  // Make stuff contiguous after checking and store it in outstream
    if (_eof == 1 && _unass_bytes == 0) {
        _output.end_input();
    }
    return;
}

void StreamReassembler::defragment() {  // Called after processing in buffer is done. Will correspond
                                        // trackmap with buffer and store stuff in output

    string tmpstr = "";
    size_t tmpl = 0;
    // DUMMY_CODE(data, index, eof);
    while (_trackmap.front()) {
        if (_trackmap.front() == 2)
            _eof = 1;
        ++tmpl;
        _trackmap.pop_front();
        _trackmap.push_back(false);
        tmpstr += _buffer.front();
        _buffer.pop_front();
        _buffer.push_back(' ');
    }
    if (_trackmap.front() == 2)
        _eof = 1;
    _output.write(tmpstr);
    _unass_bytes -= tmpl;
    _base_index += tmpl;
    // if (index+tmplen>=eof_idx) _eof=true;
}

size_t StreamReassembler::unassembled_bytes() const { return {_unass_bytes}; }

bool StreamReassembler::empty() const { return {_unass_bytes == 0}; }
