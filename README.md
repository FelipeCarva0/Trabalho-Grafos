# Segmentacao de Imagens Baseada em Grafos

Projeto em C++/OpenCV com tres abordagens de segmentacao inspiradas em algoritmos classicos de grafos.

## Visao Geral

Este projeto implementa:

- Solucao A: Efficient Graph-Based Image Segmentation (estilo Felzenszwalb-Huttenlocher).
- Solucao B: Hierarchical Segmentations with Quasi-Flat Zones (MST + hierarquia por limiar lambda).
- Solucao C: Image Foresting Transform (IFT) com sementes distribuidas na grade.

Cada solucao gera imagens segmentadas automaticamente em pastas separadas dentro de `assets/output`.

## Tecnologias

- C++
- OpenCV
- MSYS2 (UCRT64)
- g++
- Make

## Quick Start

### 1) Compilar (gera executavel da main)

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

- a solucao (`A`, `B`, `C` ou `T` para todas),
- a imagem em `assets/images`,
- e o parametro da tecnica.

## Uso

Ao executar, informe:

- Solucao A: valor `k` (recomendado: `8000`)
- Solucao B: valor `lambda` (recomendado: `40`)
- Solucao C: numero de sementes `n` (recomendado: `300`)

Imagens de entrada disponiveis em `assets/images`:

- `baseball.jpg`
- `building.jpg`
- `cat.png`
- `horse.jpg`
- `lioness.jpg`

## Saidas Geradas

As segmentacoes sao salvas automaticamente em:

- `assets/output/A`
	- versoes em `rgb`, `gray` e `mean`
- `assets/output/B/<nome-da-imagem>`
	- hierarquia para multiplos valores de `lambda` em `rgb`, `gray` e `mean`
	- mapa de saliencia: `assets/output/B/<nome-da-imagem>_saliency.<ext>`
- `assets/output/C`
	- versoes em `rgb`, `grayscale` e `mean`

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

## Comandos Uteis

Recompilar o executavel da main:

```bash
g++ -Wall -Wextra -g3 src/*.cpp -IC:/msys64/ucrt64/include -IC:/msys64/ucrt64/include/opencv4 -LC:/msys64/ucrt64/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -o output/main.exe
```

Executar:

```bash
.\output\main.exe
```