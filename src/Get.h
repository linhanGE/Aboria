
#ifndef GET_H_ 
#define GET_H_ 

#include <boost/mpl/vector.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <algorithm>
#include <tuple>
#include <type_traits>

#include "Utils.h"


namespace Aboria {

namespace mpl = boost::mpl;

// implementation of c++11 make_index_sequence 
// copied from http://stackoverflow.com/questions/17424477/implementation-c14-make-integer-sequence

template<size_t...> struct index_sequence { using type = index_sequence; };
template<typename T1, typename T2> struct concat;
template<size_t... I1, size_t... I2> struct concat<index_sequence<I1...>, index_sequence<I2...>>: index_sequence<I1..., (sizeof...(I1) + I2)...> {};

template<size_t N> struct make_index_sequence;
template<size_t N> struct make_index_sequence: concat<typename make_index_sequence<N/2>::type, typename make_index_sequence<N-N/2>::type>::type {};
template <> struct make_index_sequence<0>: index_sequence<>{};
template <> struct make_index_sequence<1>: index_sequence<0>{};

/// 
/// helper class to find an element of mpl_type_vector from a Variable type T
template<typename T, typename mpl_type_vector>
struct get_elem_by_type {
    typedef T type;
    typedef typename T::value_type value_type;

    /// 
    /// iter is a boost mpl iterator to the found element in Variable T
    typedef typename mpl::find<mpl_type_vector,T>::type iter;

    /// 
    /// index contains the index of the found element
    static const size_t index = iter::pos::value;
    static_assert(index < mpl::size<mpl_type_vector>::type::value,"variable not found in particle");
};


/// helper class to find an element of mpl_type_vector from an
/// unsigned int index I
template<unsigned int I, typename mpl_type_vector>
struct get_elem_by_index {
    BOOST_MPL_ASSERT_RELATION( (mpl::size<mpl_type_vector>::type::value), >, I );
    typedef typename mpl::at<mpl_type_vector,mpl::int_<I> > type;

    /// 
    /// value_type is the variable's value_type at index I 
    typedef typename type::value_type value_type;
    static const size_t index = I;
};

template<typename tuple_type>
struct getter_helper {};

template <typename ... T>
struct getter_helper<tuple_ns::tuple<T ...>> {
    typedef typename tuple_ns::tuple<T...> tuple_type; 
    template <unsigned int N>
    using return_type = tuple_ns::tuple_element<N,tuple_type>;
    template <unsigned int N, typename T2>
    static inline auto get(T2&& tuple) -> decltype(tuple_ns::get<N>(tuple)) { 
        return tuple_ns::get<N>(tuple);
    }
};

/*

template <typename ... T>
struct getter_helper<std::tuple<T ...>> {
    typedef typename std::tuple<T...> tuple_type; 
    template <unsigned int N>
    using return_type = std::tuple_element<N,tuple_type>;
    template <unsigned int N, typename T2>
    static inline auto get(T2&& tuple) -> decltype(std::get<N>(tuple)) { 
        return std::get<N>(tuple);
    }
};

#ifdef HAVE_THRUST
template <typename ... T>
struct getter_helper<thrust::tuple<T ...>> {
    typedef thrust::tuple<T...> tuple_type; 
    template <unsigned int N>
    using return_type = thrust::tuple_element<N,tuple_type>;
    template <unsigned int N, typename T2>
    static inline auto get(T2&& tuple) -> decltype(thrust::get<N>(tuple)) { 
        return thrust::get<N>(tuple);
    }
};
#endif
*/

template <typename TUPLE, typename mpl_vector_type> 
struct getter_type{
    typedef TUPLE tuple_type;
    template <typename T>
    using elem_by_type = get_elem_by_type<T,mpl_vector_type>;
    template <typename T>
    using return_type = typename getter_helper<tuple_type>::template return_type<elem_by_type<T>::index>;


