#include <zxcvbn/util.hpp>

#include <algorithm>
#include <codecvt>
#include <locale>
#include <string>
#include <utility>

#include <cassert>

namespace zxcvbn {

namespace util {

std::string ascii_lower(const std::string & in) {
  const char A = 0x41, Z = 0x5A;
  const char a = 0x61;
  auto result = in;
  std::transform(result.begin(), result.end(), result.begin(),
                 [&] (char c) {
                   return (c >= A && c <= Z
                           ? c - A + a
                           : c);
                 });
  return result;
}

std::string reverse_string(const std::string & in) {
#ifdef _MSC_VER
  std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t> conv;
#else
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
#endif
  auto ret = conv.from_bytes(in);
  std::reverse(ret.begin(), ret.end());
  return conv.to_bytes(ret);
}

const std::codecvt_utf8<char32_t> char32_conv;

bool utf8_valid(std::string::const_iterator start,
                std::string::const_iterator end) {
  while (start != end) {
    std::mbstate_t st;

    const char *from = &*start;
    const char *from_end = &*end;
    const char *from_next;

    char32_t new_char;
    char32_t *to_next;

    auto res = char32_conv.in(st, from, from_end, from_next,
                              &new_char, &new_char + 1, to_next);
    if (!((res == std::codecvt_base::partial &&
              from_next != from_end) ||
             (res == std::codecvt_base::ok &&
              from_next == from_end))) {
      return false;
    }
    start += (from_next - from);
  }
  return true;
}

bool utf8_valid(const std::string & str) {
  return utf8_valid(str.begin(), str.end());
}

template<class It>
It _utf8_iter(It start, It end) {
  assert(start != end);
  std::mbstate_t st;
  auto amt = char32_conv.length(st, &*start, &*end, 1);
  return start + amt;
}

const char *utf8_iter(const char *start, const char *end) {
  return _utf8_iter(start, end);
}

std::string::size_type character_len(const std::string & str,
                                     std::string::size_type start,
                                     std::string::size_type end) {
    std::mbstate_t mbst;
    std::string::size_type pos = start, count = 0;
    while (pos < end) {
        std::string::size_type consumed = char32_conv.length(mbst, str.c_str() + pos, str.c_str() + end, 1);
        if(consumed > 0) {
            pos += consumed;
            count++;
        }
        else pos = end;
    }
    return count;
}

std::string::size_type character_len(const std::string & str) {
  return character_len(str, 0, str.size());
}

template<class It>
std::pair<char32_t, It> _utf8_decode(It it, It end) {
  std::mbstate_t st;
  char32_t new_char;
  char32_t *to_next;

  assert(it != end);

  const char *from = &*it;
  const char *from_end = &*end;
  const char *from_next;
  auto res = char32_conv.in(st, from, from_end, from_next,
                            &new_char, &new_char + 1, to_next);
  (void) res;

  return std::make_pair(new_char, it + (from_next - from));
}
/*
std::pair<char32_t, std::string::iterator> utf8_decode(std::string::iterator start,
                                                       std::string::iterator end) {
  return _utf8_decode(start, end);
}

std::pair<char32_t, std::string::const_iterator> utf8_decode(std::string::const_iterator start,
                                                             std::string::const_iterator end) {
  return _utf8_decode(start, end);
}
*/
char32_t utf8_decode(const std::string & start,
                     std::string::size_type & idx) {
  const char *cstring = start.c_str();
  auto ret = _utf8_decode(cstring + idx, cstring + start.length());
  idx += (start.begin() + (ret.second - cstring)) - (start.begin() + idx);
  return ret.first;
}

}

}
