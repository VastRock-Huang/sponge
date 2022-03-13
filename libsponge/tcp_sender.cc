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
    , _stream(capacity)
    , _retransmission_timeout(retx_timeout) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    // if `_sending_end` has been set, the sender shouldn't send any new bytes
    if (_sending_ending) {
        return;
    }
    // if the window size is 0, it should act like the window size is 1
    size_t sending_space = _ackno + (_window_size != 0 ? _window_size : 1) - next_seqno_absolute();
    // have the sending space and not get to sending ending
    while (sending_space > 0 && !_sending_ending) {
        TCPSegment segment;
        TCPHeader &header = segment.header();
        if (next_seqno_absolute() == 0) {
            header.syn = true;
            --sending_space;
        }
        header.seqno = next_seqno();
        Buffer &buffer = segment.payload();
        buffer = stream_in().read(min(sending_space, TCPConfig::MAX_PAYLOAD_SIZE));
        // don't add FIN if this would make the segment exceed the receiver's window
        sending_space -= buffer.size();
        if (stream_in().eof() && sending_space > 0) {
            header.fin = true;
            --sending_space;
            // set `_sending_ending` true, so that sender will never send any new bytes
            _sending_ending = true;
        }

        size_t len = segment.length_in_sequence_space();
        if (len == 0) {
            return;
        }

        segments_out().emplace(segment);
        if (!_timer.started()) {
            _timer.start();
        }
        _outstanding_segments.emplace(segment);

        _next_seqno += len;
        _bytes_in_flight += len;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    _ackno = unwrap(ackno, _isn, next_seqno_absolute());
    // impossible ackno (beyond next seqno) should be ignored
    if (_ackno > next_seqno_absolute()) {
        return;
    }
    _window_size = window_size;

    // the flag indicating that if new data has been acknowledged
    bool has_new = false;
    while (!_outstanding_segments.empty()) {
        TCPSegment segment = _outstanding_segments.front();
        size_t len = segment.length_in_sequence_space();
        uint64_t seqno = unwrap(segment.header().seqno, _isn, next_seqno_absolute());
        // the segment is not fully acknowledged, should stop
        if (seqno + len > _ackno) {
            break;
        }
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
    TCPSegment segment;
    segment.header().seqno = next_seqno();
    segments_out().emplace(segment);
}