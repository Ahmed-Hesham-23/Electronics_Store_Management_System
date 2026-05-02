#include "Products.cpp"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <sstream>
using namespace std;

class inventory
{
private:
    vector<product *> products;         // owns every pointer in this list
    string FILE_PATH = "Warehouse.txt"; // persistence file name

    //  Private helper: linear search by product ID:: Returns the index inside 'products', or -1 if not found.

    int indexOfId(const string &id)
    {
        for (int i = 0; i < (int)products.size(); i++)
        {
            if (products[i]->getProductId() == id)
                return i;
        }
        return -1;
    }

public:
    // Construtors
    inventory() = default;
    inventory(const inventory &) = delete;
    inventory &operator=(const inventory &) = delete;
    // Destructor
    ~inventory()
    {
        for (auto *p : products)
            delete p;
    }
    // Core Updatations add - remove - search
    bool addProduct(product *p)
    {
        if (!p)
            throw invalid_argument("There is no product to be added!\n");
        if (indexOfId(p->getProductId()) != -1)
        {
            cout << "Product ID " << p->getProductId() << " is already exists!\n";
            return false;
        }
        products.push_back(p);
        return true;
    }

    bool removeProduct(const string &id)
    {
        int index = indexOfId(id);
        if (index == -1)
        {
            cout << "Product with ID " << id << " doesn't exists\n";
            return false;
        }
        delete products[index];
        products.erase(products.begin() + index);
        return true;
    }
    // Searching By ID , Name , Brand , Price Range
    product *findById(const string &id)
    {
        int index = indexOfId(id);
        return (index == -1) ? NULL : products[index];
    }
    vector<product *> findByName(const string &keyword) const
    {
        vector<product *> results;
        string kw = keyword;
        transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
        for (auto *p : products)
        {
            string n = p->getName();
            transform(n.begin(), n.end(), n.begin(), ::tolower);
            if (n.find(kw) != string::npos)
                results.push_back(p);
        }
        return results;
    }

    vector<product *> findByBrand(const string &brand)
    {
        vector<product *> result;
        string br = brand;
        transform(br.begin(), br.end(), br.begin(), ::tolower);

        for (auto *p : products)
        {
            string tempBrand = p->getBrand();
            transform(tempBrand.begin(), tempBrand.end(), tempBrand.begin(), ::tolower);

            if (tempBrand == br)
                result.push_back(p);
        }
        return result;
    }

    vector<product *> findByPriceRange(double min, double max)
    {
        if (min < 0 || max < min || max <= 0)
            throw invalid_argument("Invalid Price Range\n");

        vector<product *> result;
        for (auto *p : products)
        {
            if (p->getCurrentPrice() >= min && p->getCurrentPrice() <= max)
                result.push_back(p);
        }
        return result;
    }
    // Update the stock when new products entered the warehouse
    bool restock(const string &id, int qty)
    {
        product *p = findById(id);
        if (!p)
            return false;
        p->restock(qty);
        return true;
    }
    // Update the stock when selling
    bool sell(const string &id, int qty)
    {
        product *p = findById(id);
        if (!p)
            return false;
        return p->sell(qty);
    }
    // Discount Managemnt
    bool applyDiscount(const string &id, double amount, DiscountType type)
    {
        product *p = findById(id);
        if (!p)
            return false;
        p->setDiscountSale(amount, type);
        return true;
    }

    bool discontinue(const string &id)
    {
        product *p = findById(id);
        if (!p)
            return false;
        p->discontinue();
        return true;
    }
    // Statistics & Reporting
    // Total number of distinct products in the inventory
    int count() const { return (int)products.size(); }
    // True when the inventory holds no products at all
    bool isEmpty() const { return products.empty(); }
    // Read-only access to the full product list (for iteration by callers)
    const vector<product *> &all() { return products; }

