#ifndef NOH_H
#define NOH_H

#include "coord.h"

class Noh
{
public:
    Coord pos;
    Coord ant;
    double g;
    double h;

    //construtor
    Noh();
    Noh(Coord p, Coord a, double _g, double _h);

    //destrutor
    ~Noh();

    //função de consulta
    double getCost() const;

    //Operadores
    bool operator==(const Coord& N) const;
};

/// Classe do functor (para busca com parametro)
class maior_que
{
private:
    Noh S;
public:
    inline maior_que(Noh SS): S(SS) {}
    inline bool operator()(const Noh& N)
    {
        return (S.getCost() < N.getCost());
    }
};

#endif
