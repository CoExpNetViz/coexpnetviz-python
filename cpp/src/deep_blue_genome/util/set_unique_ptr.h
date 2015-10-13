#pragma once

/**
 * Helpers for (unordered) sets of unique_ptr<T>
 *
 * Taken from: http://stackoverflow.com/a/17853770/1031434
 */

#include <memory>

namespace DEEP_BLUE_GENOME {

namespace impl {
	template<class T>
	class maybe_deleter{
            private:
		bool _delete;

            public:
		explicit maybe_deleter(bool doit = true)
		:	_delete(doit)
		{
		}

		void operator()(T* p) const{
			if(_delete) delete p;
		}

            public: // treat as private (failed to friend boost::serialization)
		template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			ar & _delete;
		}
	};
}

template<class T>
using set_unique_ptr = std::unique_ptr<T, impl::maybe_deleter<T>>;

/**
 * Make key object to search for raw pointer in a set of set_unique_ptr
 */
template<class T>
set_unique_ptr<T> make_find_ptr(T* raw){
    return set_unique_ptr<T>(raw, impl::maybe_deleter<T>(false));
}

/**
 * Like make_unique
 */
template<typename T, typename... Args>
set_unique_ptr<T> make_set_unique(Args&&... args) {
    return set_unique_ptr<T>(new T(std::forward<Args>(args)...));
}

} // end namespace