    // Number of products that are currently available (in stock)
    int countAvailable() const
    {
        int n = 0;
        for (auto *p : products)
            if (p->isAvailable())
                n++;
        return n;
    }
    // Number of products that are NOT currently available
    int countOutOfStock() const
    {
        int n = 0;
        for (auto *p : products)
            if (!p->isAvailable())
                n++;
        return n;
    }
    // Total monetary value of all stock on hand (price × quantity, every product)
    double totalInventoryValue() const
    {
        double v = 0;
        for (auto *p : products)
            v += p->getCurrentPrice() * p->getStockQuantity();
        return v;
    }
    // Printing --> Calls displayDetails() on every product (runtime polymorphism: each subclass prints its own specialized format).
    void printAll() const
    {
        if (products.empty())
        {
            cout << "  No products in inventory.\n";
            return;
        }
        cout << "\n  ===== PRODUCT CATALOG (" << count() << " items) =====\n";
        for (auto *p : products)
            p->displayDetails(); // Polymorphism
    }
    //  Runs findByName and displays every match, or a not-found message.
    void printSearchResults(const string &keyword) const
    {
        auto res = findByName(keyword);
        if (res.empty())
        {
            cout << "  No products matching '" << keyword << "'.\n";
            return;
        }
        for (auto *p : res)
            p->displayDetails();
    }
    // Prints a one-screen overview: counts and total stock value.
    void printInventorySummary() const
    {
        cout << "\n  ===== INVENTORY SUMMARY =====\n";
        cout << "  Total Products   : " << count() << "\n";
        cout << "  Available        : " << countAvailable() << "\n";
        cout << "  Out of Stock     : " << countOutOfStock() << "\n";
        cout << "  Inventory Value  : $" << fixed << setprecision(2)
             << totalInventoryValue() << "\n";
    }
    // Maintenance
    // =========================================================================
    //  File I/O  – CSV format with a header row
    // =========================================================================
    //
    //  Column layout (35 columns total, 0-indexed):
    //
    //   Common (all types)   : 0:type  1:productId  2:sku  3:name  4:brand
    //                          5:model  6:mainPrice  7:stockQuantity  8:description
    //
    //   Smartphone only      : 9:ram  10:storage  11:batteryMah  12:inches
    //                          13:cameraMP  14:phoneOS  15:has5G  16:hasNFC
    //                          17:refreshRateHz  18:chipset
    //
    //   Laptop only          : 19:ramGB  20:storageGB  21:isSSD  22:displayInches
    //                          23:laptopOS  24:cpu  25:gpu  26:batteryWh
    //                          27:formFactor  28:weightKg
    //
    //   Accessory only       : 29:accessoryCategory  30:color  31:material
    //                          32:connectivity  33:isWireless  34:compatibleWith
    //
    //  Columns that don't apply to a product type are written as "none".
    //  Fields that contain commas are wrapped in double quotes.
    //  The compatibleWith field uses '|' as an internal separator so it
    //  remains a single CSV cell.
    // =========================================================================

