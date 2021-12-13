#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) { return isn + static_cast<uint32_t>(n); }

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    const uint32_t offset = n - isn - static_cast<uint32_t>(checkpoint);
    uint64_t result;
    if (offset <= (1U << 31)) {
        result = checkpoint + offset;
    } else {
        result = checkpoint - ((1UL << 32) - offset);
        // if `result` has 64 bit underflow, chose the right one.
        if (result > checkpoint) {
            result = checkpoint + offset;
        }
    }
    return result;

//    // simplify implementation
//    int32_t offset = static_cast<uint32_t>(checkpoint) - (n - isn);
//    int64_t result = checkpoint - offset;
//    return result >= 0 ? result : result + (1UL << 32);
}