    getter_type() {}
    explicit getter_type(const tuple_type& data):data(data) {}
    getter_type(const getter_type& other):data(other.data) {}
    getter_type(getter_type&& other):data(std::move(other.data)) {}
    //getter_type(const reference& other):data(other.data) {}

    /*
    template<typename... _UElements, typename = typename
        enable_if<__and_<is_convertible<const _UElements&,
					_Elements>...>::value>::type>
        constexpr tuple(const tuple<_UElements...>& __in)
        : _Inherited(static_cast<const _Tuple_impl<0, _UElements...>&>(__in))
        { }

      template<typename... _UElements, typename = typename
        enable_if<__and_<is_convertible<_UElements,
					_Elements>...>::value>::type>
        constexpr tuple(tuple<_UElements...>&& __in)
        : _Inherited(static_cast<_Tuple_impl<0, _UElements...>&&>(__in)) { }
        */


    template <typename tuple_type2, typename = typename
	//std::enable_if<std::__and_<std::is_convertible<const T2&, tuple_elements>...>::value>::type>
    //std::enable_if<getter_helper<tuple_type>::template is_convertible<const T2&...>::value>::type>
    std::enable_if<std::is_convertible<tuple_type2,tuple_type>::value>::type>
    getter_type(const getter_type<tuple_type2,mpl_vector_type>& other):data(other.data) {}

    template <typename tuple_type2, typename = typename
	//std::enable_if<std::__and_<std::is_convertible<T2, tuple_elements>...>::value>::type>
    //std::enable_if<getter_helper<tuple_type>::template is_convertible<T2...>::value>::type>
    std::enable_if<std::is_convertible<tuple_type2,tuple_type>::value>::type>
    getter_type(const getter_type<tuple_type2,mpl_vector_type>&& other):data(std::move(other.data)) {}

    template <typename T1, typename T2, typename... T3>
    getter_type(T1&& arg1, T2&& arg2, T3&&... args):data(std::forward<T1>(arg1),std::forward<T2>(arg2),std::forward<T3>(args)...) {}

    /*
#ifdef HAVE_THRUST
    template <typename T1, typename... T2,typename = typename
    std::enable_if<std::is_convertible<thrust::tuple<T2...>,tuple_type>::value>::type>
    getter_type(const getter_type<thrust::tuple<T2...>,T1>& other):data(other.data) {}

    template <typename T1, typename... T2,typename = typename
    std::enable_if<std::is_convertible<thrust::tuple<T2...>,tuple_type>::value>::type>
    getter_type(const getter_type<thrust::tuple<T2...>,T1>&& other):data(std::move(other.data)) {}
#endif
*/

    getter_type& operator=( const getter_type& other ) {
        data = other.data;
        return *this;
    }
    getter_type& operator=( getter_type&& other ) {
        data = std::move(other.data);
        return *this;
    }
    template <typename T1, typename T2> 
    getter_type& operator=( const getter_type<T1,T2>& other) {
        data = other.data;
        return *this;
    }
    template <typename T1, typename T2> 
    getter_type& operator=( getter_type<T1,T2>&& other) {
        data = std::move(other.data);
        return *this;
    }
    
    void swap(getter_type &other) {
        data.swap(other.data);
    }

    /*
    template<typename T>
    const typename return_type<T>::type & 
    get_by_type() const {
        return getter_helper<tuple_type>::get<elem_by_type<T>::index>(data);        
    }

    template<typename T>
    typename return_type<T>::type & 
    get_by_type() {
        return getter_helper<tuple_type>::get<elem_by_type<T>::index>(data);        
    }

    template<unsigned int N>
    const typename getter_helper<tuple_type>::template return_type<N>::type &
    get_by_index() const {
        return getter_helper<tuple_type>::get<N>(data);        
    }

    template<unsigned int N>
    typename getter_helper<tuple_type>::template return_type<N>::type &
    get_by_index() {
        return getter_helper<tuple_type>::get<N>(data);        
    }
    */

