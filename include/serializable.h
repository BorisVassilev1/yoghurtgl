#pragma once
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <map>
#include <string>
#include <type_traits>

namespace ygl {

class Serializable {
   public:
	void serialize(std::ostream &out);
	void deserialize(std::istream &in);
};

class ISerializable {
   public:
	virtual void serialize(std::ostream &out)  = 0;
	virtual void deserialize(std::istream &in) = 0;
	virtual ~ISerializable()				   = default;
};

template <class T>
concept IsResource =
	std::is_base_of<ygl::ISerializable, T>::value && requires(T, std::istream &in, const std::string &path) {
														 { T::name } -> std::same_as<const char *&>;
														 { new T(in, path) };
													 };

class ResourceFactory {
	using ResourceConstructor = std::function<ygl::ISerializable *(std::istream &, const std::string &)>;

	static std::map<std::string, ResourceConstructor> fabricators;
	
   public:
	static void											  init();
	static ISerializable *fabricate(std::istream &in, const std::string &path);
	static void			  registerResource(const std::string &name, ResourceConstructor f);
};

}	  // namespace ygl
