#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader &header = seg.header();
    if(!_ackno.has_value() && !header.syn) {
        return;
    }
    if (!_ackno.has_value() && header.syn) {
        _isn = header.seqno;
        _reassembler.push_substring(seg.payload().copy(), 0, header.fin);
    } else if (!stream_out().input_ended()) {
        uint64_t stream_index = to_stream_index(
            unwrap(header.seqno, _isn,
                   to_absolute_seqno(_reassembler.first_unassembled_index() - 1)));
        _reassembler.push_substring(seg.payload().copy(), stream_index, header.fin);
    }
    _ackno = wrap(to_absolute_seqno(_reassembler.first_unassembled_index()), _isn);
    if(stream_out().input_ended()) {
        _ackno = _ackno.value() + 1;
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    return _ackno;
}

size_t TCPReceiver::window_size() const { return stream_out().remaining_capacity(); }
