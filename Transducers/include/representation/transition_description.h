#ifndef TRANSDUCERS_REPRESENTATION_TRANSITION_DESCRIPTION_H_
#define TRANSDUCERS_REPRESENTATION_TRANSITION_DESCRIPTION_H_

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <map>
#include <utility>

namespace representation {

// Typedefs to specialise on deterministic or non-deterministic transitions
typedef std::map<std::pair<int, unsigned short>, int> dt_type;
typedef std::multimap<std::pair<int, unsigned short>, int> nt_type;

// Typedefs for the possible types of output symbols
typedef std::multimap<int, int> fsm_type;
typedef std::map<std::pair<int, unsigned short>, int> dft_type;
typedef std::map<std::tuple<int, unsigned short, int>, int> push_dft_type;

template <class MapType, class OutputMapType>
class transition_description {
public:
	friend class boost::serialization::access;

	typedef MapType transition_map;
	typedef std::multimap<std::pair<int, unsigned short>, std::pair<int, int>> pop_map;
	typedef OutputMapType output_map;

	transition_description() {}


	transition_map transitions;
	transition_map push;
	pop_map pop;
	output_map output;
	int start_state;
	int num_states;

	void add_transition(int start, unsigned short c, int end, int push, int output) {

	};
private:
	template <class Archive>
	void serialize(Archive& ar, const unsigned int version) {
		std::string class_name(typeid(*this).name());
		std::string serialized_name;
		ar & serialized_name;
		if (class_name != serialized_name) {
			std::ostringstream stream;
			stream << "Wrong type for serialization. Expected: " << class_name << " got: " << serialized_name;
			throw std::runtime_error(stream.str());
		}

		ar & transitions;
		ar & push;
		ar & pop;
		ar & output;
		ar & start_state;
		ar & num_states;
	}
};

typedef transition_description<dt_type, dft_type> dft_description;

}

#endif
