#include <iostream>
#include <fstream>
#include <map>
#include <list>

struct SymbolInfo{
    char symbol;
    int freq, prev, next;
};

struct freqComparison{
    bool operator() (SymbolInfo l, SymbolInfo r){
        return l.freq > r.freq;
    }
};

void make_freqList(const std::map<char, int> &freqAlph, std::list<SymbolInfo> &freqList){
    for (std::map<char, int>::const_iterator Iter = freqAlph.begin(); Iter != freqAlph.end(); Iter++) {
        SymbolInfo s_inf;
        s_inf.symbol = Iter->first;
        s_inf.freq = Iter->second;
        freqList.push_back(s_inf);
    }
    
    freqList.sort(freqComparison());
     
}

void make_bound(std::list<SymbolInfo> &freqList, std::list<SymbolInfo>::iterator &curSymb, std::list<SymbolInfo>::iterator &prevSymb){
    prevSymb = curSymb;
    curSymb++;
    
    for(; curSymb != freqList.end(); curSymb++){
        curSymb->prev = prevSymb->next;
        curSymb->next = prevSymb->prev + curSymb->freq;
        prevSymb++;
    }
}

double make_code(std::ifstream &inp, std::ofstream &out) {
    if (!inp.is_open()) {
        std::cout << "Ошибка открытия файла!" << "\n";
        return 0.0;
    }
    
    int cnt = 0;
    std::map <char, int> freqAlph;
    std::list<SymbolInfo> freqList;
    std::map<char, int>::iterator Iterator;
    
    while(!inp.eof()){
        char symbol = inp.get();
        freqAlph[symbol]++;
        cnt++;
    }
    /*
    std::cout << "freqAlph:" << std::endl;
    for (const auto& pair : freqAlph) {
        std::cout << "Symbol: " << pair.first << ", Frequency: " << pair.second << std::endl;
    }
    */
    make_freqList(freqAlph, freqList);
    
    freqList.begin()-> next = freqList.begin()->freq;
    freqList.begin()-> prev = 0;
    
    std::list<SymbolInfo>::iterator curSymb = freqList.begin();
    std::list<SymbolInfo>::iterator prevSymb = freqList.begin();
    
    make_bound(freqList, curSymb, prevSymb);
    
    /*
    std::cout << "freqList:" << std::endl;
    for (const auto& info : freqList) {
        std::cout << "Symbol: " << info.symbol << ", Frequency: " << info.freq << ", Prev: " << info.prev << ", Next: " << info.next << std::endl;
    }
    */
    
    int bits = 0;
    
    for (Iterator = freqAlph.begin(); Iterator != freqAlph.end(); Iterator++){
        if (Iterator->second != 0) bits += 40;
        
        out.write((char*)(&bits), sizeof(bits));
        
        for (int i = 0; i < 256; i++){
            if (freqAlph[char(i)] > 0){
                char currentSymbol = char(i);
                out.write((char*)(&currentSymbol), sizeof(currentSymbol));
                out.write((char*)(&freqAlph[char(i)]), sizeof(freqAlph[char(i)]));
            }
        }
        inp.clear();
        inp.seekg(0);
        
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
