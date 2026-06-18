#include "include/mst.hpp"
#include <iostream>
#include <filesystem>
#include <random>
#include <numeric>
#include <algorithm>
#include <unordered_map>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;


static string imagePath = "assets/images/baseball.jpg";

// ------------------------------------------------------------
// Disjoint-set com rank (union by rank + path compression)
// Guarda também o índice do nó BPTAO criado na última união.
// ------------------------------------------------------------
struct DSU {
    vector<int> parent, rank_;
    vector<int> bptao_node; // qual nó da árvore representa este componente

    DSU(int n) : parent(n), rank_(n, 0), bptao_node(n) {
        iota(parent.begin(), parent.end(), 0);
        iota(bptao_node.begin(), bptao_node.end(), 0);
    }

    int find(int u) {
        if (parent[u] != u) parent[u] = find(parent[u]);
        return parent[u];
    }

    // retorna a raiz do conjunto resultante
    int unite(int u, int v) {
        u = find(u); v = find(v);
        if (u == v) return u;
        if (rank_[u] < rank_[v]) swap(u, v);
        parent[v] = u;
        if (rank_[u] == rank_[v]) rank_[u]++;
        return u;
    }
};

// ------------------------------------------------------------
// Nó da BPTAO (Binary Partition Tree by Altitude Ordering)
// Cada nó folha corresponde a um pixel; os nós internos são
// criados conforme o algoritmo de Kruskal processa as arestas.
// ------------------------------------------------------------
struct BPTNode {
    int  left  = -1;   // filho esquerdo (-1 → folha)
    int  right = -1;   // filho direito  (-1 → folha)
    int  level = 0;    // peso da aresta que criou este nó
    int  size  = 1;    // número de pixels no sub-componente
};

// ------------------------------------------------------------
// Estrutura principal: grafo + BPTAO
// ------------------------------------------------------------
struct SolucaoB {
    // ---- grafo ----
    int rows, cols, N;  // dimensões e nº de pixels
    Mat image;

    struct Edge { int u, v, w; };
    vector<Edge> edges;

    // ---- árvore hierárquica ----
    vector<BPTNode> tree; // índices 0..N-1: folhas; N.. : internos

    // ---- mapeamento pixel→raiz da árvore ----
    // (atualizado pelo DSU durante Kruskal)
    DSU dsu;

    SolucaoB(const string& path) : dsu(0) {
        image = imread(path, IMREAD_COLOR);
        if (image.empty()) {
            cerr << "Erro ao carregar: " << path << endl;
            return;
        }
        rows = image.rows;
        cols = image.cols;
        N    = rows * cols;
        dsu  = DSU(N);

        // Inicializa folhas da árvore (uma por pixel)
        tree.resize(N);
        for (int i = 0; i < N; i++) {
            tree[i].left  = -1;
            tree[i].right = -1;
            tree[i].level =  0;
            tree[i].size  =  1;
        }
    }

    // Peso L1 entre dois pixels vizinhos (mesmo do Solução A)
    int weight(int r1, int c1, int r2, int c2) {
        Vec3b a = image.at<Vec3b>(r1, c1);
        Vec3b b = image.at<Vec3b>(r2, c2);
        return abs((int)a[0]-(int)b[0])
             + abs((int)a[1]-(int)b[1])
             + abs((int)a[2]-(int)b[2]);
    }

