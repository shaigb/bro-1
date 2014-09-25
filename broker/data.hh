#ifndef BROKER_DATA_HH
#define BROKER_DATA_HH

#include <broker/util/variant.hh>
#include <broker/util/optional.hh>
#include <broker/util/hash.hh>
#include <cstdint>
#include <string>
#include <set>
#include <map>
#include <vector>

namespace broker {

class data;
using set = std::set<data>;
using vector = std::vector<data>;
using table = std::map<data, data>;

class record : util::totally_ordered<record> {
public:

	using field = util::optional<data>;

	/**
	 * Default construct an empty record.
	 */
	record();

	/**
	 * Construct a record from a list of fields.
	 */
	record(std::vector<field> arg_fields);

	/**
	 * @return the number of fields in the record.
	 */
	size_t size() const;

	/**
	 * @return a const reference to a field at a given offset, if it exists.
	 */
	util::optional<const data&> get(size_t index) const;

	/**
	 * @return a reference to a field at a given offset, if it exists.
	 */
	util::optional<data&> get(size_t index);

	std::vector<field> fields;
};

class data : util::totally_ordered<data> {
public:

	enum class tag : uint8_t {
		// Primitive types
		boolean,  // bool
		integer,  // int64_t
		count,    // uint64_t
		real,     // double
		string,   // std::string
		// TODO: time
		// TODO: interval
		// TODO: enumeration
		// TODO: port
		// TODO: address
		// TODO: subnet

		// Compound types
		set,
		table,
		vector,
		record
	};

	using value_type = util::variant<
	    tag,
	    bool,
	    uint64_t,
	    int64_t,
	    double,
	    std::string,
	    set,
	    table,
	    vector,
	    record
	>;

	template <typename T>
	using from = util::conditional_t<
        std::is_floating_point<T>::value,
        double,
        util::conditional_t<
          std::is_same<T, bool>::value,
          bool,
          util::conditional_t<
            std::is_unsigned<T>::value,
            uint64_t,
            util::conditional_t<
              std::is_signed<T>::value,
              int64_t,
              util::conditional_t<
                std::is_convertible<T, std::string>::value,
                std::string,
                util::conditional_t<
	                 std::is_same<T, set>::value ||
	                 std::is_same<T, table>::value ||
	                 std::is_same<T, vector>::value ||
	                 std::is_same<T, record>::value,
	                 T, std::false_type>
              >
            >
          >
        >
      >;

	template <typename T>
	using type = from<typename std::decay<T>::type>;

	/**
	 * Default construct data.
	 */
	data()
		{}

	/**
	  * Constructs data.
	  * @param x The instance to construct data from.
	  */
	template <typename T,
	          typename = util::disable_if_t<
	              util::is_same_or_derived<data, T>::value ||
	              std::is_same<type<T>, std::false_type>::value>
	         >
	data(T&& x)
	    : value(type<T>(std::forward<T>(x)))
		{}

	value_type value;
};

inline record::record()
	: fields()
	{}

inline record::record(std::vector<field> arg_fields)
	: fields(std::move(arg_fields))
	{}

inline size_t record::size() const
	{ return fields.size(); }

inline util::optional<const data&> record::get(size_t index) const
	{
	if ( index >= fields.size() ) return {};
	if ( ! fields[index] ) return {};
	return *fields[index];
	}

inline util::optional<data&> record::get(size_t index)
	{
	if ( index >= fields.size() ) return {};
	if ( ! fields[index] ) return {};
	return *fields[index];
	}

inline bool operator==(const record& lhs, const record& rhs)
	{ return lhs.fields == rhs.fields; }

inline bool operator<(const record& lhs, const record& rhs)
	{ return lhs.fields < rhs.fields; }

inline data::value_type& expose(data& d)
	{ return d.value; }

inline const data::value_type& expose(const data& d)
	{ return d.value; }

inline bool operator==(const data& lhs, const data& rhs)
	{ return lhs.value == rhs.value; }

inline bool operator<(const data& lhs, const data& rhs)
	{ return lhs.value < rhs.value; }

} // namespace broker

namespace std {
template <> struct hash<broker::data> {
	using value_type = broker::data::value_type;
	using result_type = typename std::hash<value_type>::result_type;
	using argument_type = broker::data;
	inline result_type operator()(const argument_type& d) const
		{ return std::hash<value_type>{}(d.value); }
};
template <> struct hash<broker::set> :
                   broker::util::container_hasher<broker::set> { };
template <> struct hash<broker::vector> :
                   broker::util::container_hasher<broker::vector> { };
template <> struct hash<broker::table::value_type> {
	using result_type = typename std::hash<broker::data>::result_type;
	using argument_type = broker::table::value_type;
	inline result_type operator()(const argument_type& d) const
		{
		result_type rval{};
		broker::util::hash_combine<broker::data>(rval, d.first);
		broker::util::hash_combine<broker::data>(rval, d.second);
		return rval;
		}
};
template <> struct hash<broker::table> :
                   broker::util::container_hasher<broker::table> { };
template <> struct hash<broker::record> {
	using result_type =
	      typename broker::util::container_hasher<
	               decltype(broker::record::fields)>::result_type;
	using argument_type = broker::record;
	inline result_type operator()(const argument_type& d) const
		{
		return broker::util::container_hasher<
		       decltype(broker::record::fields)>{}(d.fields);
		}
};
}

#endif // BROKER_DATA_HH
