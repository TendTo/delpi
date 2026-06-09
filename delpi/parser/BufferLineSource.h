/**
 * @author c3054737
 * @copyright 2026 delpi
 * @licence BSD 3-Clause License
 * @file
 * BufferLineSource class.
 */
#pragma once

#include <string>
#include <string_view>

namespace delpi {

/**
 * A class that provides a source for lines of text buffered in memory.
 * It keeps track of the current position in the buffer and allows retrieving lines one by one.
 * The buffer is represented as a contiguous block of memory,
 * and the class provides methods to access the lines without copying the data, using std::string_view.
 * The class also stores a name for the source, which can be used for error reporting or debugging purposes.
 * @note The buffer is not owned by the BufferLineSource,
 * and it is the caller's responsibility to ensure that the buffer remains valid
 * for the lifetime of the BufferLineSource object.
 */
class BufferLineSource {
 public:
  explicit BufferLineSource(const std::string_view data, std::string name = "buffer")
      : cur_(data.data()), end_(data.data() + data.size()), name_{std::move(name)} {}
  BufferLineSource(const char *data, const std::size_t length, std::string name = "buffer");

  /**
   * Retrieve the next line from the buffer.
   * A line terminates with an '\\n' or the end of the buffer.
   * Moreover, if an '\\r' character precedes the '\\n' character, it is removed from the returned view.
   * @param out view of a line in the buffer, stripped of any newline characters
   * @return true if there is still some buffer left to read
   * @return false if the end of the buffer has been reached
   */
  bool nextLine(std::string_view &out);

  /** @getter{name, BufferLineSource} */
  [[nodiscard]] const std::string &name() const { return name_; }

 protected:
  /**
   * Force a reinitialization of the object, setting @ref cur_ and @ref end_ to the new values.
   * @param data new buffer of data
   * @param length length of the new buffer
   */
  void initialize(const char *data, std::size_t length);

 private:
  const char *cur_;         ///< Pointer to the current position in the buffer
  const char *end_;         ///< Pointer to the end of the buffer
  const std::string name_;  ///< Name of the source, used for error reporting or debugging
};

}  // namespace delpi
