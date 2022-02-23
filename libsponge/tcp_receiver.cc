#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader &header = seg.header();
    if (!_isn.has_value()) {
        // the segment before receiving SYN segment should be discarded
        if (!header.syn) {
            return;
        }
        // set the ISN
        _isn = header.seqno;
    }
    // ISN occupies a seqno
    // `stream_out.bytes_written()` is equal to the index(absolute seqno) of last reassembled byte, which is checkpoint
    // use `unwrap()` to get absolute seqno, and minus 1 to get the stream index.
    uint64_t stream_index = unwrap(header.seqno + header.syn, _isn.value(), stream_out().bytes_written()) - 1;
    _reassembler.push_substring(seg.payload().copy(), stream_index, header.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    // if the ISN hasn’t been set yet, return an empty optional
    if (!_isn.has_value()) {
        return nullopt;
    }
    // `stream_out.bytes_written()+1` is the absolute seqno of the first unassembled byte
    // FIN flag also occupies a seqno
    return wrap(stream_out().bytes_written() + 1 + stream_out().input_ended(), _isn.value());
}

size_t TCPReceiver::window_size() const { return stream_out().remaining_capacity(); }
