/**
  \file G3D-base.lib/include/G3D-base/TimeQueue.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef G3D_base_TimeQueue_h
#define G3D_base_TimeQueue_h

#include "G3D-base/platform.h"
#include "G3D-base/Queue.h"

namespace G3D {

/**
 Dynamic queue that works with timestamps to allow simulating latency.

 Faster than std::deque for objects with constructors.

 Typical use:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TimeQueue<Event> eventQueue;
...

eventQueue.pushBack(newEvent, System::time() + latency);

...

Event e;
while (eventQueue.getPopFront(System::time(), e)) {
   ...
}
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

\see Queue, ThreadsafeQueue
 */
template <class T>
class TimeQueue {
private:
    
    class Entry {
    public:
        RealTime   time  = 0;
        T          value;

        Entry() {}
        Entry(T& value, RealTime time) : value(value), time(time) {}
    };

    Queue<Entry>  m_data;

public:

    /**
     Insert a new element into the front of the queue that must
     emerge no earlier than @time (a traditional queue only uses pushBack).

     If you push an event with a time later than the first event
     in the queue it will block subsequent events until that time
     is used with popFront().
     */
    inline void pushFront(const T& e, RealTime time) {
        m_data.pushFront(Entry(e, time));
    }

    /**
     Insert a new element at the end of the queue that must
     emerge no earlier than @time
    */
    inline void pushBack(const T& e, RealTime time) {
        m_data.pushBack(Entry(e, time));
    }

    /**
     pushBack
     */
    inline void enqueue(const T& e, RealTime time) {
        m_data.pushBack(Entry(e, time));
    }

    /**
     Remove the last element from the queue if its time stamp is
     less than or equal to @time and return true, otherwise return false.
     */
    inline bool getPopBack(RealTime time, T& value) {
        if ((m_data.size() > 0) && (m_data.last().time <= time)) {
            value = m_data.popBack();
            return true;
        } else {
            return false;
        }
    }

    /**
     Remove the first element from the queue if its time stamp is
     less than or equal to @time and return true, otherwise return false.
     */
    inline bool getPopFront(RealTime time, T& value) {
        if ((m_data.size() > 0) && (m_data[0].time <= time)) {
            value = m_data.popFront();
            return true;
        } else {
            return false;
        }
    }

   /**
    Removes all elements (invoking their destructors).

    @param freeStorage If false, the underlying array is not deallocated
    (allowing fast push in the future), however, the size of the Queue
    is reported as zero.
    
    */
   void clear(bool freeStorage = true) {
       m_data.clear(freeStorage);
   }

   /** Clear without freeing the underlying array. */
   void fastClear() {
       clear(false);
   }

   /**
      Number of elements in the queue. None may be available
      for popping based on the time.
    */
   inline int size() const {
       return m_data.size();
   }

   /**
    Number of elements in the queue. None may be available
      for popping based on the time.
    */
   inline int length() const {
       return size();
   }

   /**
    Performs bounds checks in debug mode
    */
   inline T& operator[](int n) {
      debugAssert((n >= 0) && (n < m_data.size()));
      return m_data[m_data.index(n)].value;
   }

   /**
    Performs bounds checks in debug mode
    */
    inline const T& operator[](int n) const {
        debugAssert((n >= 0) && (n < m_data.size()));
        return m_data[m_data.index(n)].value;
    }


    /** Returns the back element */
    inline const T& last() const {
        return (*this)[size() - 1];
    }

    inline T& last() {
        return (*this)[size() - 1];
    }

    bool empty() const {
        return size() == 0;
    }

    /**
     Returns true if the given element is in the queue.
     */
    bool contains(const T& e) const {
        for (int i = 0; i < size(); ++i) {
            if ((*this)[i] == e) {
                return true;
            }
        }

        return false;
    }

};

} // namespace

#endif
