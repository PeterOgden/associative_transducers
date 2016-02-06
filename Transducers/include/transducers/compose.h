#ifndef TRANSDUCERS_COMPOSE_H_
#define TRANSDUCERS_COMPOSE_H_

namespace transducers {
template <template <typename> class NewType, typename OldType, typename... Ts>
NewType<OldType> compose(const OldType& old, Ts&&... args) {
	return NewType<OldType>(old, std::forward<Ts>(args)...);
}

}

#endif
