#include <iomanip>
#include <fstream>
#include <list>
#include <algorithm>
#include <cmath>

#include "labirinto.h"
#include "Noh.h"

using namespace std;

/* ***************** */
/* CLASSE CELULA     */
/* ***************** */

string estadoCel2string(EstadoCel E)
{
    switch(E)
    {
    case EstadoCel::LIVRE:
        return "  ";
    case EstadoCel::OBSTACULO:
        return "##";
    case EstadoCel::ORIGEM:
        return "Or";
    case EstadoCel::DESTINO:
        return "De";
    case EstadoCel::CAMINHO:
        return "..";
    default:
        break;
    }
    return "??";
}

/* ***************** */
/* CLASSE LABIRINTO  */
/* ***************** */

/// Construtores

/// Default (labirinto vazio)
Labirinto::Labirinto(): NL(0), NC(0), mapa(), orig(), dest() {}

/// Cria um mapa com dimensoes dadas
/// numL e numC sao as dimensoes do labirinto
Labirinto::Labirinto(unsigned numL, unsigned numC)
{
    gerar(numL, numC);
}

/// Cria um mapa com o conteudo do arquivo nome_arq
/// Caso nao consiga ler do arquivo, cria mapa vazio
Labirinto::Labirinto(const string& nome_arq)
{
    ler(nome_arq);
}

/// Destrutor (nao eh obrigatorio...)
Labirinto::~Labirinto() {}

/// Torna o mapa vazio
void Labirinto::clear()
{
    // Esvazia o mapa de qualquer conteudo anterior
    NL = NC = 0;
    mapa.clear();
    // Apaga a origem e destino do caminho
    orig = dest = Coord();
}

/// Limpa o caminho anterior
void Labirinto::limpaCaminho()
{
    if (!empty()) for (unsigned i=0; i<NL; i++) for (unsigned j=0; j<NC; j++)
            {
                if (at(i,j) == EstadoCel::CAMINHO)
                {
                    set(i,j, EstadoCel::LIVRE);
                }
            }
}

/// Funcoes de consulta
unsigned Labirinto::getNumLin() const
{
    return NL;
}

unsigned Labirinto::getNumCol() const
{
    return NC;
}

Coord Labirinto::getOrig() const
{
    return orig;
}

Coord Labirinto::getDest() const
{
    return dest;
}

/// Funcao de consulta
/// Retorna o estado da celula correspondente ao i-j-esimo elemento do mapa
EstadoCel Labirinto::at(unsigned i, unsigned j) const
{
    return mapa.at(NC*i+j);
}

/// Retorna o estado da celula C
EstadoCel Labirinto::at(const Coord& C) const
{
    return at(C.lin, C.col);
}

/// Operador() de consulta - usa o metodo "at"
/// Retorna o estado da celula correspondente ao i-j-esimo elemento do mapa
EstadoCel Labirinto::operator()(unsigned i, unsigned j) const
{
    return at(i,j);
}

/// Retorna o estado da celula C
inline EstadoCel Labirinto::operator()(const Coord& C) const
{
    return at(C);
}

/// Funcao set de alteracao de valor
void Labirinto::set(unsigned i, unsigned j, EstadoCel valor)
{
    mapa.at(NC*i+j) = valor;
}

void Labirinto::set(const Coord& C, EstadoCel valor)
{
    set(C.lin, C.col, valor);
}

/// Testa se um mapa estah vazio
bool Labirinto::empty() const
{
    return mapa.empty();
}

/// Testa se um mapa tem origem e destino definidos
bool Labirinto::origDestDefinidos() const
{
    return celulaLivre(orig) && celulaLivre(dest);
}

/// Testa se uma celula eh valida dentro de um mapa
bool Labirinto::coordValida(const Coord& C) const
{
    if (!C.valida()) return false;
    if (C.lin >= int(NL)) return false;
    if (C.col >= int(NC)) return false;
    return true;
}

/// Testa se uma celula estah livre (nao eh obstaculo) em um mapa
bool Labirinto::celulaLivre(const Coord& C) const
{
    if (!coordValida(C)) return false;
    if (at(C) == EstadoCel::OBSTACULO) return false;
    return true;
}

/// Testa se um movimento Orig->Dest eh valido
bool Labirinto::movimentoValido(const Coord& Orig, const Coord& Dest) const
{
    // Soh pode mover de e para celulas livres
    if (!celulaLivre(Orig)) return false;
    if (!celulaLivre(Dest)) return false;

    // Soh pode mover para celulas vizinhas
    Coord delta=abs(Dest-Orig);
    if (delta.lin>1 || delta.col>1) return false;

    // Nao pode mover em diagonal se colidir com alguma quina
    // Se o movimento nao for diagonal, esses testes sempre dao certo,
    // pois jah testou que Orig e Dest estao livres e ou a linha ou a
    // coluna de Orig e Dest sao iguais
    if (!celulaLivre(Coord(Orig.lin,Dest.col))) return false;
    if (!celulaLivre(Coord(Dest.lin,Orig.col))) return false;

    // Movimento valido
    return true;
}

