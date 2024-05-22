#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <algorithm>

struct SymbolInfo{
    char symbol;
    int freq, l_lim, h_lim;
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

void make_bound(std::list<SymbolInfo> &freqList, std::list<SymbolInfo>::iterator &curSymb, std::list<SymbolInfo>::iterator &prevSymb)
{
    prevSymb = curSymb;
    curSymb++;
    
    for(; curSymb != freqList.end(); curSymb++){
        curSymb->l_lim = prevSymb->h_lim;
        curSymb->h_lim = prevSymb->l_lim + curSymb->freq;
        prevSymb++;
    }
}

double make_code(std::ifstream &inp, std::ofstream &out)
{
    if (!inp.is_open())
    {
        std::cout << "Ошибка открытия файла!" << "\n";
        return 0.0;
    }
    
    int cnt = 0;
    std::map <char, int> freqAlph;
    std::list<SymbolInfo> freqList;
    std::map<char, int>::iterator Iterator;
    
    while(!inp.eof())
    {
        char symbol = inp.get();
        freqAlph[symbol]++;
        cnt++;
    }
    /*
     std::cout << "freqAlph:" << std::endl;
     for (const auto& pair : freqAlph){
     std::cout << "Symbol: " << pair.first << ", Frequency: " << pair.second << std::endl;
     }
     */
    make_freqList(freqAlph, freqList);
    
    freqList.begin()-> h_lim = freqList.begin()->freq;
    freqList.begin()-> l_lim = 0;
    
    std::list<SymbolInfo>::iterator curSymb = freqList.begin();
    std::list<SymbolInfo>::iterator prevSymb = freqList.begin();
    
    make_bound(freqList, curSymb, prevSymb);
    
    /*
     std::cout << "freqList:" << std::endl;
     for (const auto& info : freqList) {
     std::cout << "Symbol: " << info.symbol << ", Frequency: " << info.freq << ", l_lim: " << info.l_lim << ", h_lim: " << info.h_lim << std::endl;
     }
     */
    
    int bits = 0;
    
    for (Iterator = freqAlph.begin(); Iterator != freqAlph.end(); Iterator++){
        if (Iterator->second != 0){
            bits += 40;
        }
    }
    
    out.write((char*)(&bits), sizeof(bits));
    
    for (int i = 0; i < 256; i++)
    {
        if (freqAlph[char(i)] > 0)
        {
            char currentSymbol = char(i);
            out.write((char*)(&currentSymbol), sizeof(currentSymbol));
            out.write((char*)(&freqAlph[char(i)]), sizeof(freqAlph[char(i)]));
        }
    }
    
    inp.clear();
    inp.seekg(0);
    
    int low_lim=0, high_lim=65535, div=0, intDenomenator=freqList.back().l_lim, firstQuarter=(high_lim+1)/4, half=firstQuarter*2, thirdQuarter=firstQuarter*3, bits_to_follow = 0;
    char Byte = 0;
    cnt = 0;
    
    while (!inp.eof())
    {
        char currentSymbol = inp.get(); div++;
        for (curSymb = freqList.begin(); curSymb != freqList.end(); curSymb++)
        {
            if (currentSymbol == curSymb -> symbol) break;
        }
        
        if (currentSymbol != curSymb -> symbol)
        {
            std::cout<< "Ошибка !" <<"\n";
            break;
        }
        
        int prev_low_lim = low_lim;
        low_lim = low_lim + curSymb -> l_lim * (high_lim - low_lim + 1) / intDenomenator;
        high_lim = prev_low_lim + curSymb -> h_lim * (high_lim - prev_low_lim + 1) / intDenomenator - 1;
        
        while (true)
        {
            if (high_lim < half)
            {
                cnt++;
                
                if (cnt == 8)
                {
                    cnt = 0;
                    out << Byte;
                    Byte = 0;
                }
                for (; bits_to_follow>0; bits_to_follow--)
                {
                    Byte = Byte | (1<<(7-cnt));
                    cnt++;
                    
                    if (cnt == 8)
                    {
                        cnt = 0;
                        out << Byte;
                        Byte = 0;
                    }
                }
            }
            
            else if (low_lim >= half)
            {
                Byte = Byte | (1<<(7 - cnt));
                cnt++;
                
                if (cnt == 8)
                {
                    cnt = 0;
                    out << Byte;
                    Byte = 0;
                }
                for (; bits_to_follow>0; bits_to_follow--)
                {
                    Byte = Byte | (1<<(7-cnt));
                    cnt++;
                    
                    if (cnt == 8)
                    {
                        cnt = 0;
                        out << Byte;
                        Byte = 0;
                    }
                }
                
                low_lim -= half;
                high_lim -= half;
            }
            
            else if ((low_lim >= firstQuarter) && (high_lim < thirdQuarter))
            {
                bits_to_follow++;
                low_lim -= firstQuarter;
                high_lim -= firstQuarter;
            }
            else break;
            
            low_lim += low_lim;
            high_lim += high_lim + 1;
        }
    }


    
    out << Byte;
    
    inp.clear();
    inp.seekg(0, std::ios::end);
    out.seekp(0, std::ios::end);
    
    double sizeInp = inp.tellg();
    double sizeOut = out.tellp();
    
    inp.close();
    out.close();
    return sizeOut / sizeInp;
}

int main(){
    std::ifstream inp("file.txt", std::ios::out | std::ios::binary);
    std::ofstream out("out_file.txt", std::ios::out | std::ios::binary);
    
    double value = make_code(inp, out);
    
    std::cout << value << "\n";
    
    return 0;
}
