#include <iostream>
#include "include/mst.hpp"
#include "include/mstB.hpp"
#include "include/ift.hpp"
#include "include/utils.hpp"

using namespace std;
string imagePath = "assets/images/building.jpg";
string imageName;

int main() {
    cout << "Escolha a solucao (A, B, C ou Todos): \n" << endl;

    cout << "A: Solucao A (Efficient Graph-Based Image Segmentation)" << endl;
    cout << "B: Solucao B (Hierarchical Segmentations with Graphs Quasi-flat)" << endl;
    cout << "C: Solucao C (The Image Foresting Transform Theory and Algorithms)" << endl;
    cout << "T: Executa todas as solucoes (A, B e C) com os valores recomendados\n" << endl;

    cout << "Digite a letra correspondente a solucao desejada: ";

    char choice;
    cin >> choice;

    cout << "Digite o nome da imagem com extensao (ex: cat.png ou building.jpg): ";
    cin >> imageName;

    imagePath = "assets/images/" + imageName;

    switch (choice) {
        case 'A':
        case 'a': {
            cout << "Escolha valor K (recomendado: 8000): " << endl;
            float k;
            cin >> k;
            MST mst(imagePath, k);
            mst.segment();
            break;
        }

        case 'B':
        case 'b': {
            cout << "Escolha valor Lambda (recomendado: 40): " << endl;
            int lambda;
            cin >> lambda;
            MSTB mstB(imagePath, lambda);
            mstB.segment();
            break;
        }

        case 'C':
        case 'c': {
            cout << "Escolha numero de sementes (recomendado: 300): " << endl;
            int n;
            cin >> n;
            IFT ift(imagePath, n);
            ift.segment();
            break;
        }

        case 'T':
        case 't': {
            cout << "Executando todas as solucoes com os valores recomendados..." << endl;
            
            MST mst(imagePath, 8000);
            mst.segment();

            MSTB mstB(imagePath, 40);
            mstB.segment();

            IFT ift(imagePath, 300);
            ift.segment();

            break;
        }

        default: {
            cout << "Opcao invalida!" << endl;
            break;
        }
    }

    return 0;
}
