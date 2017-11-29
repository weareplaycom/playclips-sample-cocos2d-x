
#pragma once

namespace estd {
    using namespace std;

    template <typename C>
    using Value_type = typename C::value_type;

    template<typename C, typename P>
    Value_type<C> find_if(C& c, P p) {
        return std::find_if(c.begin(),
                            c.end(),
                            p);
    }
}
