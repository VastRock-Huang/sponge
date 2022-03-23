#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _cap(capacity) {}

size_t ByteStream::write(const string &data) {
    const size_t size = min(data.size(), _cap - _buffer_size);
    //
    _buffer.append(BufferList(move(string().assign(data.begin(), data.begin() + size))));
    _buffer_size += size;
    _total_written += size;
    return size;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    string str = _buffer.concatenate();
    return string().assign(string().assign(str.begin(), str.begin() + min(len, _buffer_size)));
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    const size_t size = min(len, _buffer_size);
    _buffer.remove_prefix(size);
    _buffer_size -= size;
    _total_read += size;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    const size_t size = min(len, _buffer_size);
    string str = _buffer.concatenate();
    _buffer.remove_prefix(size);
    _buffer_size -= size;
    _total_read += size;
    return str.assign(str.begin(), str.begin() + size);
}

void ByteStream::end_input() { _end = true; }

bool ByteStream::input_ended() const { return _end; }

size_t ByteStream::buffer_size() const { return _buffer_size; }

bool ByteStream::buffer_empty() const { return _buffer_size == 0; }

bool ByteStream::eof() const { return _end && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _total_written; }

size_t ByteStream::bytes_read() const { return _total_read; }

size_t ByteStream::remaining_capacity() const { return _cap - _buffer_size; }
