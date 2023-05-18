#pragma once
#include <fstream>
#include <functional>
#include <iostream>
#include <map>

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

class SerializableFactory {
	using SerializableConstructor = std::function<void(std::istream &, ygl::Serializable *)>;

	static std::map<std::string, SerializableConstructor> fabricators;

   public:
	static void makeSerializable(const std::string &name, std::istream &in, ygl::Serializable *res);
	static void registerSerializable(const std::string &name, SerializableConstructor f);
};

}	  // namespace ygl
