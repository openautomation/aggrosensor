#pragma once

// A queue/stack which doesn't use heap memory.
// You are cautioned against storing objects in here which use dynamic memory.
template <class T, unsigned int SIZE>
class RingBuffer {
  //typedef unsigned char IndexType;    // change this to int if you need to store more than 255 elements
  unsigned int start, count;
  T buffer[SIZE];
  
public:
  RingBuffer() {
    start = count = 0;
  }
 
  bool isFull() {
    return count == SIZE;
  }
 
  int isEmpty() {
    return count == 0;
  }
  
  // Allocates the next empty position in the queue and returns its address (for you to fill).
  // Returns NULL if the queue is full.
  T* push() {
    if (isFull()) {
        Serial.print("Error: buffer overflow\n");
        return NULL;
    }
   
    int index = start + count++;
    if (index >= SIZE) index = 0;
    return buffer + index;
  }

  // Copies source into the next empty position in the queue.
  // Returns false if the queue is full, true otherwise.
  bool push(const T &source) {
    T *element = push();
    if (element == NULL) return false;
    memcpy(element, &source, sizeof(T));
    return true;
  }

  // Returns the address of the first non-empty element.  Returns NULL if the queue is empty.
  // WARNING: the element is now considered empty, and may be overwritten the next time you call push()!
  // Use the other popFirst() if you're afraid of this happening.
  T* popFirst() {
    if (isEmpty()) {
      Serial.print("Error: buffer underflow\n");
      return NULL;
    }

    T *element = buffer + start++;
    if (start >= SIZE) start = 0;
    count--;
    return element;
  }
  
  // Copies the first element into the argument.
  // Returns false if the queue is empty, true otherwise.
  bool popFirst(T &dest) {
    T *element = popFirst();
    if (element == NULL) return false;
    memcpy(&dest, element, sizeof(T));
    return true;
  }
 
  // Returns the address of the most recently added element.  Returns NULL if the queue is empty.
  // WARNING: the element is now considered empty, and may be overwritten the next time you call push()!
  // Use the other popLast() if you're afraid of this happening.
  T* popLast() {
    if (isEmpty()) {
        Serial.print("Error: buffer underflow\n");
        return NULL;
    }

    int index = start + count - 1;
    if (index >= SIZE) index = count - SIZE - 1;   
    count--;
    return buffer + index;
  }
  
  // Copies the most recently added element into the argument.
  // Returns false if the queue is empty, true otherwise.
  bool popLast(T &dest) {
    T *element = popFirst();
    if (element == NULL) return false;
    memcpy(&dest, element, sizeof(T));
    return true;
  }
};