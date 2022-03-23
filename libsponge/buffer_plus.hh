#ifndef SPONGE_BUFFER_PLUS_HH
#define SPONGE_BUFFER_PLUS_HH

#include <memory>
#include <string>

//! \brief A reference-counted read-only string that can discard bytes from the front
class BufferPlus {
  private:
    std::shared_ptr<std::string> _storage{};
    size_t _starting_offset{};
    size_t _ending_offset{};

  public:
    BufferPlus() = default;

    //! \brief Construct by taking ownership of a string
    BufferPlus(std::string &&str) noexcept : _storage(std::make_shared<std::string>(std::move(str))) {}

    //! \name Expose contents as a std::string_view
    //!@{
    std::string_view str() const {
        if (!_storage) {
            return {};
        }
        return {_storage->data() + _starting_offset, _storage->size() - _starting_offset - _ending_offset};
    }

    operator std::string_view() const { return str(); }
    //!@}

    //! \brief Get character at location `n`
    uint8_t at(const size_t n) const { return str().at(n); }

    //! \brief Size of the string
    size_t size() const {
        return _storage ? _storage->size() - _starting_offset - _ending_offset : 0;
    }

    //! \brief Make a copy to a new std::string
    std::string copy() const { return std::string(str()); }

    //! \brief Discard the first `n` bytes of the string (does not require a copy or move)
    //! \note Doesn't free any memory until the whole string has been discarded in all copies of the Buffer.
    void remove_prefix(size_t n) {
        if(!_storage) {
            return;
        }
        if (n >= str().size()) {
            _storage.reset();
            return;
        }
        _starting_offset += n;
    }

    //! \brief Discard the last `n` bytes of the string (does not require a copy or move)
    void remove_suffix(size_t n) {
        if(!_storage) {
            return;
        }
        if(n >= str().size()) {
            _storage.reset();
            return;
        }
        _ending_offset += n;
    }
};

#endif  // SPONGE_BUFFER_PLUS_HH
