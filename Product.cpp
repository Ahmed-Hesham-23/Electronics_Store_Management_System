#include "product.h"
using namespace std;

void product::updateStatus() {
    if (status == ProductStatus::DISCONTINUED || status == ProductStatus::COMING_SOON)
        return;
    status = (stockQuantity > 0) ? ProductStatus::AVAILABLE : ProductStatus::OUT_OF_STOCK;
}

string product::statusToString() const {
    switch (status) {
        case ProductStatus::AVAILABLE: return "Available";
        case ProductStatus::OUT_OF_STOCK: return "Out of Stock";
        case ProductStatus::DISCONTINUED: return "Discontinued";
        case ProductStatus::COMING_SOON: return "Coming Soon";
        default: return "Unknown";
    }
}

void product::displayBase(std::ostream &out) const {
    out << "========================================\n";
    out << " Name : " << name << " (" << brand << " " << model << ")\n";
    out << " SKU  : " << sku << " | ID: " << productId << "\n";
    out << " Price: $" << fixed << setprecision(2) << currentPrice;
    if (onSale) out << " [ON SALE - was $" << fixed << setprecision(2) << mainPrice << "]";
    out << "\n";
    out << " Stock: " << stockQuantity << " | Status: " << statusToString() << "\n";
    if (!description.empty()) out << " Desc : " << description << "\n";
}

product::product() : mainPrice(0), currentPrice(0), discount(0), onSale(false), stockQuantity(0), status(ProductStatus::AVAILABLE) {}

product::product(const string &id, const string &sku, const string &name, const string &brand, const string &model, double price, int stock)
    : productId(id), sku(sku), name(name), brand(brand), model(model), mainPrice(price), currentPrice(price), discount(0), onSale(false), stockQuantity(stock),
      status(stock > 0 ? ProductStatus::AVAILABLE : ProductStatus::OUT_OF_STOCK) {
    if (price < 0) throw invalid_argument("Price cannot be negative.");
    if (stock < 0) throw invalid_argument("Stock cannot be negative.");
}

void product::print(ostream &out) const { displayBase(out); }
product::~product() {}

void product::setDescription(const string &d) { description = d; }
void product::markComingSoon() { status = ProductStatus::COMING_SOON; stockQuantity = 0; }

void product::setDiscountSale(double amount, DiscountType type) {
    if (type == DiscountType::PERCENTAGE) {
        if (amount < 0 || amount > 100) throw invalid_argument("Percentage must be between 0 and 100.");
        discount = mainPrice * (amount / 100.0);
    } else {
        if (amount < 0 || amount > mainPrice) throw invalid_argument("Fixed discount amount must be between 0 and the product price.");
        discount = amount;
    }
    currentPrice = mainPrice - discount;
    onSale = (discount > 0);
}

void product::resetDiscount() { discount = 0; currentPrice = mainPrice; onSale = false; }
double product::getFinalPrice(double taxes) const { return currentPrice + currentPrice * (taxes / 100.0); }

bool product::sell(int quantity) {
    if (quantity <= 0) throw invalid_argument("Incorrect quantity to be sold");
    if (stockQuantity < quantity) return false;
    stockQuantity -= quantity;
    updateStatus();
    return true;
}

void product::restock(int quantity) {
    if (quantity <= 0) throw invalid_argument("Incorrect quantity to be restocked");
    stockQuantity += quantity;
    updateStatus();
}

bool product::isAvailable() const { return status == ProductStatus::AVAILABLE && stockQuantity > 0; }
void product::discontinue() { status = ProductStatus::DISCONTINUED; stockQuantity = 0; }

ostream &operator<<(ostream &out, const product &p) {
    p.print(out);
    return out;
}