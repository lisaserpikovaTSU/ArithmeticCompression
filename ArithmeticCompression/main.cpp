#include <iostream>
#include <fstream>

double make_code(std::ifstream &inp, std::ofstream &out) {
    if (!inp.is_open()) {
        std::cout << "Ошибка открытия файла!" << "\n";
        return 0.0;
    }
    
    inp.close();
    out.close();
    
    return 1.0;
}

int main() {
    std::ifstream inp("file.txt", std::ios::out | std::ios::binary);
    std::ofstream out("out_file.txt", std::ios::out | std::ios::binary);
    
    double value = make_code(inp, out);
    
    std::cout << value << "\n";
    
    return 0;
}
