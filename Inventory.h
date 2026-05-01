#ifndef TEST_INVENTORY_H
#define TEST_INVENTORY_H
using namespace std;
#include <iostream>
#include <vector>
#include "Product.h"


class Inventory {
private:
    vector<product*> products;

public:
    void addProduct(product* p);

    void removeProduct(string id);

    product* searchProduct(string id);

    void updateStock(string id, int qty);

    vector<product*> getAllProducts();
};


#endif
