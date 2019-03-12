/* SimpleQueue

Copyright 2019 Bert Melis

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

/**
 * @file SimpleQueue.h
 * @brief SimpleQueue API
 *
 * A simple queue
 */

#pragma once

/**
 * @brief Simple queue class.
 * 
 * @tparam T Type of the elements in the buffer.
 */
template <typename T>
class SimpleQueue {
 public:
  /**
   * @brief Construct a new SimpleQueue object.
   * 
   * @param size Maximum number of elements in the queue.
   */
  SimpleQueue(size_t size) :
    _buffer(nullptr),
    _firstPosition(0),
    _nextPosition(0),
    _count(0),
    _size(size) {
      _buffer = new T[_size];
    }

  /**
   * @brief Destroy the SimpleQueue object.
   * 
   * Not that when the buffer holds pointers, the actual objects 
   * will not be destroyed.
   * 
   */
  ~SimpleQueue() {
    delete[] _buffer;
  }

  /**
   * @brief Copies and adds an element to the buffer. 
   * 
   * @param t Element to add.
   * @return true Element was successfully added.
   * @return false Element has not been added (eg. queue full).
   */
  bool add(T t) {
    if (_count < _size) {
      _buffer[_nextPosition++] = t;
      ++_count;
      if (_nextPosition == _size) {
        // rollover to front of array
        _nextPosition = 0;
      }
      return true;
    }
    return false;
  }

  /**
   * @brief Removes the first element from the ringbuffer.
   * 
   * Note that the element is not really destroyed but the pointer
   * to the head of the buffer is moved.
   * 
   * Remove() on an empty buffer generates no error.
   * 
   */
  void remove() {
    if (_count > 0) {
      ++_firstPosition;
      if (_firstPosition == _size) {
        // rollover to front of array
        _firstPosition = 0;
      }
      --_count;
    }
  }

  /**
   * @brief Returns a pointer to the first element.
   * 
   * A pointer to the first element in the ringbuffer is returned.
   * To actually remove the element from the buffer, call `remove()`.
   * 
   * @return T* Pointer to the first element. nullptr on an empty buffer.
   */
  T* first() const {
    if (_count > 0) {
      return &_buffer[_firstPosition];
    } else {
      return nullptr;
    }
  }

  /**
   * @brief Return the number of elements in the buffer.
   * 
   * @return size_t number of elements.
   */
  size_t size() const {
    return _count;
  }

 private:
  T* _buffer;
  size_t _firstPosition;
  size_t _nextPosition;
  size_t _count;
  const size_t _size;
};
