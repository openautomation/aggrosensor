#pragma once

// array using static-memory that sorts elements in ascending order, smallest item at the beginning
// assumes T has operator < overloaded
// uses memcpy, so T should not have a destructor!
template <class T, unsigned int SIZE>
class SortedArray {
public:
  unsigned int count;
  T array[SIZE];
  
public:
  SortedArray() {
    count = 0;
  }

  unsigned int size() { return count; }
  bool isFull() { return count == SIZE; }
  int isEmpty() { return count == 0; }

  // insert in the correct place in the array
  // TODO could do binary search
  bool add(T &e) {
    if(isFull()) return false;
    
    for (int i = 0; i < count; i++) {
      // does e belong here?
      if (e < array[i]) {
        // move item at i and following items toward the end to make room
        memcpy(array+i+1, array+i, (count-i)*sizeof(T));
        array[i] = e;
        count++;
        return true;
      }
    }
    
    // belongs at the end
    array[count++] = e;
    return true;
  }
  
  // index operator
  T& operator[](unsigned int index) {
    if (index >= count) return array[0];  // error
    return array[index];
  }
  
  // removes an item; optionally places a copy in dest
  // returns true if success
  bool remove(unsigned int index, T *dest = NULL) {
    if (index >= count) return false;
    // copy out the item if requested
    if (dest) memcpy(dest, array+index, sizeof(T));
    // copy remaining items closer to front
    memcpy(array+index, array+index+1, (count-index)*sizeof(T));
    //for (index++; index < count; index++) array[index-1] = array[index];
    count--;
    return true;
  }
};
