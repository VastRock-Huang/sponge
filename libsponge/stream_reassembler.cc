#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    size_t first_unassembled = _output.bytes_read() + _output.buffer_size();
    size_t first_unacceptable = _output.bytes_read() + _capacity;
    // the index of data after removing the reassembled bytes
    size_t idx = index;
    // the length of data after removing the reassembled or unacceptable bytes
    size_t len = data.size();
    // the eof flag of data after removing
    bool eof_flag = eof;
    // if the whole data string is reassembled or unacceptable, it will be discarded
    if (idx + len < first_unassembled || idx >= first_unacceptable) {
        return;
    }
    // remove the unassembled and unacceptable bytes in data
    // only update the `idx` and `len`
    if (idx < first_unassembled) {
        len -= first_unassembled - idx;
        idx = first_unassembled;
    }
    if (idx + len > first_unacceptable) {
        len = first_unacceptable - idx;
        // the eof must be false when removing the tail of data
        eof_flag = false;
    }
    // update the `_eof` flag, only when the end byte can be stored, will `_eof` be true
    _eof = _eof || eof_flag;

    // insert the substr of data after removing to `_buffer`
    insert_block(idx, data.substr(idx - index, len));

    // traverse the beginning of `_buffer` and write the assembled strings to `_output`
    while (!_buffer.empty()) {
        const Block &b = *_buffer.begin();
        // if the block `b` is not the assembled string
        if (first_unassembled != b.index) {
            break;
        }
        _output.write(b.data.copy());
        _unassembled_bytes -= b.size();
        first_unassembled += b.size();
        _buffer.erase(_buffer.begin());
    }
    // end the input of `_output` if `_eof` flag has been set and no unassembled bytes
    if (_eof && _unassembled_bytes == 0) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes == 0; }

//! use the data and its index to build a `Block` and insert it to `_buffer`
void StreamReassembler::insert_block(size_t index, string &&data) {
    // the size of `_unassembled_bytes` should update
    size_t delta_bytes = data.size();
    // do nothing if no data
    if (delta_bytes == 0) {
        return;
    }
    Block block(index, move(data));
    // a flag to record whether to insert
    bool hasInsert = false;
    // traverse all blocks in `_buffer`
    // note: using iterator because some blocks will be erased, use `for-range` is wrong
    for (auto it = _buffer.begin(); it != _buffer.end();) {
        const Block &b = *it;
        // if the whole `block` is to the left of `b`, it can be inserted all
        if (block.index + block.size() <= b.index) {
            _buffer.insert(block);
            hasInsert = true;
            break;
        }
        // if the whole `block` is to the right of `b`, it should compare with next one
        if (block.index >= b.index + b.size()) {
            ++it;
            continue;
        }
        // if `b` is part of the `block`, we should erase `b`
        if (block.index < b.index && block.index + block.size() > b.index + b.size()) {
            // update `delta_bytes`
            delta_bytes -= b.size();
            it = _buffer.erase(it);
        } else if (block.index < b.index) {
            // if the tail of `block` overlaps with `b`, remove its suffix and insert to `_buffer`
            size_t delta = block.index + block.size() - b.index;
            block.data.remove_suffix(delta);
            _buffer.insert(block);
            delta_bytes -= delta;
            hasInsert = true;
            break;
        } else if (block.index + block.size() > b.index + b.size()) {
            // if the head of `block` overlaps with `b`, remove its prefix and compare with the next block
            size_t delta = b.index + b.size() - block.index;
            block.data.remove_prefix(delta);
            block.index += delta;
            delta_bytes -= delta;
            ++it;
        } else {
            // if the `block` is part of `b`, it should not be inserted to `_buffer`
            delta_bytes = 0;
            hasInsert = true;
            break;
        }
    }
    // insert `block` to `_buffer` if it has not been inserted after traversing `_buffer`
    if (!hasInsert) {
        _buffer.insert(block);
    }
    _unassembled_bytes += delta_bytes;
}
