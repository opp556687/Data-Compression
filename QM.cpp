#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

// initialize
unsigned int QcTable[] = {0x59eb, 0x5522, 0x504f, 0x4b35, 0x4639, 0x415e, 0x3c3d, 0x385e, 0x32b4, 0x2e17, 0x299a, 0x2516, 0x1edf, 0x1aa9, 0x174e, 0x1424, 0x119c, 0x0f6b, 0x0d51, 0x0bb6, 0x0a40, 0x0861, 0x0706, 0x05cd, 0x04de, 0x040f, 0x0363, 0x02d4, 0x025c, 0x01f8, 0x01a4, 0x0160, 0x0125, 0x00f6, 0x00cb, 0x00ab, 0x008f, 0x0068, 0x004e, 0x003b, 0x002c, 0x001a, 0x000d, 0x0006, 0x0003, 0x0001};

int QcInc[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};

int QcDec[] = {-1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 2, 1, 2, 1, 2, 2, 1, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 3, 2, 2, 2, 1};

char LPS = '1';
char MPS = '0';
unsigned int Qc = 0x59EB;
int state = 0;
unsigned int A = 0x10000;
unsigned short C = 0x0000;
int buffer = 0;
int counter = 0;
int bitLength = 0;

void encode(ofstream &outfile, unsigned char data)
{
    if (((int)data & 0x80) == ((int)MPS - 48)) // if recevie MPS
    {
        A = A - Qc;
        if (A < 0x8000)
        {
            if (A < Qc)
            {
                C += A;
                A = Qc;
            }
            A <<= 1;
            state += QcInc[state];
            Qc = QcTable[state];
            buffer += ((C & 0x8000) != 0);
            counter++;
            C <<= 1;
            bitLength++;
        }
    }
    else // if receive LPS
    {
        A = A - Qc;
        if (A >= Qc)
        {
            C += A;
            A = Qc;
        }
        A <<= 1;
        if (QcDec[state] >= 0)
        {
            state -= QcDec[state];
            Qc = QcTable[state];
        }
        else
        {
            swap(MPS, LPS);
        }
        buffer += ((C & 0x8000) != 0);
        counter++;
        C <<= 1;
        bitLength++;
    }

    if (counter == 8) // buffer is full write to compressed file
    {
        outfile << ((unsigned char)buffer);
        buffer = 0;
        counter = 0;
    }
    else
    {
        buffer <<= 1;
    }
}

void grayLevel(ifstream &infile, ofstream &outfile)
{
    cout << "[+] Start encoding gray level image..." << endl;
    vector<unsigned char> data;
    while (1)
    {
        unsigned char temp = infile.get();
        if (infile.eof())
        {
            break;
        }
        data.push_back(temp);
    }

    for (int i = 0; i < 8; i++) // encode from MSB to LSB
    {
        bitLength = 0;
        for (int j = 0; j < data.size(); j++)
        {
            encode(outfile, data[j]);
            data[j] <<= 1;
        }
        cout << (7 - i) << " bit-plane's bit-stream length = " << bitLength << endl;
    }
    if (counter != 0) // still some data in buffer
    {
        buffer <<= (8 - counter);
        outfile << ((unsigned char)buffer);
    }
    cout << "[+] Encode complete!!" << endl;
}

void grayLevelGrayCode(ifstream &infile, ofstream &outfile)
{
    cout << "[+] Start encoding gray level image with gray code..." << endl;
    vector<unsigned char> data;
    while (1)
    {
        unsigned char temp = infile.get();
        if (infile.eof())
        {
            break;
        }
        temp = temp ^ (temp / 2);
        data.push_back(temp);
    }

    for (int i = 0; i < 8; i++)
    {
        bitLength = 0;
        for (int j = 0; j < data.size(); j++)
        {
            encode(outfile, data[j]);
            data[j] <<= 1;
        }
        cout << (7 - i) << " bit-plane's bit-stream length = " << bitLength << endl;
    }
    if (counter != 0) // still some data in buffer
    {
        buffer <<= (8 - counter);
        outfile << ((unsigned char)buffer);
    }
    cout << "[+] Encode complete!!" << endl;
}

void binaryImage(ifstream &infile, ofstream &outfile)
{
    cout << "[+] Start encoding binary image..." << endl;
    while (1)
    {
        unsigned char temp = infile.get();
        if (infile.eof())
        {
            break;
        }
        encode(outfile, ((temp - 48) << 7));
    }
    if (counter != 0)
    {
        buffer <<= (8 - counter);
        outfile << ((unsigned char)buffer);
    }
    cout << "[+] Encode complete!!" << endl;
}

int menu()
{
    int choice;
    cout << "Which function to use?" << endl;
    cout << "1) encode gray level images" << endl;
    cout << "2) encode gray level image with gray code" << endl;
    cout << "3) encode binary image" << endl;
    cout << "> ";
    cin >> choice;
    return choice;
}

int main()
{
    int choice = menu();
    string inFilename, outFilename = "compressed";
    ifstream infile;
    ofstream outfile;
    cout << "Enter the file to compress: ";
    cin >> inFilename;
    infile.open(inFilename.c_str(), ios::in | ios::binary);
    if (!infile)
    {
        cout << "Can't open " << inFilename << endl;
        exit(0);
    }
    outfile.open(outFilename.c_str(), ios::out | ios::binary);
    if (!outfile)
    {
        cout << "Can't open " << outFilename << endl;
        exit(0);
    }
    switch (choice)
    {
    case 1:
        grayLevel(infile, outfile);
        break;
    case 2:
        grayLevelGrayCode(infile, outfile);
        break;
    case 3:
        binaryImage(infile, outfile);
        break;
    default:
        cout << "Invalid choice!!" << endl;
    }
    return 0;
}