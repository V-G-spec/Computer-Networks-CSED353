#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) { _capacity(capacity); }

size_t ByteStream::write(const string &data) {
    size_t count = 0;
    for (char b: data){
	if (_buffer_size>=_capacity){
	    break;
	}
	_buffer_size+=1;
	_bytes_written+=1;
	count+=1;
	_stream.push_back(b);
    }
    return count;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t to_peek;
    if (len<_buffer_size){
	to_peek=len;
    } else {
	to_peek=_buffer_size;
    }
    string return_str = "";
    for (size_t i=0; i<to_peek; i++){
	return_str+=_stream[i];
    }
    return return_str;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t to_pop;
    if (len<_buffer_size){
	to_pop = len;
    } else {
	to_pop = _buffer_size;
    }
    while(to_pop--){
	_bytes_read +=1;
	_buffer_size -=1;
	_stream.pop_front();
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    const string retStr = peek_output(len);
    pop_output(len);
    return retStr;
}

void ByteStream::end_input() {_ended_ip = 1}

bool ByteStream::input_ended() const { return _ended_ip; }

size_t ByteStream::buffer_size() const { return _buffer_size; }

bool ByteStream::buffer_empty() const { return (_buffer_size==0); }

bool ByteStream::eof() const {
    if (input_ended() && buffer_empty()){
	return true;
    } else {return false;}
}

size_t ByteStream::bytes_written() const { return _bytes_written; }

size_t ByteStream::bytes_read() const { return _bytes_read; }

size_t ByteStream::remaining_capacity() const { return abs(_capacity-_buffer_size); }
