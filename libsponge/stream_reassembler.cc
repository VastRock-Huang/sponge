#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _buffer(capacity), _map(capacity), _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    const size_t buffer_end = _buffer_begin + _output.remaining_capacity();
    if (index >= buffer_end) {
        return;
    }
    if (eof && index + data.size() <= buffer_end) {
        _eof = true;
    }
    for (size_t buf_idx = max(index, _buffer_begin), data_idx = buf_idx - index;
         data_idx < data.size() && buf_idx < buffer_end;
         ++data_idx, ++buf_idx) {
        if (_map[buf_idx - _buffer_begin])
            continue;
        _buffer[buf_idx - _buffer_begin] = data[data_idx];
        _map[buf_idx - _buffer_begin] = true;
        ++_unassembled_bytes;
    }
    string bytes;
    while (_map.front()) {
        bytes.push_back(_buffer.front());
        _buffer.pop_front();
        _map.pop_front();
        _buffer.push_back('\0');
        _map.push_back(false);
    }
    size_t len = bytes.size();
    if (len > 0) {
        _buffer_begin += len;
        _unassembled_bytes -= len;
        _output.write(bytes);
    }

    if (_eof && _unassembled_bytes == 0) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes == 0; }