    //  Writes the full inventory to FILE_PATH in CSV format:: Throws runtime_error if the file cannot be opened.
    void saveToFile() const
    {
        ofstream file(FILE_PATH);
        if (!file.is_open())
            throw runtime_error("Can't open the file!\n");
        file << "type,productId,sku,name,brand,model,mainPrice,stockQuantity,description,"
             << "ram,storage,batteryMah,inches,cameraMP,phoneOS,has5G,hasNFC,refreshRateHz,chipset,"
             << "ramGB,storageGB,isSSD,displayInches,laptopOS,cpu,gpu,batteryWh,formFactor,weightKg,"
             << "accessoryCategory,color,material,connectivity,isWireless,compatibleWith\n";

        for (auto *p : products)
        {
            string type = p->getType();

            // Helper lambda: wrap a field in quotes if it contains a comma
            auto q = [](const string &s) -> string
            {
                if (s.find(',') != string::npos)
                    return "\"" + s + "\"";
                return s;
            };
            // --- Common columns (1-9) ---
            file << q(type) << ","
                 << q(p->getProductId()) << ","
                 << q(p->getSku()) << ","
                 << q(p->getName()) << ","
                 << q(p->getBrand()) << ","
                 << q(p->getModel()) << ","
                 << p->getMainPrice() << ","
                 << p->getStockQuantity() << ","
                 << q(p->getDescription()) << ",";
            if (type == "smartphone")
            {
                auto *sp = dynamic_cast<smartPhone *>(p);
                // Smartphone columns (10-19)
                file << sp->getRam() << ","
                     << sp->getStorage() << ","
                     << sp->getBattery() << ","
                     << sp->getDisplay() << ","
                     << sp->getCamera() << ","
                     << q(sp->getOS()) << ","
                     << sp->getHas5G() << ","
                     << sp->getHasNFC() << ","
                     << sp->getRefreshRate() << ","
                     << q(sp->getChipset()) << ",";
                // Laptop columns (20-29) — none
                file << "none,none,none,none,none,none,none,none,none,";
                // Accessory columns (30-35) — none
                file << "none,none,none,none,none,none";
            }
            else if (type == "laptop")
            {
                auto *lp = dynamic_cast<laptop *>(p);
                // Smartphone columns (10-19) — none
                file << "none,none,none,none,none,none,none,none,none,none,";
                // Laptop columns (20-29)
                file << lp->getRam() << ","
                     << lp->getStorage() << ","
                     << lp->getSSD() << ","
                     << lp->getDisplay() << ","
                     << q(lp->getOS()) << ","
                     << q(lp->getCPU()) << ","
                     << q(lp->getGPU()) << ","
                     << lp->getBatteryWh() << ","
                     << static_cast<int>(lp->getFormFactor()) << ","
                     << lp->getWeightKg() << ","; // ← see note below
                // Accessory columns (30-35) — none
                file << "none,none,none,none,none,none";
            }
            else // "Accessory"
            {
                auto *ac = dynamic_cast<accessory *>(p);
                // Smartphone columns (10-19) — none
                file << "none,none,none,none,none,none,none,none,none,none,";
                // Laptop columns (20-29) — none
                file << "none,none,none,none,none,none,none,none,none,";
                // Accessory columns (30-35)
                file << static_cast<int>(ac->getCategory()) << ","
                     << q(ac->getColor()) << ","
                     << q(ac->getMaterial()) << ","
                     << q(ac->getConnectivity()) << ","
                     << ac->getIsWireless() << ",";
                // compatibleWith — join with '|' so it stays one CSV field
                const auto &devs = ac->getCompatibleDevices();
                string compat;
                for (size_t i = 0; i < devs.size(); ++i)
                {
                    if (i)
                        compat += "|";
                    compat += devs[i];
                }
                file << q(compat.empty() ? "none" : compat);
            }

            file << "\n";
        }
        file.close();
        cout << "  Inventory saved to " << FILE_PATH << " (" << count() << " products).\n";
    }
    //  Reads FILE_PATH, clears the current inventory, and rebuilds every
    //  product from the CSV rows with all their specialised fields.
    void loadFromFile()
    {
        ifstream file(FILE_PATH);
        if (!file.is_open())
        {
            cout << "  No existing warehouse file found at " << FILE_PATH << ". Starting fresh.\n";
            return;
        }
        for (auto *p : products)
            delete p;
        products.clear();
        string line;
        getline(file, line);
        auto splitCSV = [](const string &row) -> vector<string>
        {
            vector<string> cols;
            string cur;
            bool inQuote = false;
            for (char c : row)
            {
                if (c == '"')
                {
                    inQuote = !inQuote;
                }
                else if (c == ',' && !inQuote)
                {
                    cols.push_back(cur);
                    cur.clear();
                }
                else
                {
                    cur += c;
                }
            }
            cols.push_back(cur);
            return cols;
        };
        auto toInt = [](const string &s)
        { return (s == "none" || s.empty()) ? 0 : stoi(s); };
        auto toDbl = [](const string &s)
        { return (s == "none" || s.empty()) ? 0.0 : stod(s); };
        auto toBool = [](const string &s)
        { return s == "1"; };
        auto toStr = [](const string &s)
        { return (s == "none") ? "" : s; };
        int lineNum = 1;
        while (getline(file, line))
        {
            ++lineNum;
            if (line.empty())
                continue;

            vector<string> c = splitCSV(line);
            if (c.size() < 35)
            {
                cerr << "  Warning: line " << lineNum << " has too few columns, skipping.\n";
                continue;
            }

            // Columns indices:
            // 0:type  1:productId  2:sku  3:name  4:brand  5:model
            // 6:mainPrice  7:stockQuantity  8:description
            // Smartphone: 9-18
            // Laptop:     19-28
            // Accessory:  29-34

            string type = c[0];
            try
            {
                if (type == "smartphone")
                {
                    auto *sp = new smartPhone(
                        c[1], c[2], c[3], c[4], c[5],
                        toDbl(c[6]), toInt(c[7]),
                        toInt(c[9]),  // ram
                        toInt(c[10]), // storage
                        toInt(c[11]), // battery
                        toDbl(c[12]), // display inches
                        toInt(c[13]), // camera MP
                        toStr(c[14])  // OS
                    );
                    sp->setDescription(toStr(c[8]));
                    sp->setHas5G(toBool(c[15]));
                    sp->setHasNFC(toBool(c[16]));
                    if (toInt(c[17]) > 0)
                        sp->setRefreshRate(toInt(c[17]));
                    sp->setChipset(toStr(c[18]));
                    products.push_back(sp);
                }
                else if (type == "laptop")
                {
                    auto *lp = new laptop(
                        c[1], c[2], c[3], c[4], c[5],
                        toDbl(c[6]), toInt(c[7]),
                        toInt(c[19]),  // ramGB
                        toInt(c[20]),  // storageGB
                        toBool(c[21]), // isSSD
                        toDbl(c[22]),  // displayInches
                        toStr(c[23]),  // OS
                        toStr(c[24]),  // cpu
                        toStr(c[25]),  // gpu
                        toInt(c[26]),  // batteryWh
                        static_cast<LaptopFormFactor>(toInt(c[27])),
                        toDbl(c[28]) // weightKg
                    );
                    lp->setDescription(toStr(c[8]));
                    products.push_back(lp);
                }
                else if (type == "accessory")
                {
                    auto *ac = new accessory(
                        c[1], c[2], c[3], c[4], c[5],
                        toDbl(c[6]), toInt(c[7]),
                        static_cast<AccessoryCategory>(toInt(c[29])),
                        toStr(c[30]), // color
                        toStr(c[31])  // material
                    );
                    ac->setDescription(toStr(c[8]));
                    ac->setConnectivity(toStr(c[32]));
                    ac->setWireless(toBool(c[33]));
                    // compatibleWith: '|'-separated
                    string compat = toStr(c[34]);
                    if (!compat.empty())
                    {
                        stringstream ss(compat);
                        string dev;
                        while (getline(ss, dev, '|'))
                            if (!dev.empty())
                                ac->addCompatibility(dev);
                    }
                    products.push_back(ac);
                }
                else
                {
                    cerr << "  Warning: unknown type '" << type << "' on line " << lineNum << ", skipping.\n";
                }
            }
            catch (const exception &e)
            {
                cerr << "  Error on line " << lineNum << ": " << e.what() << ", skipping.\n";
            }
        }
        file.close();
        cout << "  Loaded " << count() << " products from " << FILE_PATH << ".\n";
    }
    inventory &operator+=(product *p)
    {
        addProduct(p);
        return *this;
    }

    inventory &operator-=(const string &id)
    {
        if (!removeProduct(id))
            cout << "  [Inventory] Product '" << id << "' not found for removal.\n";
        return *this;
    }

    friend ostream &operator<<(ostream &out, const inventory &inv)
    {
        out << "Inventory: " << inv.count() << " product(s) | "
            << inv.countAvailable() << " available | "
            << "Value: $" << fixed << setprecision(2) << inv.totalInventoryValue();
        return out;
    }
};

int main()
{
    return 0;
}