#include "stream_reassembler.hh"
#include<string>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _eof(false), _unass_bytes(0), _base_index(0), _trackmap(capacity, false), _buffer(capacity, ' '), _output(capacity), _capacity(capacity) {}


void StreamReassembler::defragment(){ //Called after processing in buffer is done. Will correspond trackmap with buffer and store stuff in output
    
    string tmp = "";
    size_t tmplen = 0;
    //DUMMY_CODE(data, index, eof);
    while(_trackmap.front()==true){
	++tmplen;
	_trackmap.pop_front();
	_trackmap.push_back(false);
	tmp+= _buffer.front();
	_buffer.pop_front();
	_buffer.push_back(' ');
    }
    _output.write(tmp);
    _unass_bytes-=tmplen;
    _base_index+=tmplen;
    
}



//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    size_t tmplen = data.length();
    if (eof==true && tmplen==0 && _unass_bytes==0) {
	_output.end_input();
	//_eof = true;
	return;
    }
    if (eof==true){
	_eof = true;
    }

    //Ignoring invalid idx
    if (index >= _base_index + _capacity) {
	return;
    }
    // Case 2 from reg
    else if (index >= _base_index) { //Not yet sure about >= or >
	size_t gap = index - _base_index;
	size_t to_fill = min(tmplen, _capacity - gap - _output.buffer_size());
	if (to_fill!=tmplen) _eof = false;
	//_unass_bytes += to_fill;
	for(size_t i=gap; i<to_fill+gap; i++){
	    if(_trackmap[i]==true) continue;
	    _buffer[i] = data[i-gap];
	    _trackmap[i] = true;
	    ++_unass_bytes;
	}
    }
    // Case 3 from reg
    else if (index+tmplen > _base_index){
	size_t gap = _base_index - index;
	size_t tofill = min(index+tmplen - _base_index, _capacity - _output.buffer_size());
	if (tofill != index+tmplen - _base_index) _eof=false;
	//_unass_bytes+=to_fill;
	for (size_t i =0; i<tofill; i++) {
	    if (_trackmap[i]==true) continue;
	    _buffer[i] = data[i+gap];
	    _trackmap[i] = true;
	    ++_unass_bytes;
	}
    }
    defragment(); //Make stuff contiguous after checking
    if (eof==true && _unass_bytes==0) {
	_eof = eof;
	_output.input_ended();
    }
    return;
}

size_t StreamReassembler::unassembled_bytes() const { return {_unass_bytes}; }

bool StreamReassembler::empty() const { return {_unass_bytes==0}; }
