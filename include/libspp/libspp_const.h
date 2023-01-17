#include <array>
#include <bitset>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <span>
#include <memory>
#include <optional>
#include <utility>
#include <vector>
#include <variant>

#include "libgetsetproxy/proxy.h"

// https://public.ccsds.org/Pubs/133x0b2e1.pdf

struct SPPPacket;

namespace spp {
  constexpr int VERSION_NUMBER_LEN = 3;
  constexpr int TYPE_FLAG_LEN = 1;
  constexpr int SEC_HDR_FLAG_LEN = 1;
  constexpr int APP_ID_LEN = 11;
  constexpr int SEQ_FLAGS_LEN = 2;
  constexpr int SEQ_CNT_OR_NAME_LEN = 14;
  constexpr int DATA_LEN_LEN = 16;
};

#pragma pack(push, 1)
struct SPPPrimaryHeader{
  friend SPPPacket;
private:
  uint16_t _app_id_h : spp::APP_ID_LEN - 8 = 0;
  uint16_t _sec_hdr_flag : spp::SEC_HDR_FLAG_LEN = 0;
  uint16_t _type : spp::TYPE_FLAG_LEN = 0;
  uint16_t _version_number : spp::VERSION_NUMBER_LEN = 0;
  uint16_t _app_id_l : 8 = 0;
  uint16_t _seq_cnt_or_name_h : spp::SEQ_CNT_OR_NAME_LEN - 8 = 0;
  uint16_t _seq_flags : spp::SEQ_FLAGS_LEN = 0;
  uint16_t _seq_cnt_or_name_l : 8 = 0;
  uint16_t _data_len_h : spp::DATA_LEN_LEN - 8 = 0;
  uint16_t _data_len_l : 8 = 0;

public:
  struct Iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = std::byte;
    using pointer           = value_type*;
    using reference         = value_type&;

    Iterator(const pointer ptr) : m_ptr(ptr) {}

    reference operator*() const { return *m_ptr; }
    pointer operator->() { return m_ptr; }

    // Prefix increment
    Iterator& operator++() { m_ptr++; return *this; }  

    // Postfix increment
    Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

    friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
    friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };     
  private:
    pointer m_ptr;

  };

  // TODO: do I need non-const member functions for the iterator?
  auto begin() { return Iterator(reinterpret_cast<std::byte*>(this)); }
  auto end() { return Iterator(reinterpret_cast<std::byte*>(this + 6)); }

  auto begin() const { return Iterator(reinterpret_cast<const std::byte*>(this)); }
  auto end() const { return Iterator(reinterpret_cast<const std::byte*>(this + 6)); }
};
#pragma pack(pop)
static_assert(std::is_trivially_copyable_v<SPPPrimaryHeader>,
              "SPPPrimaryHeader is not trivially copyable");
static_assert(std::is_standard_layout_v<SPPPrimaryHeader>,
              "SPPPrimaryHeader is not a standard layout type");
static_assert(sizeof(SPPPrimaryHeader) == 6,
              "SPPPrimaryHeader is not of size 6 as in the spec");

// 0 denotes a data section of a single byte
static constexpr std::size_t SPP_MAX_DATA_LEN = std::pow(2, 16);

struct SPPDataField {
  friend SPPPacket;
  friend auto operator<<(std::ostream & output, SPPPacket & packet) -> std::ostream &;
  friend auto operator>>(std::istream & input, SPPPacket & packet) -> std::istream &;
private:
  // TODO: initialise into secondary header and user data field
  std::vector<std::byte> _data = std::vector<std::byte>(0);

  auto resize(int len) {
    _data.resize(len);
  }

  auto size() const -> size_t {
    return _data.size();
  }

public:
  // TODO: do I need non-const member functions for the iterator?
  auto begin() { return _data.begin(); }
  auto end() { return _data.end(); }

  auto begin() const { return _data.begin(); }
  auto end() const { return _data.end(); }
};


struct SPPPacket {
private:
  SPPPrimaryHeader primary_header;
  SPPDataField data_field;

  // TODO: refactor so that SPPPacket is laid out in memory more like the real packet?
  bool dirty_length = true;

public:
  // TODO: could this iterator be implemented more efficiently, without having to compare
  // the variant each time?
  struct Iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = int;
    using value_type        = std::byte;
    using reference         = std::byte&;

    Iterator(
      SPPPrimaryHeader::Iterator it,
      SPPPacket* parent
      ) : it(it), parent(parent) {}
    Iterator(std::vector<std::byte>::iterator it) : it(it) {}

    const reference operator*() const { 
      return std::visit(
        [](auto x) -> reference { return *x; },
        it);
    } 

    // Prefix increment
    Iterator& operator++() { 
      // Increment the iterator
      if (std::holds_alternative<SPPPrimaryHeader::Iterator>(it)) {
        std::get<SPPPrimaryHeader::Iterator>(it)++;
      } else {
        std::get<std::vector<std::byte>::iterator>(it)++;
      }

      // Wrap over to next field
      if (std::holds_alternative<SPPPrimaryHeader::Iterator>(it) &&
          std::get<const SPPPrimaryHeader::Iterator>(it) == parent->primary_header.end()) {
        it = parent->data_field.begin();
      }
      return *this; 
    }

    // Postfix increment
    Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

    Iterator operator+(int x) const {
      auto tmp = *this;
      for (int i=0; i<x; i++) {
        if (std::holds_alternative<std::vector<std::byte>::iterator>(tmp.it) &&
            std::get<std::vector<std::byte>::iterator>(tmp.it) == tmp.parent->data_field.end()) {
          break;
        } 
        tmp++;
      }
      return tmp;
    }

