//
// Created by LENOVO on 2024/10/31.
//

#ifndef IO_H
#define IO_H

#define INPUT_OPEN(x)      std::ifstream x("testfile.txt");\
                            if (!x.is_open()) {\
                            std::cerr << "Error opening file" << std::endl;\
                            return -1;\
                           }

#define OUTPUT_OPEN(x, y)    std::ofstream x(#y);\
                                if (!x.is_open()) {\
                                std::cerr << "Error opening file" << std::endl;\
                                return -1;\
                             }


#endif //IO_H
