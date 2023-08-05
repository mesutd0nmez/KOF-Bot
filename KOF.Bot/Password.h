#include <iostream>
#include <vector>
#include <string>
#include <cstring>

class Password {
private:
    static const std::vector<unsigned char> _encodingArray;
    static const std::vector<unsigned char> _alphabetArray;

    static unsigned int StepA(unsigned int ins) {
        unsigned int res = 0;
        unsigned int dat;
        for (int i = 0; i < 64; i++) {
            dat = ins - (ins & 0xFFFFFFFE);
            ins >>= 1;

            if (dat == 1)
                res += dat << _encodingArray[i];

            if (ins == 0)
                return res;
        }

        return res;
    }

    static void StepB(unsigned int ins, std::string& encoded) {
        std::vector<unsigned char> outChar;

        for (int i = 0; i < 7; i++) {
            unsigned int dat = (unsigned int)(((unsigned long long)ins * 0x38e38e39) >> 35);
            ins -= dat * 9 << 2;

            if (ins < 36)
                outChar.push_back(_alphabetArray[ins]);

            ins = dat;
        }

        encoded = std::string(outChar.begin(), outChar.end());
    }

    static std::vector<unsigned int> ByteArrayToUintArray(const std::vector<unsigned char>& data) {
        int length = (data.size() + 3) / 4;
        std::vector<unsigned int> data2(length);
        memcpy(data2.data(), data.data(), data.size());
        return data2;
    }

public:
    static std::string PasswordHash(const std::string& inputPassword) {
        std::vector<unsigned char> inputData(inputPassword.begin(), inputPassword.end());
        std::vector<unsigned int> dat = ByteArrayToUintArray(inputData);

        std::string tmpEncodedString;

        for (size_t i = 0; i < dat.size(); i++) {
            std::string tmpValue;
            StepB(StepA(dat[i] + 0x3E8), tmpValue);

            if (dat.size() > 1)
                tmpEncodedString += tmpValue;
            else
                tmpEncodedString = tmpValue;
        }

        return tmpEncodedString;
    }
};

const std::vector<unsigned char> Password::_encodingArray = { 0x1a, 0x1f, 0x11, 0x0a, 0x1e, 0x10, 0x18, 0x02, 0x1d, 0x08, 0x14, 0x0f, 0x1c, 0x0b, 0x0d, 0x04, 0x13, 0x17, 0x00, 0x0c, 0x0e, 0x1b, 0x06, 0x12, 0x15, 0x03, 0x09, 0x07, 0x16, 0x01, 0x19, 0x05, 0x12, 0x1d, 0x07, 0x19, 0x0f, 0x1f, 0x16, 0x1b, 0x09, 0x1a, 0x03, 0x0d, 0x13, 0x0e, 0x14, 0x0b, 0x05, 0x02, 0x17, 0x10, 0x0a, 0x18, 0x1c, 0x11, 0x06, 0x1e, 0x00, 0x15, 0x0c, 0x08, 0x04, 0x01 };
const std::vector<unsigned char> Password::_alphabetArray = { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a };