    // Seção 5 do artigo: Grid Graph (4-adjacência horizontal+vertical)
    void buildGraph() {
        edges.clear();
        edges.reserve(2 * N);

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                int id = r * cols + c;
                if (c + 1 < cols)
                    edges.push_back({id, id + 1, weight(r,c,r,c+1)});
                if (r + 1 < rows)
                    edges.push_back({id, id + cols, weight(r,c,r+1,c)});
            }
        }

        // Ordena por peso — passo fundamental para Kruskal / BPTAO
        sort(edges.begin(), edges.end(),
             [](const Edge& a, const Edge& b){ return a.w < b.w; });
    }

    // --------------------------------------------------------
    // Algoritmo central (Seção 8.1 do artigo):
    // Constrói a BPTAO durante o Kruskal.
    //
    // Para cada aresta {u,v} (em ordem crescente de peso):
    //   se u e v pertencem a componentes distintos:
    //     1. cria nó interno na árvore com filhos = raízes dos dois
    //        componentes na BPTAO e nível = peso da aresta
    //     2. une os dois componentes no DSU
    //     3. aponta o novo nó BPTAO como representante do componente
    //        resultante
    // --------------------------------------------------------
    void buildBPTAO() {
        // Os índices 0..N-1 já estão reservados para folhas.
        // Nós internos serão adicionados a partir do índice N.

        for (const Edge& e : edges) {
            int ru = dsu.find(e.u);
            int rv = dsu.find(e.v);
            if (ru == rv) continue;

            // Índices dos nós BPTAO que representam os dois componentes
            int bu = dsu.bptao_node[ru];
            int bv = dsu.bptao_node[rv];

            // Cria nó interno
            BPTNode node;
            node.left  = bu;
            node.right = bv;
            node.level = e.w;
            node.size  = tree[bu].size + tree[bv].size;

            int newIdx = (int)tree.size();
            tree.push_back(node);

            // Une no DSU e registra o novo nó BPTAO
            int newRoot = dsu.unite(ru, rv);
            dsu.bptao_node[newRoot] = newIdx;
        }
    }

    // --------------------------------------------------------
    // Extração de segmentação: corte na BPTAO por nível máximo
    //
    // Percorre a árvore de cima para baixo.  Quando um nó
    // interno tem nível > cutLevel (peso da aresta), seus
    // filhos tornam-se raízes de regiões distintas.
    //
    // cutLevel = 0 → máxima fragmentação (cada pixel separado)
    // cutLevel = MAX_INT → segmento único
    //
    // O parâmetro targetRegions permite cortar por número de
    // regiões: busca binária no espaço de cutLevel.
    // --------------------------------------------------------
    unordered_map<int,int> extractSegmentation(int cutLevel) {
        // Mapeia nó BPTAO → id de região (pixel-leaf → region)
        // Percurso DFS iterativo
        unordered_map<int,int> leafToRegion;
        int regionCounter = 0;

        struct Frame { int nodeIdx; int regionId; };
        vector<Frame> stack;

        int root = (int)tree.size() - 1; // último nó inserido = raiz
        stack.push_back({root, -1});

        while (!stack.empty()) {
            auto [idx, parentRegion] = stack.back();
            stack.pop_back();

            bool isLeaf = (tree[idx].left == -1);

            if (isLeaf) {
                // Pixel: atribui região do pai
                leafToRegion[idx] = (parentRegion >= 0) ? parentRegion
                                                         : regionCounter++;
            } else {
                // Nó interno
                bool corta = (tree[idx].level > cutLevel);

                if (corta) {
                    // Cada filho inicia uma região nova
                    stack.push_back({tree[idx].right, regionCounter++});
                    stack.push_back({tree[idx].left,  regionCounter++});
                } else {
                    // Propaga a região do pai para os filhos
                    int rid = (parentRegion >= 0) ? parentRegion
                                                  : regionCounter++;
                    stack.push_back({tree[idx].right, rid});
                    stack.push_back({tree[idx].left,  rid});
                }
            }
        }

        return leafToRegion;
    }

    // Busca o menor cutLevel que produz ≤ targetRegions regiões
    int findCutLevel(int targetRegions) {
        // Coleta todos os níveis distintos da árvore
        vector<int> levels;
        levels.reserve(tree.size());
        for (int i = N; i < (int)tree.size(); i++)
            levels.push_back(tree[i].level);
        sort(levels.begin(), levels.end());
        levels.erase(unique(levels.begin(), levels.end()), levels.end());

        // Busca binária: queremos o menor nível tal que
        // countRegions(level) <= targetRegions
        int lo = 0, hi = (int)levels.size() - 1, best = levels.back();
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            auto seg = extractSegmentation(levels[mid]);
            int nReg = 0;
            for (auto& [k,v] : seg) nReg = max(nReg, v+1);

            if (nReg <= targetRegions) {
                best = levels[mid];
                hi   = mid - 1;
            } else {
                lo = mid + 1;
            }
        }
        return best;
    }

    // --------------------------------------------------------
    // Renderiza a segmentação com cores aleatórias (RGB) ou
    // escala de cinza determinística (igual à Solução A)
    // --------------------------------------------------------
    Mat render(const unordered_map<int,int>& leafToRegion, bool rgb) {
        Mat result(rows, cols, CV_8UC3);

        mt19937 rng(42);
        uniform_int_distribution<int> dist(0, 255);

        // Paleta de cores por região
        unordered_map<int,Vec3b> palette;

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                int pixelId = r * cols + c;
                int regId   = leafToRegion.at(pixelId);

                Vec3b color;
                if (rgb) {
                    if (palette.find(regId) == palette.end())
                        palette[regId] = Vec3b(dist(rng), dist(rng), dist(rng));
                    color = palette[regId];
                } else {
                    int gray = (regId * 2654435761u) % 256;
                    color    = Vec3b(gray, gray, gray);
                }
                result.at<Vec3b>(r, c) = color;
            }
        }
        return result;
    }
};

