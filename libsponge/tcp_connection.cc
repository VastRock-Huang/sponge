#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    if (!_active) {
        return;
    }
    // reset the timer
    _time_since_last_segment_received = 0;
    const TCPHeader &header = seg.header();
    // if TCP does not receive SYN from remote peer, and not send SYN to remote peer
    if (!_receiver.ackno().has_value() && _sender.next_seqno_absolute() == 0) {
        // at this time, TCP acts as a server,
        // and should not receive any segment except it has SYN flag
        if (!header.syn) {
            return;
        }
        _receiver.segment_received(seg);
        // try to send SYN segment, use for opening TCP at the same time
        connect();
        return;
    }
    // the TCP ends in an unclean shutdown when receiving RST segment
    if (header.rst) {
        unclean_shutdown();
        return;
    }
    _receiver.segment_received(seg);
    // if TCP sends SYN segment as a client but does not receive SYN from remote peer,
    // the local TCP should not handle it, too.
    if (!_receiver.ackno().has_value()) {
        return;
    }

    // set the `_linger_after_streams_finish` the first time the inbound stream ends
    if (!_sender.stream_in().eof() && _receiver.stream_out().input_ended()) {
        _linger_after_streams_finish = false;
    }
    // use the remote peer's ackno and window size to update the local sending window
    if (header.ack) {
        _sender.ack_received(header.ackno, header.win);
    }

    _sender.fill_window();
    // makes sure that at least one segment is sent in reply
    if (seg.length_in_sequence_space() > 0 && _sender.segments_out().empty()) {
        _sender.send_empty_segment();
    }
    // an extra special case to respond to a keep-alive segment
    if (seg.length_in_sequence_space() == 0 && header.seqno == _receiver.ackno().value() - 1) {
        _sender.send_empty_segment();
    }
    send_segments();
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    if (!_active) {
        return 0;
    }
    size_t ret = _sender.stream_in().write(data);
    _sender.fill_window();
    send_segments();
    return ret;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    if (!_active) {
        return;
    }
    _time_since_last_segment_received += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);
    // TCP unclean shutdown if the number of consecutive retransmissions
    // is more than an upper limit
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        send_RST();
        return;
    }
    // TCP clean shutdown if necessary
    if (_receiver.stream_out().input_ended() && _sender.stream_in().eof() && _sender.bytes_in_flight() == 0 &&
        (!_linger_after_streams_finish || _time_since_last_segment_received >= 10 * _cfg.rt_timeout)) {
        _active = false;
        return;
    }
    // send segments when `_sender.tick()` has a retransmission
    send_segments();
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    send_segments();
}

void TCPConnection::connect() {
    _sender.fill_window();
    send_segments();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            send_RST();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::send_segments() {
    while (!_sender.segments_out().empty()) {
        TCPSegment segment = _sender.segments_out().front();
        _sender.segments_out().pop();
        optional<WrappingInt32> ackno = _receiver.ackno();
        // if TCP does not receive SYN segments from the remote peer, i.e. at SYN_SENT state
        // TCP will not set ACK flag and seqno
        if (ackno.has_value()) {
            segment.header().ack = true;
            segment.header().ackno = ackno.value();
        }
        // set the local receiver's window size
        segment.header().win = _receiver.window_size() <= numeric_limits<uint16_t>::max()
                                   ? _receiver.window_size()
                                   : numeric_limits<uint16_t>::max();
        _segments_out.emplace(segment);
    }
}

void TCPConnection::send_RST() {
    _sender.fill_window();
    if (_sender.segments_out().empty()) {
        _sender.send_empty_segment();
    }
    TCPSegment segment = _sender.segments_out().front();
    _sender.segments_out().pop();
    optional<WrappingInt32> ackno = _receiver.ackno();
    if (ackno.has_value()) {
        segment.header().ack = true;
        segment.header().ackno = ackno.value();
    }
    segment.header().win = _receiver.window_size() <= numeric_limits<uint16_t>::max() ? _receiver.window_size()
                                                                                      : numeric_limits<uint16_t>::max();
    segment.header().rst = true;
    _segments_out.emplace(segment);

    unclean_shutdown();
}

inline void TCPConnection::unclean_shutdown() {
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _active = false;
}
