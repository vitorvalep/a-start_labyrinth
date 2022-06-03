#include "noh.h"

using namespace std;
//Construtores
Noh::Noh(): pos(), ant(), g(0.0), h(0.0) {}
Noh::Noh(Coord p, Coord a, double _g, double _h): pos(p), ant(a),g(_g),h(_h) {}

//Destrutor
Noh::~Noh() {}

//Operadores
bool Noh::operator==(const Coord& N) const
{
    //return pos==N.pos && g==N.g && h==N.h;
    return this->pos==N;
}

//Consulta
double Noh::getCost() const
{
    return g+h;
}
