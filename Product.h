#ifndef ELECTRONICS_STORE_MANAGEMENT_SYSTEM_PRODUCT_H
#define ELECTRONICS_STORE_MANAGEMENT_SYSTEM_PRODUCT_H

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <stdexcept>
#include <algorithm>

enum class ProductStatus {
    AVAILABLE,
    OUT_OF_STOCK,
    DISCONTINUED,
    COMING_SOON
};

enum class DiscountType {
    PERCENTAGE,
    FIXED_AMOUNT
};

class product {
private:
    std::string productId;
    std::string sku;
    std::string name;
    std::string brand;
    std::string model;
    std::string description;
    double mainPrice;
    double currentPrice;
    double discount;
    bool onSale;
    ProductStatus status;
    int stockQuantity;

    void updateStatus();
    std::string statusToString() const;

protected:
    void displayBase(std::ostream &out) const;

public:
    product();
    product(const std::string &id, const std::string &sku,
            const std::string &name, const std::string &brand,
            const std::string &model, double price, int stock);

    virtual void displayDetails() const = 0;
    virtual void print(std::ostream &out) const;
    virtual ~product();

    // Getters
    std::string getProductId() const { return productId; }
    std::string getSku() const { return sku; }
    std::string getName() const { return name; }
    std::string getBrand() const { return brand; }
    std::string getModel() const { return model; }
    std::string getDescription() const { return description; }
    double getMainPrice() const { return mainPrice; }
    double getCurrentPrice() const { return currentPrice; }
    double getDiscount() const { return discount; }
    bool getOnSale() const { return onSale; }
    int getStockQuantity() const { return stockQuantity; }
    ProductStatus getStatus() const { return status; }

    // Setters & Mutators
    void setDescription(const std::string &d);
    void markComingSoon();
    void setDiscountSale(double amount, DiscountType type);
    void resetDiscount();
    double getFinalPrice(double taxes = 0) const;
    bool sell(int quantity);
    void restock(int quantity);
    bool isAvailable() const;
    void discontinue();

    bool operator==(const product &other) const { return productId == other.productId; }
    bool operator<(const product &other) const { return currentPrice < other.currentPrice; }
    friend std::ostream &operator<<(std::ostream &out, const product &p);
};

#endif //ELECTRONICS_STORE_MANAGEMENT_SYSTEM_PRODUCT_H