    friend bool operator== (const Iterator& a, const Iterator& b) { return a.it == b.it; };
    friend bool operator!= (const Iterator& a, const Iterator& b) { return a.it != b.it; };     

  private:
    std::variant<const SPPPrimaryHeader::Iterator, const std::vector<std::byte>::iterator> it;
    const SPPPacket* parent;
  };

  Iterator begin() { return Iterator(primary_header.begin(), this); }
  Iterator end() { return Iterator(data_field.end()); }

  auto version_number() const & {
    return static_cast<int>(primary_header._version_number);
  }

  auto version_number() & {
    return Proxy{
      [this]() -> decltype(auto) { return std::as_const(*this).version_number(); },
      [this](int x) { primary_header._version_number = x; }
    };
  }

  auto type() const & {
    return static_cast<int>(primary_header._type);
  }

  auto type() & {
    return Proxy{
      [this]() -> decltype(auto) { return std::as_const(*this).type(); },
      [this](int x) { primary_header._type = x; }
    };
  }

  auto sec_hdr_flag() const & {
    return static_cast<int>(primary_header._sec_hdr_flag);
  }

  auto sec_hdr_flag() & {
    return Proxy{
      [this]() -> decltype(auto) { return std::as_const(*this).sec_hdr_flag(); },
      [this](int x) { primary_header._sec_hdr_flag = x; }
    };
  }

  auto app_id() const & {
    return static_cast<int>(primary_header._app_id_h << 8 | primary_header._app_id_l);
  }

  auto app_id() & {
    return Proxy{
      [this]() -> decltype(auto) { return std::as_const(*this).app_id(); },
      [this](int x) { primary_header._app_id_h = (x >> 8) & 0x03; primary_header._app_id_l = x & 0xff; }
    };
  }

  auto seq_flags() const & {
    return static_cast<int>(primary_header._seq_flags);
  }

  auto seq_flags() & {
    return Proxy{
      [this]() -> decltype(auto) { return std::as_const(*this).seq_flags(); },
      [this](int x) { primary_header._seq_flags = x; }
    };
  }

  auto seq_cnt_or_name() const & {
    return static_cast<int>(primary_header._seq_cnt_or_name_h << 8 | primary_header._seq_cnt_or_name_l);
  }

  auto seq_cnt_or_name() & {
    return Proxy{
      [this]() -> decltype(auto) { return std::as_const(*this).seq_cnt_or_name(); },
      [this](int x) {
        primary_header._seq_cnt_or_name_h = (x >> 8) & 0x3f;
        primary_header._seq_cnt_or_name_l = x  & 0xff;
      }
    };
  }

  auto data_len() const & {
    return static_cast<int>(primary_header._data_len_h << 8 | primary_header._data_len_l);
  }

  auto data_len() & {
    return Proxy{
      [this]() -> decltype(auto) { return std::as_const(*this).data_len(); },
      [this](int len) {
        data_field.resize(len+1);
        dirty_length = false;
        primary_header._data_len_h = (len >> 8) & 0xff;
        primary_header._data_len_l = (len) & 0xff;
      }
    };
  }

  auto data() const & -> auto const & {
    return data_field._data;
  }

  auto data() & {
    return Proxy{
      [this]() -> decltype(auto) { return std::as_const(*this).data(); },
      [this](std::span<std::byte> s) { 
        // Data section must contain at least a single octet
        if (s.size() == 0) {
          throw (std::invalid_argument("Data field must contain at least one byte"));
        }
        dirty_length = true;
        int size = std::min(s.size(), SPP_MAX_DATA_LEN);
        data_field.resize(size);
        std::copy_n(s.begin(), size, data_field._data.begin());
      }
    };
  }

  SPPPacket() = default;
  SPPPacket(uint8_t const *const input) {
    // Memcpy the fixed length header
    std::memcpy(&primary_header, input, sizeof(SPPPrimaryHeader));
    auto len = data_len() + 1;   // Length 0 is used to mean a single byte
    data_field.resize(len);
    std::memcpy(&data_field._data, &(input[sizeof(SPPPrimaryHeader)]), len);
  }

  auto size() -> size_t {
    return sizeof(primary_header) + data_field.size();
  }

  friend auto operator<<(std::ostream & output, SPPPacket & packet) -> std::ostream &;
  friend auto operator>>(std::istream & input, SPPPacket & packet) -> std::istream &;
};


auto operator<<(std::ostream & output, SPPPacket & packet) -> std::ostream & {
  if (packet.dirty_length) {
    // Set the length in accordance with the data length
    packet.data_len() = packet.data_field.size() - 1;
    packet.dirty_length = false;
  }
  output.write(reinterpret_cast<char*>(&packet.primary_header), sizeof(SPPPrimaryHeader));
  output.write(reinterpret_cast<char*>(packet.data_field._data.data()), packet.data_field.size());
  return output;
}


// TODO: is this safe in the case where there we didn't read enough input?
auto operator>>(std::istream & input, SPPPacket & packet) -> std::istream & {
  std::array<char, sizeof(SPPPrimaryHeader)> header = {};
  input.read(header.data(), sizeof(SPPPrimaryHeader));
  std::memcpy(&packet.primary_header, header.data(), sizeof(SPPPrimaryHeader));

  auto data_len = packet.data_len() + 1;   // Length 0 is used to mean a single byte
  std::vector<char> data;
  data.resize(data_len);
  input.read(&data[0], data_len);
  packet.data_field.resize(data_len);
  std::memcpy(packet.data_field._data.data(), data.data(), data_len);
  return input;
}