    const tuple_type & get_tuple() const { return data; }
    tuple_type & get_tuple() { return data; }
            
    tuple_type data;
};


template <typename tuple_type, typename mpl_vector_type> 
void swap(getter_type<tuple_type,mpl_vector_type> x,
          getter_type<tuple_type,mpl_vector_type> y) {
    x.swap(y);
}




template<typename tuple_of_iterators>
struct zip_helper {};


/*

template <typename ... T>
struct zip_helper<std::tuple<T ...>> {
    typedef std::false_type is_thrust;
    typedef std::tuple<T...> tuple_iterator_type; 
    typedef std::tuple<typename std::iterator_traits<T>::value_type ...> tuple_value_type; 
    typedef std::tuple<typename std::iterator_traits<T>::reference ...> tuple_reference; 
    typedef typename std::tuple<T...> iterator_tuple_type;
    template <unsigned int N>
    using tuple_element = std::tuple_element<N,iterator_tuple_type>;
    typedef typename std::iterator_traits<typename tuple_element<0>::type>::difference_type difference_type;
    typedef typename std::iterator_traits<typename tuple_element<0>::type>:: iterator_category iterator_category;
    typedef make_index_sequence<std::tuple_size<iterator_tuple_type>::value> index_type;
    template <unsigned int N, typename T2>
    static inline auto get(T2&& tuple) -> decltype(std::get<N>(tuple)) { 
        return std::get<N>(tuple);
    }
};
*/

#ifdef HAVE_THRUST
}
namespace thrust {
    struct iterator_traits<thrust::null_type> {
        typedef thrust::null_type value_type;
        typedef thrust::null_type reference;
    };
}
namespace Aboria {
    /*
template <typename ... T>
struct zip_helper<thrust::tuple<T ...>> {
    typedef std::true_type is_thrust;
    typedef thrust::tuple<T...> tuple_iterator_type; 
    typedef thrust::tuple<typename thrust::iterator_traits<T>::value_type ...> tuple_value_type; 
    typedef thrust::tuple<typename thrust::iterator_traits<T>::reference ...> tuple_reference; 
    typedef typename thrust::tuple<T...> iterator_tuple_type;
    template <unsigned int N>
    using tuple_element = thrust::tuple_element<N,iterator_tuple_type>;
    typedef typename thrust::iterator_traits<typename tuple_element<0>::type>::difference_type difference_type;
    typedef typename thrust::iterator_traits<typename tuple_element<0>::type>:: iterator_category iterator_category;
    typedef make_index_sequence<thrust::tuple_size<iterator_tuple_type>::value> index_type;
    template <unsigned int N, typename T2>
    CUDA_HOST_DEVICE
    static inline auto get(T2&& tuple) -> decltype(thrust::get<N>(tuple)) { 
        return thrust::get<N>(tuple);
    }
};
*/
#endif

template <typename ... T>
struct zip_helper<tuple_ns::tuple<T ...>> {
    typedef std::false_type is_thrust;
    typedef tuple_ns::tuple<T...> tuple_iterator_type; 
    typedef tuple_ns::tuple<typename tuple_ns::iterator_traits<T>::value_type ...> tuple_value_type; 
    typedef tuple_ns::tuple<typename tuple_ns::iterator_traits<T>::reference ...> tuple_reference; 
    typedef tuple_ns::tuple<typename tuple_ns::iterator_traits<T>::pointer...> tuple_pointer; 
    typedef tuple_ns::tuple<typename tuple_ns::iterator_traits<T>::value_type*...> tuple_raw_pointer; 
    typedef typename tuple_ns::tuple<T...> iterator_tuple_type;
    template <unsigned int N>
    using tuple_element = tuple_ns::tuple_element<N,iterator_tuple_type>;
    typedef typename tuple_ns::iterator_traits<typename tuple_element<0>::type>::difference_type difference_type;
    typedef typename tuple_ns::iterator_traits<typename tuple_element<0>::type>:: iterator_category iterator_category;
    typedef make_index_sequence<tuple_ns::tuple_size<iterator_tuple_type>::value> index_type;
    template <unsigned int N, typename T2>
    CUDA_HOST_DEVICE
    static inline auto get(T2&& tuple) -> decltype(tuple_ns::get<N>(tuple)) { 
        return tuple_ns::get<N>(tuple);
    }
};



namespace detail {
    template <typename iter>
    CUDA_HOST_DEVICE
    typename std::iterator_traits<iter>::difference_type distance(const iter& from, const iter& to, std::random_access_iterator_tag) {
#ifdef __CUDA_ARCH__
        assert(false); 
        return 0; 
#else
        return to-from;
#endif
    }

#ifdef HAVE_THRUST
    template <typename iter>
    CUDA_HOST_DEVICE
    typename thrust::iterator_traits<iter>::difference_type distance(const iter& from, const iter& to, thrust::random_access_device_iterator_tag) {
        return to-from;
    }
#endif

template <typename iter>
    CUDA_HOST_DEVICE
    bool equal(const iter& from, const iter& to, std::random_access_iterator_tag) {
#ifdef __CUDA_ARCH__
        assert(false); 
        return false;
#else
        return to == from;
#endif
    }

#ifdef HAVE_THRUST
    template <typename iter>
    CUDA_HOST_DEVICE
    bool equal(const iter& from, const iter& to, thrust::random_access_device_iterator_tag) {
        return to == from;
    }
#endif

template <typename iter>
    CUDA_HOST_DEVICE
    inline void increment(const iter& arg, std::random_access_iterator_tag) {
#ifdef __CUDA_ARCH__
        assert(false); 
#else
        ++arg;
#endif
    }

#ifdef HAVE_THRUST
    template <typename iter>
    CUDA_HOST_DEVICE
    inline void increment(const iter& arg, thrust::random_access_device_iterator_tag) {
        ++arg;
    }
#endif


template <typename iter>
    CUDA_HOST_DEVICE
    inline void decrement(const iter& arg, std::random_access_iterator_tag) {
#ifdef __CUDA_ARCH__
        assert(false); 
#else
        --arg;
#endif
    }

#ifdef HAVE_THRUST
    template <typename iter>
    CUDA_HOST_DEVICE
    inline void decrement(const iter& arg, thrust::random_access_device_iterator_tag) {
        --arg;
    }
#endif

template <typename iter>
    CUDA_HOST_DEVICE
    inline void advance(const iter& arg, const typename std::iterator_traits<iter>::difference_type& n, std::random_access_iterator_tag) {
#ifdef __CUDA_ARCH__
        assert(false); 
#else
        arg += n;
#endif
    }

#ifdef HAVE_THRUST
    template <typename iter>
    CUDA_HOST_DEVICE
    inline void advance(const iter& arg, const typename thrust::iterator_traits<iter>::difference_type& n, thrust::random_access_device_iterator_tag) {
        arg += n;
    }
#endif


