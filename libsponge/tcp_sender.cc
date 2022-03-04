#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    while (_sending_space > 0) {
        TCPHeader header;
        if (stream_in().eof()) {
            header.fin = true;
        }
        if (next_seqno_absolute() == 0) {
            header.syn = true;
        }
        header.seqno = next_seqno();
        Buffer buffer(_stream.read(min(_sending_space - header.syn - header.fin, TCPConfig::MAX_PAYLOAD_SIZE)));
        TCPSegment segment;
        segment.header() = header;
        segment.payload() = buffer;
        size_t len = segment.length_in_sequence_space();
        if(len == 0) {
            return;
        }

        segments_out().emplace(segment);
        if (!_timer.started()) {
            _timer.start();
        }
        _outstanding_segments.emplace(segment);
        _next_seqno += len;
        _bytes_in_flight += len;
        _sending_space -= len;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    uint64_t absolute_ackno = unwrap(ackno, _isn, next_seqno_absolute());
    _window_size = window_size;
    _sending_space += absolute_ackno + (window_size != 0 ? window_size : 1) - next_seqno_absolute();

    if (absolute_ackno == 1) {
        --_bytes_in_flight;
        return;
    }

    bool has_new = false;
    while (!_outstanding_segments.empty()) {
        TCPSegment segment = _outstanding_segments.front();
        size_t len = segment.length_in_sequence_space();
        uint64_t seqno = unwrap(segment.header().seqno, _isn, next_seqno_absolute());
        if (seqno + len > absolute_ackno)
            break;
        _outstanding_segments.pop();
        _bytes_in_flight -= len;
        has_new = true;
    }
    if (has_new) {
        _retransmission_timeout = _initial_retransmission_timeout;
        if (!_outstanding_segments.empty()) {
            _timer.start();
        } else {
            _timer.stop();
        }
        _consecutive_retransmissions = 0;
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    if (!_timer.expired(ms_since_last_tick, _retransmission_timeout)) {
        return;
    }
    segments_out().push(_outstanding_segments.front());
    if (_window_size != 0) {
        ++_consecutive_retransmissions;
        _retransmission_timeout <<= 1;
    }
    _timer.start();
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

void TCPSender::send_empty_segment() {
    TCPHeader header;
    header.seqno = next_seqno();
    if (next_seqno_absolute() == 0) {
        header.syn = true;
    }
    if (stream_in().eof()) {
        header.fin = true;
    }
    Buffer buffer;
    TCPSegment segment;
    segment.header() = header;
    segment.payload() = buffer;

    segments_out().emplace(segment);
    _bytes_in_flight += segment.length_in_sequence_space();
    _next_seqno += segment.length_in_sequence_space();
}