/// Fixa a origem do caminho a ser encontrado
bool Labirinto::setOrigem(const Coord& C)
{
    if (!celulaLivre(C)) return false;
    // Se for a mesma origen nao faz nada
    if (C==orig) return true;

    limpaCaminho();

    // Apaga a origem anterior no mapa, caso esteja definida
    if (coordValida(orig)) set(orig, EstadoCel::LIVRE);

    // Fixa a nova origem
    orig = C;
    // Marca a nova origem no mapa
    set(orig, EstadoCel::ORIGEM);

    return true;
}

/// Fixa o destino do caminho a ser encontrado
bool Labirinto::setDestino(const Coord& C)
{
    if (!celulaLivre(C)) return false;
    // Se for o mesmo destino nao faz nada
    if (C==dest) return true;

    limpaCaminho();

    // Apaga o destino anterior no mapa, caso esteja definido
    if (coordValida(dest)) set(dest, EstadoCel::LIVRE);

    // Fixa o novo destino
    dest = C;
    // Marca o novo destino no mapa
    set(dest, EstadoCel::DESTINO);

    return true;
}

/// Imprime o mapa no console
void Labirinto::imprimir() const
{
    if (empty())
    {
        cout << "+------------+" << endl;
        cout << "| MAPA VAZIO |" << endl;
        cout << "+------------+" << endl;
        return;
    }

    unsigned i,j;

    // Impressao do cabecalho
    cout << "    ";
    for (j=0; j<NC; j++)
    {
        cout << setfill('0') << setw(2) << j << setfill(' ') << setw(0) << ' ' ;
    }
    cout << endl;

    cout << "   +";
    for (j=0; j<NC; j++) cout << "--+" ;
    cout << endl;

    // Imprime as linhas
    for (i=0; i<NL; i++)
    {
        cout << setfill('0') << setw(2) << i << setfill(' ') << setw(0) << " |" ;
        for (j=0; j<NC; j++)
        {
            cout << estadoCel2string(at(i,j)) << '|' ;
        }
        cout << endl;

        cout << "   +";
        for (j=0; j<NC; j++) cout << "--+" ;
        cout << endl;
    }
}

/// Leh um mapa do arquivo nome_arq
/// Caso nao consiga ler do arquivo, cria mapa vazio
/// Retorna true em caso de leitura bem sucedida
bool Labirinto::ler(const string& nome_arq)
{
    // Limpa o mapa
    clear();

    // Abre o arquivo
    ifstream arq(nome_arq.c_str());
    if (!arq.is_open())
    {
        return false;
    }

    string prov;
    int numL, numC;
    int valor;

    // Leh o cabecalho
    arq >> prov >> numL >> numC;
    if (prov != "LABIRINTO" ||
            numL<ALTURA_MIN_MAPA || numL>ALTURA_MAX_MAPA ||
            numC<LARGURA_MIN_MAPA || numC>LARGURA_MAX_MAPA)
    {
        arq.close();
        return false;
    }

    // Redimensiona o mapa
    NL = numL;
    NC = numC;
    mapa.resize(NL*NC);

    // Leh as celulas do arquivo
    for (unsigned i=0; i<NL; i++)
        for (unsigned j=0; j<NC; j++)
        {
            arq >> valor;
            if (valor == 0) set(i,j,EstadoCel::OBSTACULO);
            else set(i,j,EstadoCel::LIVRE);
        }
    arq.close();
    return true;
}

/// Salva um mapa no arquivo nome_arq
/// Retorna true em caso de escrita bem sucedida
bool Labirinto::salvar(const string& nome_arq) const
{
    // Testa o mapa
    if (empty()) return false;

    // Abre o arquivo
    ofstream arq(nome_arq.c_str());
    if (!arq.is_open())
    {
        return false;
    }

    // Salva o cabecalho
    arq << "LABIRINTO " << NL << ' ' << NC << endl;

    // Salva as celulas do mapa
    for (unsigned i=0; i<NL; i++)
    {
        for (unsigned j=0; j<NC; j++)
        {
            if (at(i,j) == EstadoCel::OBSTACULO) arq << 0;
            else arq << 1;
            arq << ' ';
        }
        arq << endl;
    }

    arq.close();
    return true;
}

/// Gera um novo mapa aleatorio
/// numL e numC sao as dimensoes do labirinto
/// perc_obst eh o percentual de casas ocupadas no mapa. Se <=0, assume um valor aleatorio
/// entre PERC_MIN_OBST e PERC_MAX_OBST
/// Se os parametros forem incorretos, gera um mapa vazio
/// Retorna true em caso de geracao bem sucedida (parametros corretos)
bool Labirinto::gerar(unsigned numL, unsigned numC, double perc_obst)
{
    // Limpa o mapa
    clear();

    // Inicializa a semente de geracao de numeros aleatorios
    srand(time(nullptr));

    // Calcula o percentual de obstaculos no mapa
    if (perc_obst <= 0.0)
    {
        perc_obst = PERC_MIN_OBST +
                    (PERC_MAX_OBST-PERC_MIN_OBST)*(rand()/double(RAND_MAX));
    }

    // Testa os parametros
    if (numL<ALTURA_MIN_MAPA || numL>ALTURA_MAX_MAPA ||
            numC<LARGURA_MIN_MAPA || numC>LARGURA_MAX_MAPA ||
            perc_obst<PERC_MIN_OBST || perc_obst>PERC_MAX_OBST)
    {
        return false;
    }

    // Assume as dimensoes passadas como parametro
    NL = numL;
    NC = numC;

    // Redimensiona o mapa
    mapa.resize(NL*NC);

    // Preenche o mapa
    bool obstaculo;
    for (unsigned i=0; i<NL; i++) for (unsigned j=0; j<NC; j++)
        {
            obstaculo = (rand()/double(RAND_MAX) <= perc_obst);
            if (obstaculo) set(i,j,EstadoCel::OBSTACULO);
            else set(i,j,EstadoCel::LIVRE);
        }
    return true;
}