    namespace zip_iterator {

    template<typename reference, typename iterator_tuple_type, std::size_t... I>
    CUDA_HOST_DEVICE
    static reference make_reference(const iterator_tuple_type& tuple, index_sequence<I...>, std::true_type) {
        return reference(*(zip_helper<iterator_tuple_type>::get<I>(tuple))...);
    }

    template<typename reference, typename iterator_tuple_type, std::size_t... I>
    static reference make_reference(const iterator_tuple_type& tuple, index_sequence<I...>, std::false_type) {
        return reference(*(zip_helper<iterator_tuple_type>::get<I>(tuple))...);
    }

    template<typename iterator_tuple_type, std::size_t... I>
    CUDA_HOST_DEVICE
    static void increment_impl(iterator_tuple_type& tuple, index_sequence<I...>) {
        //using expander = int[];
        //(void)expander { 0, (++std::get<I>(tuple),0)...};
        int dummy[] = { 0, (detail::increment(
                                zip_helper<iterator_tuple_type>::get<I>(tuple),
                                zip_helper<iterator_tuple_type>::iterator_category()),0)...};
        static_cast<void>(dummy);
    }

    template<typename iterator_tuple_type, std::size_t... I>
    CUDA_HOST_DEVICE
    static void decrement_impl(iterator_tuple_type& tuple, index_sequence<I...>) {
        int dummy[] = { 0, (detail::decrement(
                                zip_helper<iterator_tuple_type>::get<I>(tuple),
                                zip_helper<iterator_tuple_type>::iterator_category()),void(),0)...};
        static_cast<void>(dummy);
    }

