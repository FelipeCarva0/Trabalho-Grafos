#ifndef __MST_HPP__
#define __MST_HPP__

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

using namespace std;
using namespace cv;

class MST {
    private: 
        struct Vertice {
            int lin, col;
            Vec3b cor; //RGB
        };

        struct Aresta {
            int v1, v2;
            int peso; //w

            Aresta(int v1, int v2, int peso)
            {
                this->v1 = v1;
                this->v2 = v2;
                this->peso = peso;
            }
        };

        float k;

        Mat imagem;

        vector<Vertice> vertices;
        vector<Aresta> arestas;

    public:
        MST(string &imagemPath, float k);
        void construirGrafo();
        int calcularPeso(Vertice v1, Vertice v2);
        Mat segmentacao();
};

#endif // __MST_HPP__