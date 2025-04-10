#pragma once
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <map>
#include <string>
#include <type_traits>

/**
 * @file serializable.h
 * @brief Serialization interfaces and a factory for serailizables
 */

namespace ygl {

/**
 * @brief Serializable abstract class. No virtual functions, use only when types can be resolved statically.
 */
class Serializable {
   public:
	/**
	 * @brief Serialize to stream
	 *
	 * @param out - stream to write to
	 */
	void serialize(std::ostream &out);
	/**
	 * @brief Read from stream
	 *
	 * @param in - stream to read from
	 */
	void deserialize(std::istream &in);
};

/**
 * @brief A class that can read 'itself' several times. For example, the ECS Scene should be able to read several files
 * while appending to itself.
 */
class AppendableSerializable {
   public:
	/**
	 * @brief Write data to stream
	 *
	 * @param out - stream to write to
	 */
	virtual void write(std::ostream &out) = 0;
	/**
	 * @brief Read data from stream
	 *
	 * @param in - stream to read from
	 */
	virtual void read(std::istream &in) = 0;
	virtual ~AppendableSerializable()	= default;
};

/**
 * @brief A Serializable. Can be written into a stream.
 */
class ISerializable {
   public:
	/**
	 * @brief Write to stream
	 *
	 * @param out - stream to write to
	 */
	virtual void serialize(std::ostream &out) = 0;
	virtual ~ISerializable()				  = default;
};

/**
 * @brief A Resource extends ISerializable, has static const char* member name and has a constructor that accepts
 * std::istream&. It can be written to a stream and is constructed from it. ResourceFactory indexes resources by their
 * names.
 *
 * @tparam T [TODO:description]
 * @return [TODO:description]
 */
template <class T>
concept IsResource = std::is_base_of<ISerializable, T>::value && requires(T, std::istream &in) {
																		  { T::name } -> std::same_as<const char *&>;
																		  { new T(in) };
																	  };

/**
 * @brief A Factory for Resources
 */
class ResourceFactory {
	using ResourceConstructor = std::function<ISerializable *(std::istream &)>;

	static std::map<std::string, ResourceConstructor> fabricators;

   public:
	static void init();		///< initialization. Automatically called by ygl::init()
	/**
	 * @brief Produces an ISerializable reading from \a in.
	 *
	 * @param in - stream to read from
	 */
	static ISerializable *fabricate(std::istream &in);
	/**
	 * @brief Registers a resource that then can be fabricated.
	 *
	 * @param name - name of the resource
	 * @param f - function that constructs a resource, reading from a file
	 */
	static void registerResource(const std::string &name, ResourceConstructor f);

	/**
	 * @brief Registers a resource that then can be fabricated.
	 *
	 * @tparam T - a type that satisfies ygl::IsResource<T>
	 */
	template <class T>
		requires IsResource<T>
	static void registerResource() {
		registerResource(T::name, [](std::istream &in) { return new T(in); });
	}
};

}	  // namespace ygl