    template<typename iterator_tuple_type, std::size_t... I>
    CUDA_HOST_DEVICE
    static void advance_impl(iterator_tuple_type& tuple, 
            const typename zip_helper<iterator_tuple_type>::difference_type n,  index_sequence<I...>) {
        int dummy[] = { 0, (detail::advance(
                                zip_helper<iterator_tuple_type>::get<I>(tuple),n,
                                zip_helper<iterator_tuple_type>::iterator_category()
                                ),void(),0)...};
        static_cast<void>(dummy);
    }
    
    }
}



template <typename iterator_tuple_type, typename mpl_vector_type=mpl::vector<int>>
class zip_iterator: public boost::iterator_facade<zip_iterator<iterator_tuple_type,mpl_vector_type>,
    getter_type<typename zip_helper<iterator_tuple_type>::tuple_value_type,mpl_vector_type>,
    boost::random_access_traversal_tag,
    getter_type<typename zip_helper<iterator_tuple_type>::tuple_reference,mpl_vector_type>
        > {

public:
    typedef iterator_tuple_type tuple_type;
    typedef getter_type<typename zip_helper<iterator_tuple_type>::tuple_value_type,mpl_vector_type> value_type;
    typedef getter_type<typename zip_helper<iterator_tuple_type>::tuple_reference,mpl_vector_type> reference;
    typedef getter_type<typename zip_helper<iterator_tuple_type>::tuple_pointer,mpl_vector_type> pointer;
    typedef getter_type<typename zip_helper<iterator_tuple_type>::tuple_raw_pointer,mpl_vector_type> raw_pointer;
    typedef typename zip_helper<iterator_tuple_type>::difference_type difference_type;

    template <typename T>
    using elem_by_type = get_elem_by_type<T,mpl_vector_type>;

    template<typename T>
    struct return_type {
        static const size_t N = elem_by_type<T>::index;
        typedef const typename zip_helper<iterator_tuple_type>::template tuple_element<N>::type type;
    };

    CUDA_HOST_DEVICE
    zip_iterator() {}

    CUDA_HOST_DEVICE
    explicit zip_iterator(iterator_tuple_type iter) : iter(iter) {}

    template <typename ...T>
    CUDA_HOST_DEVICE
    explicit zip_iterator(T... args) : iter(args...) {}

    CUDA_HOST_DEVICE
    const iterator_tuple_type & get_tuple() const { return iter; }

    CUDA_HOST_DEVICE
    iterator_tuple_type & get_tuple() { return iter; }

private:

    typedef typename zip_helper<iterator_tuple_type>::index_type index_type;

    CUDA_HOST_DEVICE
    void increment() { 
        detail::zip_iterator::increment_impl(iter,index_type()); 
    }
    
    CUDA_HOST_DEVICE
    void decrement() { detail::zip_iterator::decrement_impl(iter,index_type()); }

    CUDA_HOST_DEVICE
    bool equal(zip_iterator const& other) const { 
        return detail::equal(
                zip_helper<iterator_tuple_type>::get<0>(other.iter),
                zip_helper<iterator_tuple_type>::get<0>(iter),
                zip_helper<iterator_tuple_type>::iterator_category());
    }

    CUDA_HOST_DEVICE
    reference dereference() const { return detail::zip_iterator::make_reference<reference>(iter,index_type(),std::false_type()); }

    CUDA_HOST_DEVICE
    difference_type distance_to(zip_iterator const& other) const { 
        return detail::distance(
                zip_helper<iterator_tuple_type>::get<0>(other.iter),
                zip_helper<iterator_tuple_type>::get<0>(iter),
                zip_helper<iterator_tuple_type>::iterator_category());
    }

    CUDA_HOST_DEVICE
    void advance(difference_type n) { detail::zip_iterator::advance_impl(iter,n,index_type()); }

    iterator_tuple_type iter;
    friend class boost::iterator_core_access;
};

/*
template <typename mpl_vector_type, typename ... tuple_args>
class zip_iterator<thrust::tuple<tuple_args...>,mpl_vector_type>: public boost::iterator_facade<zip_iterator<thrust::tuple<tuple_args...>,mpl_vector_type>,
    getter_type<typename zip_helper<thrust::tuple<tuple_args...>>::tuple_value_type,mpl_vector_type>,
    boost::random_access_traversal_tag,
    getter_type<typename zip_helper<thrust::tuple<tuple_args...>>::tuple_reference,mpl_vector_type>
        > {

public:
    typedef thrust::tuple<tuple_args...> iterator_tuple_type;
    typedef getter_type<typename zip_helper<iterator_tuple_type>::tuple_value_type,mpl_vector_type> value_type;
    typedef getter_type<typename zip_helper<iterator_tuple_type>::tuple_reference,mpl_vector_type> reference;
    typedef typename zip_helper<iterator_tuple_type>::difference_type difference_type;

    template <typename T>
    using elem_by_type = get_elem_by_type<T,mpl_vector_type>;

    template<typename T>
    struct return_type {
        static const size_t N = elem_by_type<T>::index;
        typedef const typename zip_helper<iterator_tuple_type>::template tuple_element<N>::type type;
    };

    CUDA_HOST_DEVICE
    zip_iterator() {}

    CUDA_HOST_DEVICE
    explicit zip_iterator(iterator_tuple_type iter) : iter(iter) {}

    template <typename ...T>
    CUDA_HOST_DEVICE
    explicit zip_iterator(T... args) : iter(args...) {}

    CUDA_HOST_DEVICE
    const iterator_tuple_type & get_tuple() const { return iter; }

    CUDA_HOST_DEVICE
    iterator_tuple_type & get_tuple() { return iter; }

private:

    typedef typename zip_helper<iterator_tuple_type>::index_type index_type;

    CUDA_HOST_DEVICE
    void increment() { detail::zip_iterator::increment_impl(iter,index_type(),std::true_type()); }
    
    CUDA_HOST_DEVICE
    void decrement() { detail::zip_iterator::decrement_impl(iter,index_type(),std::true_type()); }

    CUDA_HOST_DEVICE
    bool equal(zip_iterator const& other) const { return zip_helper<iterator_tuple_type>::get<0>(iter) == zip_helper<iterator_tuple_type>::get<0>(other.iter); }

    CUDA_HOST_DEVICE
    reference dereference() const { return detail::zip_iterator::make_reference<reference>(iter,index_type(),std::true_type()); }

    CUDA_HOST_DEVICE
    difference_type distance_to(zip_iterator const& other) const { return zip_helper<iterator_tuple_type>::get<0>(other.iter) - zip_helper<iterator_tuple_type>::get<0>(iter); }

    CUDA_HOST_DEVICE
    void advance(difference_type n) { detail::zip_iterator::advance_impl(iter,n,index_type(),std::true_type()); }

    iterator_tuple_type iter;
    friend class boost::iterator_core_access;
};
*/


template <typename T>
struct is_zip_iterator {
    typedef mpl::bool_<false> type;
    static const bool value = false; 
};

template <typename tuple_type, typename mpl_vector_type>
struct is_zip_iterator<zip_iterator<tuple_type,mpl_vector_type>> {
    typedef mpl::bool_<true> type;
    static const bool value = true; 
};


//
// Particle getters/setters
//

/// get a variable from a particle \p arg
/// \param T Variable type
/// \param arg the particle
/// \return a const reference to a T::value_type holding the variable data
///
template<typename T, typename value_type>
typename value_type::template return_type<T>::type const & 
get(const value_type& arg) {
    //std::cout << "get const reference" << std::endl;
    return getter_helper<typename value_type::tuple_type>::get<value_type::template elem_by_type<T>::index>(arg.get_tuple());
    //return arg.template get<T>();
}

template<typename T, typename value_type>
typename value_type::template return_type<T>::type & 
get(value_type& arg) {
    //std::cout << "get reference" << std::endl;
    return getter_helper<typename value_type::tuple_type>::get<value_type::template elem_by_type<T>::index>(arg.get_tuple());
    //return arg.template get<T>();
}

template<typename T, typename value_type>
typename value_type::template return_type<T>::type & 
get(value_type&& arg) {
    //std::cout << "get reference" << std::endl;
    return getter_helper<typename value_type::tuple_type>::get<value_type::template elem_by_type<T>::index>(arg.get_tuple());
    //return arg.template get<T>();
}

template<unsigned int N, typename value_type>
const typename getter_helper<typename value_type::tuple_type>::template return_type<N>::type &
get_by_index(const value_type& arg) {
    return getter_helper<typename value_type::tuple_type>::get<N>(arg.get_tuple());
}

template<unsigned int N, typename value_type>
typename getter_helper<typename value_type::tuple_type>::template return_type<N>::type &
get_by_index(value_type& arg) {
    return getter_helper<typename value_type::tuple_type>::get<N>(arg.get_tuple());        
}

template<unsigned int N, typename value_type>
typename getter_helper<typename value_type::tuple_type>::template return_type<N>::type &
get_by_index(value_type&& arg) {
    return getter_helper<typename value_type::tuple_type>::get<N>(arg.get_tuple());
}

/*
template<typename T, typename value_type>
typename value_type::template return_type<T>::type const & 
get(const value_type& arg) {
//auto get(value_type&& arg)
//-> decltype(copy_to_host(std::get<std::remove_reference<value_type>::type::template elem_by_type<T>::index>(arg.get_tuple()))) {
     typedef typename value_type::type::template return_type<T>::type element_type;
     return copy_to_host(
             (element_type)
                 std::get<value_type::template elem_by_type<T>::index>(arg.get_tuple()));        
}

template<typename T, typename value_type>
typename value_type::template return_type<T>::type const & 
get(const value_type& arg) {
//auto get(value_type&& arg)
//-> decltype(copy_to_host(std::get<std::remove_reference<value_type>::type::template elem_by_type<T>::index>(arg.get_tuple()))) {
     typedef typename value_type::type::template return_type<T>::type element_type;
     return copy_to_host(
             (element_type)
                 std::get<value_type::template elem_by_type<T>::index>(arg.get_tuple()));        
}

template<typename T, typename value_type>
typename std::remove_reference<value_type>::type::template return_type<T>::type & 
get(value_type& arg) {
//auto get(value_type&& arg)
//-> decltype(copy_to_host(std::get<std::remove_reference<value_type>::type::template elem_by_type<T>::index>(arg.get_tuple()))) {
     return copy_to_host(std::get<std::remove_reference<value_type>::type::template elem_by_type<T>::index>(arg.get_tuple()));        
}

template<typename T, typename value_type>
auto get(value_type&& arg)
-> decltype(std::get<std::remove_reference<value_type>::type::template elem_by_type<T>::index>(arg.get_tuple())) {
     return std::get<std::remove_reference<value_type>::type::template elem_by_type<T>::index>(arg.get_tuple());        
}

*/


}




#endif //GET_H_