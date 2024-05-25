#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <algorithm>
#include <vector>

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
        curSymb->h_lim = curSymb->l_lim + curSymb->freq;
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

    int count = 0;
    std::map <char, int> freqAlph;
    std::map <char, int> ::iterator Iterator;
    std::list<SymbolInfo> freqList;

    while(!inp.eof())
    {
        char currentSymbol = inp.get();
        freqAlph[currentSymbol]++;
        count++;
    }

    make_freqList(freqAlph, freqList);

    freqList.begin()-> h_lim=freqList.begin()->freq;
    freqList.begin()->l_lim=0;

    std::list<SymbolInfo>::iterator currentSymbol = freqList.begin();
    std::list<SymbolInfo>::iterator previousSymbol = freqList.begin();

    make_bound(freqList, currentSymbol, previousSymbol);
    int bits=0;

    for (Iterator = freqAlph.begin(); Iterator != freqAlph.end(); Iterator++)
        if (Iterator->second != 0)
            bits += 40;

    out.write((char*)(&bits), sizeof(bits));

    for (int i=0; i<256; i++)
    {
        if (freqAlph[char(i)]>0)
        {
            char curSymbol = char(i);
            out.write((char*)(&curSymbol), sizeof(curSymbol));
            out.write((char*)(&freqAlph[char(i)]), sizeof(freqAlph[char(i)]));
        }
    }

    inp.clear();
    inp.seekg(0);

    int lowBound=0, upBound=65535, divisor =0, intervalDenominator=freqList.back().h_lim, firstQuarter=(upBound+1)/4, Half=firstQuarter*2, thirdQuarter=firstQuarter*3, bits_to_follow=0;
    char Byte=0;
    count=0;

    while(!inp.eof())
    {
        char curSymbol =inp.get(); divisor++;
        for(currentSymbol=freqList.begin(); currentSymbol!=freqList.end(); currentSymbol++)
        {
            if(curSymbol==currentSymbol->symbol) break;
        }

        if(curSymbol!=currentSymbol->symbol)
        {
            std::cout<<"Ошибка!"<<"\n";
            break;
        }

        int prevLowBound=lowBound;
        lowBound=lowBound+currentSymbol->l_lim*(upBound-lowBound+1)/intervalDenominator;
        upBound= prevLowBound+currentSymbol-> h_lim*(upBound- prevLowBound+1)/intervalDenominator-1;

        while(true)
        {
            if(upBound<Half)
            {
                count++;

                if(count==8)
                {
                    count = 0;
                    out << Byte;
                    Byte = 0;
                }

                for(; bits_to_follow>0; bits_to_follow--)
                {
                    Byte =Byte | (1<<(7-count));
                    count++;

                    if(count==8)
                    {
                        count = 0;
                        out << Byte;
                        Byte = 0;
                    }
                }
            }

            else if(lowBound>=Half)
            {
                Byte=Byte | (1<<(7-count));
                count++;

                if(count==8)
                {
                    count = 0;
                    out << Byte;
                    Byte = 0;
                }

                for(; bits_to_follow>0; bits_to_follow--)
                {
                    count++;
                    if(count==8)
                    {
                        count = 0;
                        out << Byte;
                        Byte = 0;
                    }
                }

                lowBound-=Half;
                upBound-=Half;
            }

            else if((lowBound>=firstQuarter) && (upBound<thirdQuarter))
            {
                bits_to_follow++;
                lowBound-=firstQuarter;
                upBound-=firstQuarter;
            }
            else break;

            lowBound+=lowBound;
            upBound+=upBound+1;
        }
    }
    out<<Byte;

    inp.clear();
    inp.seekg(0, std::ios::end);
    out.seekp(0, std::ios::end);

    double sizeInp = inp.tellg();
    double sizeOut = out.tellp();

    inp.close();
    out.close();
    return sizeOut / sizeInp;
}

bool decode(std::ifstream &inp, std::ofstream &out)
{
    if (!inp.is_open())
    {
        std::cout << "Ошибка открытия файла!" << "\n";
        return false;
    }
    int count = 0, res_bits, freq_bits;
    char sym;

    std::map <char, int> freqAlph;
    std::map <char, int> ::iterator Iterator;
    std::list<SymbolInfo> freqList;

    inp.read((char*)&res_bits, sizeof(res_bits));
    while (res_bits>0)
    {
        inp.read((char*)&sym, sizeof(sym));
        inp.read((char*)&freq_bits, sizeof(freq_bits));
        res_bits-=40;
        freqAlph[sym]=freq_bits;
    }

    make_freqList(freqAlph, freqList);

    freqList.begin()-> h_lim=freqList.begin()->freq;
    freqList.begin()->l_lim=0;

    std::list<SymbolInfo>::iterator currentSymbol = freqList.begin();
    std::list<SymbolInfo>::iterator previousSymbol = freqList.begin();

    make_bound(freqList, currentSymbol, previousSymbol);

    count=0;
    int low_lim=0, high_lim=65535, intervalDenominator=freqList.back().h_lim, firstQuarter=(high_lim+1)/4, Half=firstQuarter*2, thirdQuarter=firstQuarter*3, value=(inp.get()<<8) | inp.get();
    char symbl=inp.get();

    while(!inp.eof())
    {
        int calFreq = ((value - low_lim + 1) * intervalDenominator - 1) / (high_lim - low_lim + 1);
        for (currentSymbol = freqList.begin(); currentSymbol-> h_lim <= calFreq; currentSymbol++);
        int prev_low_lim = low_lim;
        low_lim = low_lim + (currentSymbol->l_lim) * (high_lim - low_lim + 1) / intervalDenominator;
        high_lim = prev_low_lim + (currentSymbol-> h_lim) * (high_lim - prev_low_lim + 1) / intervalDenominator - 1;

        while(true)
        {
            if (high_lim < Half);
            else if (low_lim >= Half)
            {
                low_lim -= Half;
                high_lim -= Half;
                value -= Half;
            }

            else if ((low_lim >= firstQuarter) && (high_lim < thirdQuarter))
            {
                low_lim -= firstQuarter;
                high_lim -= firstQuarter;
                value -= firstQuarter;
            }

            else break;

            low_lim += low_lim;
            high_lim += high_lim + 1;
            value += value + (((short) symbl >> (7 - count)) & 1);
            count++;

            if (count == 8)
            {
                symbl = inp.get();
                count = 0;
            }
        }

        out << currentSymbol->symbol;
    }

    inp.close();
    out.close();
    return true;
}

int main(){
    std::ifstream inp("file.txt", std::ios::out | std::ios::binary);
    std::ofstream out("out_file.txt", std::ios::out | std::ios::binary);
    
    double value = make_code(inp, out);
    std::cout << value << "\n";
    
    std::ifstream inp_coded("out_file.txt", std::ios::out | std::ios::binary);
    std::ofstream out_decoded("decoded_file.txt", std::ios::out | std::ios::binary);
    
    if (decode(inp_coded, out_decoded))
            std::cout << "Декодированно!" << "\n";
        else
            std::cout << "Не удалось декодировать!" << "\n";
    
    inp_coded.close();
    out_decoded.close();
    
    return 0;
}
