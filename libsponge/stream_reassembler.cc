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
    // the maximum index of byte that `_buffer` can store
    const size_t buffer_end = _buffer_begin + _output.remaining_capacity();
    // if the whole substring exceed the capacity, it will be silently discarded.
    if (index >= buffer_end) {
        return;
    }
    // set the `_eof` flag if the end byte can be stored
    if (eof && index + data.size() <= buffer_end) {
        _eof = true;
    }
    // traverse each byte of the substring in `capacity` range for reassembling.
    for (size_t buf_idx = max(index, _buffer_begin), data_idx = buf_idx - index;
         data_idx < data.size() && buf_idx < buffer_end;
         ++data_idx, ++buf_idx) {
        // not store bytes repeatedly
        if (_map[buf_idx - _buffer_begin]) {
            continue;
        }
        _buffer[buf_idx - _buffer_begin] = data[data_idx];
        _map[buf_idx - _buffer_begin] = true;
        ++_unassembled_bytes;
    }
    string bytes;
    // write the assembled bytes to `_output`
    while (_map.front()) {
        bytes.push_back(_buffer.front());
        _buffer.pop_front();
        _map.pop_front();
        // keep the size of `_buffer` and `_map` always `_capacity`
        _buffer.push_back('\0');
        _map.push_back(false);
    }
    size_t len = bytes.size();
    if (len > 0) {
        // adjust `_buffer_begin` and `_unassembled_bytes` if some bytes have been reassembled
        _buffer_begin += len;
        _unassembled_bytes -= len;
        _output.write(bytes);
    }
    // end the input of `_output` if `_eof` flag has been set and no unassembled bytes
    if (_eof && _unassembled_bytes == 0) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes == 0; }
