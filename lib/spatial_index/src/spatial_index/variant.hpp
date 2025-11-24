#pragma once

namespace slicer {

//! Helper for building an overloaded set of lambdas
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

//! CTAD guide (C++17 and later)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}
