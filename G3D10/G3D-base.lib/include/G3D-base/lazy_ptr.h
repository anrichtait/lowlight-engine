/**
  \file G3D-base.lib/include/G3D-base/lazy_ptr.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_lazy_ptr_h
#define G3D_lazy_ptr_h

#include "G3D-base/platform.h"
#include "G3D-base/ReferenceCount.h"
#include <functional>
#include <mutex>

using std::nullptr_t;

namespace G3D {

/** 
  \brief Provides a level of indirection for accessing objects to allow computing them on
  demand or extending them with metadata without subclassing the object itself. For example,
  lazy loading of files.

  Useful for implementing lazy loading, for example, as done by ArticulatedModel::lazyCreate:

  ~~~~~~
      lazy_ptr<ArticulatedModel>([specification, name]{ return ArticulatedModel::create(specification, name); });
  ~~~~~~


  Analogous to shared_ptr and weak_ptr. Copies of lazy_ptrs retain the same underlying object,
  so it will only be resolved once.

  Threadsafe.
*/
template<class T>
class lazy_ptr {
private:

    class Proxy : public ReferenceCountedObject {
    public:
        mutable bool                    m_resolved;
        const std::function<shared_ptr<T>()> m_resolve;
        mutable shared_ptr<T>           m_object;
        mutable std::mutex              m_mutex;

        Proxy(const std::function<shared_ptr<T>()>& resolve, const shared_ptr<T>& object, bool resolved) : 
            m_resolved(resolved), m_resolve(resolve), m_object(object) {}

        shared_ptr<T> resolve() const {
            std::lock_guard<std::mutex> guard(m_mutex);
            if (! m_resolved) {
                debugAssert(m_resolve != nullptr);
                m_object = m_resolve();
                m_resolved = true;
            }
            return m_object;
        }


        bool operator==(const Proxy& other) const {
            // Pre-resolved case
            if ((m_resolve == nullptr) || (other.m_resolve == nullptr)) {
                return m_object == other.m_object;
            }

            // Check for same lock before locking the mutex to avoid deadlock
            if (&other == this) { return true; }             

            // lock both mutexes together to prevent deadlock
            std::lock(m_mutex, other.m_mutex);

            // Same object after resolution, or same resolution function
            const bool result = (m_resolved && other.m_resolved && (m_object.get() == other.m_object.get()));

            m_mutex.unlock();
            other.m_mutex.unlock();
            return result;
        }
    };

    shared_ptr<Proxy>  m_proxy;

public:

    /** Creates a nullptr lazy pointer */
    lazy_ptr() {}

    lazy_ptr(nullptr_t) {}

    /** Creates a lazy_ptr from a function that will create the object */
    lazy_ptr(const std::function<shared_ptr<T> (void)>& resolve) : m_proxy(new Proxy(resolve, nullptr, false)) {}

    /** Creates a lazy_ptr for an already-resolved object */
    lazy_ptr(const shared_ptr<T>& object) : m_proxy(new Proxy(nullptr, object, true)) {}

    /** Creates a lazy_ptr for an already-resolved object */
    template <class S>
    lazy_ptr(const shared_ptr<S>& object) : m_proxy(new Proxy(nullptr, dynamic_pointer_cast<T>(object), true)) {}

    /** Is the proxy itself a null pointer */
    bool isNull() const {
        return (m_proxy.get() == nullptr);
    }

    /** Returns a pointer to a T or a nullptr pointer. If there
       are multiple levels of proxies, then this call resolves all of them. */
    const shared_ptr<T> resolve() const { 
        return isNull() ? shared_ptr<T>() : m_proxy->resolve();
    }

    /** True if this object can be resolved() without triggering any evaluation */
    bool resolved() const {
        return isNull() || m_proxy->m_resolved;
    }

    /** \copydoc resolve */
    shared_ptr<T> resolve() {         
        return isNull() ? shared_ptr<T>() : m_proxy->resolve();
    }

    bool operator==(const lazy_ptr<T>& other) const {
        return (m_proxy == other.m_proxy) || 
            (! isNull() && ! other.isNull() &&
                (*m_proxy == *other.m_proxy));
    }

    bool operator!=(const lazy_ptr<T>& other) const {
        return *this != other;
    }

    /** Invokes resolve. If you intend to use multiple dereferences, it is faster to
        invoke resolve() once and store the shared_ptr. */
    const T& operator*() const {
        return *resolve();
    }

    /** Invokes resolve. If you intend to use multiple dereferences, it is faster to
        invoke resolve() once and store the shared_ptr. */
    T& operator*() {
        return *resolve();
    }

    /** Invokes resolve. If you intend to use multiple dereferences, it is faster to
        invoke resolve() once and store the shared_ptr. */
    const T* operator->() const {
        return resolve().get();
    }

    /** Invokes resolve. If you intend to use multiple dereferences, it is faster to
        invoke resolve() once and store the shared_ptr. */
    T* operator->() {
        return resolve().get();
    }
};



template<class T>
bool isNull(const lazy_ptr<T>& ptr) {
    return ptr.isNull();
}

template<class T>
bool notNull(const lazy_ptr<T>& ptr) {
    return ! ptr.isNull();
}


} // namespace G3D

#endif // G3D_lazy_ptr_h
