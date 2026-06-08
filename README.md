# Segmentação de Imagens 

## 📌 Tecnologias utilizadas

* C++
* OpenCV
* MSYS2 (UCRT64)
* g++

---

## ⚙️ Como compilar

No terminal (PowerShell ou MSYS2 UCRT64):

```bash
g++ -Wall -Wextra -g3 src/solucaoA.cpp -IC:/msys64/ucrt64/include -IC:/msys64/ucrt64/include/opencv4 -LC:/msys64/ucrt64/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -o output/solucaoA.exe
```

---

## ▶️ Como executar

Após compilar:

```bash
.\output\solucaoA.exe
```