// ------------------------------------------------------------
// Utilitário: salva imagem em assets/output/
// ------------------------------------------------------------
static void saveResult(const Mat& img, const string& srcPath,
                       const string& suffix,
                       const string& outDir = "assets/output") {
    fs::path p(srcPath);
    string base = p.stem().string();
    string ext  = p.extension().string();
    if (ext.empty()) ext = ".png";

    fs::create_directories(outDir);
    string out = outDir + "/" + base + "_" + suffix + ext;
    imwrite(out, img);
    cout << "Salvo: " << out << endl;
}

// ------------------------------------------------------------
// main
// ------------------------------------------------------------
int main() {
    cout << "Solucao B — Hierarchical MST (BPTAO, Cousty et al.)" << endl;

    SolucaoB sol(imagePath);
    if (sol.image.empty()) return 1;

    // 1) Constrói o grafo grid (Seção 5 do artigo)
    sol.buildGraph();

    // 2) Constrói a BPTAO via Kruskal (Seção 8.1)
    sol.buildBPTAO();

    cout << "BPTAO construída. Nós totais: " << sol.tree.size()
         << " (folhas: " << sol.N << ", internos: "
         << sol.tree.size() - sol.N << ")" << endl;

    // 3a) Segmentação por cutLevel fixo (ex.: peso 15)
    //     Equivale a extrair as quasi-flat zones com λ = cutLevel+1
    {
        int cutLevel = 50;
        auto seg = sol.extractSegmentation(cutLevel);
        int nReg = 0;
        for (auto& [k,v] : seg) nReg = max(nReg, v+1);
        cout << "cutLevel=" << cutLevel << " → " << nReg << " regioes" << endl;

        Mat rgb  = sol.render(seg, true);
        Mat gray = sol.render(seg, false);
        saveResult(rgb,  imagePath, "B_rgb");
        saveResult(gray, imagePath, "B_gray");
    }

    // 3b) Segmentação por número-alvo de regiões (ex.: ~50)
    {
        int target = 50;
        int cutLevel = sol.findCutLevel(target);
        auto seg = sol.extractSegmentation(cutLevel);
        int nReg = 0;
        for (auto& [k,v] : seg) nReg = max(nReg, v+1);
        cout << "target=" << target << " regioes → cutLevel=" << cutLevel
             << " → " << nReg << " regioes" << endl;

        Mat rgb  = sol.render(seg, true);
        Mat gray = sol.render(seg, false);
        saveResult(rgb,  imagePath, "B_target_rgb");
        saveResult(gray, imagePath, "B_target_gray");
    }

    return 0;
}