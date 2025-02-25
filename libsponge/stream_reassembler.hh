#ifndef SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
#define SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH

#include "buffer_plus.hh"
#include "byte_stream.hh"

#include <cstdint>
#include <set>
#include <string>

//! \brief A class that assembles a series of excerpts from a byte stream (possibly out of order,
//! possibly overlapping) into an in-order byte stream.
class StreamReassembler {
  private:
    // Your code here -- add private members as necessary.
    struct Block {
        size_t index;     //!< The index of the data in BufferPlus
        BufferPlus data;  //!< The buffer to store the string

        Block(size_t idx, std::string &&str) noexcept : index(idx), data(std::move(str)) {}

        bool operator<(const Block &block) const { return index < block.index; }

        //! \brief the size of string in block
        size_t size() const { return data.size(); }
    };

    std::set<Block> _buffer{};     //!< The set to store unassembled strings
    size_t _unassembled_bytes{0};  //!< Total number of unassembled bytes
    bool _eof{false};              //!< Flag indicating that the end of bytes has been stored into `_buffer`
    ByteStream _output;            //!< The reassembled in-order byte stream
    size_t _capacity;              //!< The maximum number of bytes

    void insert_block(size_t index, std::string &&data);

  public:
    //! \brief Construct a `StreamReassembler` that will store up to `capacity` bytes.
    //! \note This capacity limits both the bytes that have been reassembled,
    //! and those that have not yet been reassembled.
    StreamReassembler(const size_t capacity);

    //! \brief Receive a substring and write any newly contiguous bytes into the stream.
    //!
    //! The StreamReassembler will stay within the memory limits of the `capacity`.
    //! Bytes that would exceed the capacity are silently discarded.
    //!
    //! \param data the substring
    //! \param index indicates the index (place in sequence) of the first byte in `data`
    //! \param eof the last byte of `data` will be the last byte in the entire stream
    void push_substring(const std::string &data, const uint64_t index, const bool eof);

    //! \name Access the reassembled byte stream
    //!@{
    const ByteStream &stream_out() const { return _output; }
    ByteStream &stream_out() { return _output; }
    //!@}

    //! The number of bytes in the substrings stored but not yet reassembled
    //!
    //! \note If the byte at a particular index has been pushed more than once, it
    //! should only be counted once for the purpose of this function.
    size_t unassembled_bytes() const;

    //! \brief Is the internal state empty (other than the output stream)?
    //! \returns `true` if no substrings are waiting to be assembled
    bool empty() const;
};

#endif  // SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
