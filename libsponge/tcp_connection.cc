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
    const TCPHeader &header = seg.header();
    // what if the seqno is not right?
    if (header.rst) {
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _active = false;
        return;
    }
    _receiver.segment_received(seg);
    _time_since_last_segment_received = 0;
    if (!_receiver.ackno().has_value()) {
        return;
    }
    // not sure _receiver is eof() or end_input()
    if (!_sender.stream_in().eof() && _receiver.stream_out().input_ended()) {
        _linger_after_streams_finish = false;
    }

    if (header.ack) {
        _sender.ack_received(header.ackno, header.win);
        if (_sender.stream_in().eof() && header.ackno == _sender.next_seqno()) {
            _fully_ack = true;
        }
        if (_fully_ack && !_linger_after_streams_finish) {
            _active = false;
            return;
        }
    }

    _sender.fill_window();
    if (seg.length_in_sequence_space() > 0 && _sender.segments_out().empty()) {
        _sender.send_empty_segment();
    }
    if (seg.length_in_sequence_space() == 0 && header.seqno == _receiver.ackno().value() - 1) {
        _sender.send_empty_segment();
    }
    send_segments();
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    size_t ret = _sender.stream_in().write(data);
    _sender.fill_window();
    send_segments();
    return ret;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _sender.tick(ms_since_last_tick);
    _time_since_last_segment_received += ms_since_last_tick;

    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        unclean_shutdown();
        return;
    }

    if (_receiver.stream_out().eof() && _fully_ack && _linger_after_streams_finish &&
        _time_since_last_segment_received >= 10 * _cfg.rt_timeout) {
        _active = false;
        return;
    }
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
            unclean_shutdown();
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
        if (ackno.has_value()) {
            segment.header().ack = true;
            segment.header().ackno = ackno.value();
        }
        segment.header().win = _receiver.window_size() <= numeric_limits<uint16_t>::max()
                                   ? _receiver.window_size()
                                   : numeric_limits<uint16_t>::max();
        _segments_out.emplace(segment);
    }
}

void TCPConnection::unclean_shutdown() {
    _sender.fill_window();
    if (_sender.segments_out().empty()) {
        _sender.send_empty_segment();
    }
    TCPSegment segment = _sender.segments_out().front();
    _sender.segments_out().pop();
    segment.header().rst = true;
    _segments_out.emplace(segment);

    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _active = false;
}