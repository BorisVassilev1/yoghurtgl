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

class SerializableFactory {
	using SerializableConstructor = std::function<Serializable *(std::istream &)>;

	static std::map<std::string, SerializableConstructor> fabricators;

   public:
	static Serializable *makeSerializable(const std::string &name, std::istream &in);
	static void			 registerSerializable(const std::string &name, SerializableConstructor f);
};

template <class Child>
class ISerializable : public Serializable {
	static bool registered;

   public:
	ISerializable<Child>() {
		if (!ISerializable<Child>::registered)
			SerializableFactory::registerSerializable(typeid(Child).name(), [](std::istream &in) -> Serializable * {
				Child *res = new Child();
				res->deserialize(in);
				return res;
			});
		ISerializable<Child>::registered = true;
	}
};

template <class Child>
bool ygl::ISerializable<Child>::registered = false;

}	  // namespace ygl
