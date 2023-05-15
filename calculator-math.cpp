#include "calculator.h"
#include <cmath>

double math::geometricSequence(double a1, double r, double n){
    if(n >= 1e9) return a1 / (1 - r);
    return a1*(1 - std::pow(r, n)) / (1 - r);
}

double math::F(double x){
    return std::sqrt(math::geometricSequence(1, 0.81, x)) / math::geometricSequence(1, 0.9, x);
}

double math::f(double x){
    return 1200.0 * (F(x)-F(1e9)) / (F(1)-F(1e9));
}

double math::g(double x){
    return std::pow(2.0, x/800);
}

double math::gInv(double x){
    return 800 * std::log2(x);
}

double math::positivizeRating(double rating){
    if(rating >= 400.0) return rating;
    return 400.0 * std::exp((rating - 400.0) / 400.0);
}