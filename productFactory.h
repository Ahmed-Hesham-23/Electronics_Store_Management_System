#ifndef ELECTRONICS_STORE_MANAGEMENT_SYSTEM_PRODUCTFACTORY_H
#define ELECTRONICS_STORE_MANAGEMENT_SYSTEM_PRODUCTFACTORY_H
#include <string>
#include "Product.h"
#include <iostream>

using namespace std;

class ProductFactory {
public:
    static product* createProduct(
        string type,
        string id, string name, string brand, double price, int stock,
        string model,
        double screenSize = 0.0,
        int ram = 0,
        string processor = "",
        int batteryLife = 0,
        bool hasStylus = false
    );
};

#endif //ELECTRONICS_STORE_MANAGEMENT_SYSTEM_PRODUCTFACTORY_H
