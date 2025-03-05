/**
  \file G3D-base.lib/include/G3D-base/WeakCache.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_WeakCache_h
#define G3D_WeakCache_h

#include "G3D-base/ReferenceCount.h"
#include "G3D-base/Table.h"

namespace G3D {

/**
   A cache that does not prevent its members from being garbage collected.
   Useful to avoid loading or computing an expression twice.  Useful
   for memoization and dynamic programming.

   Maintains a table of weak pointers.  Weak pointers do not prevent
   an object from being garbage collected.  If the object is garbage
   collected, the cache removes its reference.

   There are no "contains" or "iterate" methods because elements can be
   flushed from the cache at any time if they are garbage collected.

   Example:
   <pre>
      WeakCache<String, shared_ptr<Texture>> textureCache;

      shared_ptr<Texture> loadTexture(String s) {
          shared_ptr<Texture> t = textureCache[s];

          if (t.isNull()) {
              t = Texture::fromFile(s);
              textureCache.set(s, t);
          }

          return t;
      }
      
      
    </pre>
 */
template<class Key, class ValueRef>
class WeakCache {
    typedef weak_ptr<typename ValueRef::element_type> ValueWeakRef;

private:

    Table<Key, ValueWeakRef> table;

public:
    /**
       Returns nullptr if the object is not in the cache
    */
    ValueRef operator[](const Key& k) {
        if (table.containsKey(k)) {
            ValueWeakRef w = table[k];
            const ValueRef& s = w.lock();
            if (! s) {
                // This object has been collected; clean out its key
                table.remove(k);
            }
            return s;
        } else {
            return nullptr;
        }
    }

    
    void getValues(Array<ValueRef>& values) {
        Array<Key> keys;
        table.getKeys(keys);
        for (int i = 0; i < keys.size(); ++i) {
            const ValueRef& value = (*this)[keys[i]];
            if (notNull(value)) {
                values.append(value);
            }
        }
    }

    void clear() {
        table.clear();
    }

    void set(const Key& k, const ValueRef& v) {
        table.set(k, v);
    }

    /** Removes k from the cache or does nothing if it is not currently in the cache.*/
    void remove(const Key& k) {
        if (table.containsKey(k)) {
            table.remove(k);
        }
    }
};

}
#endif