///Heuristica
double Labirinto::Heuristica(const Coord& ori, const Coord& de) const
{
    Coord D = abs(de - ori);
    return (sqrt(2)*min(D.lin, D.col) + fabs(D.lin - D.col));
}

/// Calcula o caminho entre a origem e o destino do labirinto usando o algoritmo A*
///
/// Retorna o comprimento do caminho (<0 se nao existe)
///
/// O parametro NC retorna o numero de nos no caminho encontrado (profundidade da busca)
/// O parametro NC retorna <0 caso nao exista caminho.
///
/// O parametro NA retorna o numero de nos em aberto ao termino do algoritmo A*
/// O parametro NF retorna o numero de nos em fechado ao termino do algoritmo A*
/// Mesmo quando nao existe caminho, esses parametros devem ser retornados
double Labirinto::calculaCaminho(int& NC, int& NA, int& NF)
{
    if (empty() || !origDestDefinidos())
    {
        // Impossivel executar o algoritmo
        NC = NA = NF = -1;
        return -1.0;
    }

    // Apaga um eventual caminho anterior
    limpaCaminho();

    // Testa se origem igual a destino
    if (orig==dest)
    {
        // Caminho tem profundidade nula
        NC = 0;
        // Algoritmo de busca nao gerou nenhum noh
        NA = NF = 0;
        // Caminho tem comprimento nulo
        return 0.0;
    }

    list<Noh> Aberto;
    list<Noh> Fechado;
    list<Noh>::iterator oldA;
    list<Noh>::iterator oldF;
    //list<Noh>::iterator old;  // cria um iterador para lista de Noh

    Coord prox;
    Coord dir;

    Noh suc;
    Noh atual;

    atual.pos = orig;
    atual.ant = Coord();
    atual.g = 0.0;
    atual.h = Heuristica(orig,dest);

    Aberto.push_front(atual);
    double comprimento = 0.0;

    // percorre o conteiner
    do
    {
        // Atualiza atual com o Noh de
        // menor custo (o 1º) de Aberto
        atual = Aberto.front();
        // Remove o 1º Noh de Aberto
        Aberto.pop_front();
        // Insere o Noh em Fechado
        Fechado.push_front(atual);

        if(atual.pos != dest)
        {
            for (dir.lin = -1; dir.lin< 2; dir.lin++)
            {
                for(dir.col = -1; dir.col< 2; dir.col++)
                {
                    if(dir != Coord(0,0) )
                    {
                        prox = atual.pos + dir;

                        if(movimentoValido(atual.pos, prox))
                        {
                            suc.pos = prox;
                            suc.ant = atual.pos;
                            suc.g = atual.g + norm(dir);
                            suc.h = Heuristica(suc.pos,dest);

                            oldF = find(Fechado.begin(),Fechado.end(),suc.pos);

                            if(oldF != Fechado.end())
                            {
                                if(suc.getCost() < oldF->getCost())
                                {
                                    Fechado.erase(oldF);
                                    oldF = Fechado.end();
                                }
                            }
                            if(oldF == Fechado.end())
                            {
                                oldA = find(Aberto.begin(),Aberto.end(),suc.pos);

                                if(oldA != Aberto.end())
                                {

                                    if(suc.getCost() < oldA->getCost())
                                    {
                                        Aberto.erase(oldA);
                                        oldA = Aberto.end();
                                    }

                                }
                            }
                            if(oldF == Fechado.end() && oldA == Aberto.end())
                            {
                                maior_que mqn(suc);
                                oldA = find_if(Aberto.begin(),Aberto.end(),mqn);
                                Aberto.insert(oldA,suc);

                            }
                        }
                    }
                }
            }
        }
        //cout<<"debug"<<endl;
    }
    while((atual.pos != dest) && !Aberto.empty());

    if(atual.pos != dest)
    {
        NC = -1;
        NF = Fechado.size();
        NA = Aberto.size();
        return -1;
    }
    else
    {
        comprimento = atual.g;
        NC = 1;
        while(atual.ant != orig)
        {
            set(atual.ant, EstadoCel::CAMINHO);
            oldF = find(Fechado.begin(),Fechado.end(),atual.ant);
            atual = *oldF;
            NC++;
        }
        NF = Fechado.size();
        NA = Aberto.size();
        return comprimento;

    }

}
