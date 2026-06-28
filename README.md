# Segmentação de Imagens Baseada em Grafos

Projeto em C++/OpenCV com três abordagens de segmentação inspiradas em algoritmos clássicos de grafos.

## Visão Geral

Este projeto implementa:

- Solução A: Efficient Graph-Based Image Segmentation (estilo Felzenszwalb-Huttenlocher).
- Solução B: Hierarchical Segmentations with Quasi-Flat Zones (MST + hierarquia por limiar lambda).
- Solução C: Image Foresting Transform (IFT) com sementes distribuídas na grade.

Cada solução gera imagens segmentadas automaticamente em pastas separadas dentro de `assets/output`.

## Tecnologias

- C++
- OpenCV
- MSYS2 (UCRT64)
- g++
- Make

## Quick Start

### 1) Compilar (gera executável da main)

⚠️ Se estiver usando MSYS2 (UCRT64), utilize este comando:

```bash
g++ -Wall -Wextra -g3 src/*.cpp -IC:/msys64/ucrt64/include -IC:/msys64/ucrt64/include/opencv4 -LC:/msys64/ucrt64/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -o output/main.exe
```

⚠️ Caso esteja usando outro ambiente (MinGW, vcpkg, etc.), o comando muda conforme sua instalação do OpenCV:

```bash
g++ -Wall -Wextra -g3 src/*.cpp -I<PATH_OPEN_CV_INCLUDE> -L<PATH_OPEN_CV_LIB> -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -o output/main.exe
```

### 2) Executar a main

```bash
.\output\main.exe
```

O fluxo interativo da `main.cpp` pede:

- a solução (`A`, `B`, `C` ou `T` para todas),
- a imagem em `assets/images`,
- e o parâmetro da técnica.

## Uso

Ao executar, informe:

- Solução A: valor `k` (recomendado: `8000`)
- Solução B: nenhum valor precisa ser informado
- Solução C: número de sementes `n` (recomendado: `300`)

Imagens de entrada disponíveis em `assets/images`:

- `baseball.jpg`
- `building.jpg`
- `cat.png`
- `horse.jpg`
- `lioness.jpg`

## Saídas Geradas

As segmentações são salvas automaticamente em:

- `assets/output/A`
    - versões em `rgb`, `gray` e `mean`
- `assets/output/B/<nome-da-imagem>`
    - hierarquia para múltiplos valores de `lambda` em `rgb`, `gray` e `mean`
    - mapa de saliência: `assets/output/B/<nome-da-imagem>_saliency.<ext>`
- `assets/output/C`
    - versões em `rgb`, `grayscale` e `mean`

## Estrutura do Projeto

```text
Trabalho-Grafos/
|-- Makefile
|-- README.md
|-- assets/
|   |-- images/
|   `-- output/
|       |-- A/
|       |-- B/
|       `-- C/
|-- output/
`-- src/
    |-- main.cpp
    |-- solucaoA.cpp
    |-- solucaoB.cpp
    |-- solucaoC.cpp
    |-- utils.cpp
    `-- include/
        |-- ift.hpp
        |-- mst.hpp
        |-- mstB.hpp
        `-- utils.hpp
```

## Comandos Úteis

Recompilar o executável da main:

```bash
g++ -Wall -Wextra -g3 src/*.cpp -IC:/msys64/ucrt64/include -IC:/msys64/ucrt64/include/opencv4 -LC:/msys64/ucrt64/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -o output/main.exe
```

Executar:

```bash
.\output\main.exe
```