#include "Product.cpp"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;

// =============================================================================
// Inventory
// =============================================================================

class Inventory
{
private:
    vector<product *> products;
    const string FILE_PATH = "Warehouse.csv";
    // Find index of a product by ID
    // Returns: index if found, -1 otherwise
    int indexOfId(const string &id) const
    {
        for (int i = 0; i < static_cast<int>(products.size()); ++i)
            if (products[i]->getProductId() == id)
                return i;
        return -1;
    }
    // CSV helper: wraps string in quotes if it contains a comma
    // Prevents breaking CSV structure
    static string csvQuote(const string &s)
    {
        return (s.find(',') != string::npos) ? "\"" + s + "\"" : s;
    }

public:
    // Disable copying (to avoid shallow copy of pointers)
    Inventory() = default;
    Inventory(const Inventory &) = delete;
    Inventory &operator=(const Inventory &) = delete;
    // Destructor: free all dynamically allocated products
    ~Inventory()
    {
        for (auto *p : products)
            delete p;
    }
    // Add a new product (ensures unique ID)
    bool addProduct(product *p)
    {
        if (!p)
            throw invalid_argument("Cannot add a null product.");
        if (indexOfId(p->getProductId()) != -1)
        {
            cout << "Product ID '" << p->getProductId() << "' already exists.\n";
            return false;
        }
        products.push_back(p);
        saveToFile();
        return true;
    }
    // Remove product by ID
    bool removeProduct(const string &id)
    {
        int index = indexOfId(id);
        if (index == -1)
        {
            cout << "Product with ID '" << id << "' not found.\n";
            return false;
        }
        delete products[index];
        products.erase(products.begin() + index);
        saveToFile();
        return true;
    }
    // Searching about Prodcuts by differenet ways
    // Way1:
    product *findById(const string &id) const
    {
        int index = indexOfId(id);
        return (index == -1) ? nullptr : products[index];
    }
    //Way2:
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
    //Way3:
    vector<product *> findByBrand(const string &brand) const
    {
        vector<product *> result;
        string br = brand;
        transform(br.begin(), br.end(), br.begin(), ::tolower);
        for (auto *p : products)
        {
            string b = p->getBrand();
            transform(b.begin(), b.end(), b.begin(), ::tolower);
            if (b == br)
                result.push_back(p);
        }
        return result;
    }
    //Way4:
    vector<product *> findByPriceRange(double minPrice, double maxPrice) const
    {
        if (minPrice < 0 || maxPrice < minPrice || maxPrice <= 0)
            throw invalid_argument("Invalid price range.");
        vector<product *> result;
        for (auto *p : products)
            if (p->getCurrentPrice() >= minPrice && p->getCurrentPrice() <= maxPrice)
                result.push_back(p);
        return result;
    }
    // Stock operations
    bool restock(const string &id, int qty)
    {
        product *p = findById(id);
        if (!p)
        {
            cout << "Restock failed: product '" << id << "' not found.\n";
            return false;
        }
        p->restock(qty);
        saveToFile();
        return true;
    }

    bool sell(const string &id, int qty)
    {
        product *p = findById(id);
        if (!p)
        {
            cout << "Sale failed: product '" << id << "' not found.\n";
            return false;
        }
        bool sold = p->sell(qty);
        if (sold)
            saveToFile();
        return sold;
    }
    // Pricing / status operations
    bool applyDiscount(const string &id, double amount, DiscountType type)
    {
        product *p = findById(id);
        if (!p)
        {
            cout << "Discount failed: product '" << id << "' not found.\n";
            return false;
        }
        p->setDiscountSale(amount, type);
        saveToFile();
        return true;
    }

    bool discontinue(const string &id)
    {
        product *p = findById(id);
        if (!p)
        {
            cout << "Discontinue failed: product '" << id << "' not found.\n";
            return false;
        }
        p->discontinue();
        saveToFile();
        return true;
    }
    // Statistics
    int count() const { return static_cast<int>(products.size()); }
    bool isEmpty() const { return products.empty(); }

    const vector<product *> &all() const { return products; }

    int countAvailable() const
    {
        int n = 0;
        for (auto *p : products)
            if (p->isAvailable())
                ++n;
        return n;
    }

    int countOutOfStock() const
    {
        int n = 0;
        for (auto *p : products)
            if (!p->isAvailable())
                ++n;
        return n;
    }

    double totalInventoryValue() const
    {
        double v = 0;
        for (auto *p : products)
            v += p->getCurrentPrice() * p->getStockQuantity();
        return v;
    }
    // Display functions
    void printAll() const
    {
        if (products.empty())
        {
            cout << "No products in inventory.\n";
            return;
        }
        cout << "\n===== PRODUCT CATALOG (" << count() << " items) =====\n";
        for (auto *p : products)
            p->displayDetails();
    }

    void printSearchResults(const string &keyword) const
    {
        auto res = findByName(keyword);
        if (res.empty())
        {
            cout << "No products matching '" << keyword << "'.\n";
            return;
        }
        for (auto *p : res)
            p->displayDetails();
    }

    void printOutOfStock() const
    {
        cout << "\n===== OUT-OF-STOCK / UNAVAILABLE =====\n";
        bool any = false;
        for (auto *p : products)
            if (!p->isAvailable())
            {
                cout << "[" << p->getProductId() << "] "
                     << p->getName() << " - " << p->getStatusString() << "\n";
                any = true;
            }
        if (!any)
            cout << "All products are in stock!\n";
    }
    // Reporting
    void printInventorySummary() const
    {
        cout << "\n===== INVENTORY SUMMARY =====\n";
        cout << "Total Products  : " << count() << "\n";
        cout << "Available       : " << countAvailable() << "\n";
        cout << "Out of Stock    : " << countOutOfStock() << "\n";
        cout << "Inventory Value : $"
             << fixed << setprecision(2) << totalInventoryValue() << "\n";
        map<string, int> byType;
        for (auto *p : products)
            byType[p->getType()]++;
        cout << "By Type:\n";
        for (auto &[type, cnt] : byType)
            cout << "  " << type << ": " << cnt << "\n";
    }

    // File I/O - CSV format (41 columns, 0-indexed)
    // Common    : 0:type 1:productId 2:sku 3:name 4:brand 5:model
    //             6:mainPrice 7:currentPrice 8:discount 9:onSale
    //             10:stockQuantity 11:description
    // Smartphone: 12:ram 13:storage 14:batteryMah 15:inches 16:cameraMP
    //             17:phoneOS 18:has5G 19:hasNFC 20:refreshRateHz 21:chipset
    // Laptop    : 22:ramGB 23:storageGB 24:isSSD 25:displayInches 26:laptopOS
    //             27:cpu 28:gpu 29:batteryWh 30:formFactor 31:weightKg
    //             32:touchscreen 33:usbPorts 34:thunderbolt
    // Accessory : 35:category 36:color 37:material 38:connectivity
    //             39:isWireless 40:compatibleWith
    // Unused columns for a type are written as "none".
    // compatibleWith uses '|' as an internal separator.

    void saveToFile() const
    {
        // Opening File
        ofstream file(FILE_PATH);
        // Validation if not exists
        if (!file.is_open())
            throw runtime_error("Cannot open '" + FILE_PATH + "' for writing.");
        // Table Headers
        file << "type,productId,sku,name,brand,model,"
                "mainPrice,currentPrice,discount,onSale,stockQuantity,description,"
                "ram,storage,batteryMah,inches,cameraMP,phoneOS,has5G,hasNFC,refreshRateHz,chipset,"
                "ramGB,storageGB,isSSD,displayInches,laptopOS,cpu,gpu,batteryWh,"
                "formFactor,weightKg,touchscreen,usbPorts,thunderbolt,"
                "accessoryCategory,color,material,connectivity,isWireless,compatibleWith\n";
        // Rows as products
        for (auto *p : products)
        {
            const string &type = p->getType();
            // Checker if there is "," as usual string not csv format
            file << csvQuote(type) << ","
                 << csvQuote(p->getProductId()) << ","
                 << csvQuote(p->getSku()) << ","
                 << csvQuote(p->getName()) << ","
                 << csvQuote(p->getBrand()) << ","
                 << csvQuote(p->getModel()) << ","
                 << p->getMainPrice() << ","
                 << p->getCurrentPrice() << ","
                 << p->getDiscount() << ","
                 << p->getOnSale() << ","
                 << p->getStockQuantity() << ","
                 << csvQuote(p->getDescription()) << ",";

            if (type == "smartphone")
            {
                // Dynamic Casting usage due to differnece in implemntation of each derived class
                auto *sp = dynamic_cast<SmartPhone *>(p);
                file << sp->getRam() << ","
                     << sp->getStorage() << ","
                     << sp->getBattery() << ","
                     << sp->getDisplay() << ","
                     << sp->getCamera() << ","
                     << csvQuote(sp->getOS()) << ","
                     << sp->getHas5G() << ","
                     << sp->getHasNFC() << ","
                     << sp->getRefreshRate() << ","
                     << csvQuote(sp->getChipset()) << ",";
                     // setting all unrelated data as NONE
                file << "none,none,none,none,none,none,none,none,none,none,none,none,none,"
                        "none,none,none,none,none,none";
            }
            else if (type == "laptop")
            {
                // Dynamic Casting usage due to differnece in implemntation of each derived class
                auto *lp = dynamic_cast<Laptop *>(p);
                // setting all unrelated data as NONE
                file << "none,none,none,none,none,none,none,none,none,none,";
                file << lp->getRam() << ","
                     << lp->getStorage() << ","
                     << lp->getSSD() << ","
                     << lp->getDisplay() << ","
                     << csvQuote(lp->getOS()) << ","
                     << csvQuote(lp->getCPU()) << ","
                     << csvQuote(lp->getGPU()) << ","
                     << lp->getBatteryWh() << ","
                     << static_cast<int>(lp->getFormFactor()) << ","
                     << lp->getWeightKg() << ","
                     << lp->getTouchscreen() << ","
                     << lp->getUsbPorts() << ","
                     << lp->getThunderbolt() << ",";
                    // setting all unrelated data as NONE
                file << "none,none,none,none,none,none";
            }
            else
            {
                // Dynamic Casting usage due to differnece in implemntation of each derived class
                auto *ac = dynamic_cast<Accessory *>(p);
                // setting all unrelated data as NONE
                file << "none,none,none,none,none,none,none,none,none,none,"
                        "none,none,none,none,none,none,none,none,none,none,none,none,none,";
                file << static_cast<int>(ac->getCategory()) << ","
                     << csvQuote(ac->getColor()) << ","
                     << csvQuote(ac->getMaterial()) << ","
                     << csvQuote(ac->getConnectivity()) << ","
                     << ac->getIsWireless() << ",";
                const auto &devs = ac->getCompatibleDevices();
                string compat;
                for (size_t i = 0; i < devs.size(); ++i)
                {
                    if (i)
                        compat += "|";
                    compat += devs[i];
                }
                file << csvQuote(compat.empty() ? "none" : compat);
            }

            file << "\n";
        }
        file.close();
        cout << "Inventory saved to " << FILE_PATH << " (" << count() << " products).\n";
    }

    void loadFromFile()
    {
        ifstream file(FILE_PATH);
        if (!file.is_open())
        {
            cout << "No existing warehouse file found at '" << FILE_PATH << "'. Starting fresh.\n";
            return;
        }
        // Lambda Function to convert CSV into suitable form (csv line to vector of strings)
        auto splitCSV = [](const string &row) -> vector<string>
        {
            vector<string> cols;
            string cur;
            bool inQuote = false; // track if we are inside quotes
            for (char c : row)
            {
                if (c == '"')
                    inQuote = !inQuote; // toggle quote mode
                else if (c == ',' && !inQuote)
                {
                    // comma outside quotes → new column
                    cols.push_back(cur);
                    cur.clear();
                }
                else
                    cur += c; // build current column
            }
            cols.push_back(cur);
            return cols; // push last column
        };
        // Data already collected as strings so the following is lamdas to convert to the right type
        auto toInt = [](const string &s)
        { return (s == "none" || s.empty()) ? 0 : stoi(s); };
        auto toDbl = [](const string &s)
        { return (s == "none" || s.empty()) ? 0.0 : stod(s); };
        auto toBool = [](const string &s)
        { return s == "1"; };
        auto toStr = [](const string &s)
        { return (s == "none") ? "" : s; };

        string line; // line to be read from csv file
        getline(file, line);
        vector<product *> loaded; // returned vector of products from existed file
        int lineNum = 1;

        while (getline(file, line))
        {
            ++lineNum;
            if (line.empty())
                continue;

            vector<string> c = splitCSV(line); // Apply the lamda function on every line
            if (c.size() < 41)
            {
                //Neglect any data with lenght less than what is expected
                cerr << "Warning: line " << lineNum << " has too few columns, skipping.\n";
                continue;
            }
            // Starting to build the vector product based on each type of products read from the file
            // Keeping in mind validation for the inputs
            const string &type = c[0];
            // using try and catch to negelect any wrong type even that its lenght is logically correct
            try
            {
                if (type == "smartphone")
                {
                    auto *sp = new SmartPhone(
                        c[1], c[2], c[3], c[4], c[5],
                        toDbl(c[6]), toInt(c[10]),
                        toInt(c[12]), toInt(c[13]), toInt(c[14]),
                        toDbl(c[15]), toInt(c[16]), toStr(c[17]));
                    sp->setDescription(toStr(c[11]));
                    sp->setHas5G(toBool(c[18]));
                    sp->setHasNFC(toBool(c[19]));
                    if (toInt(c[20]) > 0)
                        sp->setRefreshRate(toInt(c[20]));
                    sp->setChipset(toStr(c[21]));
                    double disc = toDbl(c[8]);
                    if (disc > 0)
                        sp->setDiscountSale(disc, DiscountType::FIXED_AMOUNT);
                    loaded.push_back(sp);
                }
                else if (type == "laptop")
                {
                    auto *lp = new Laptop(
                        c[1], c[2], c[3], c[4], c[5],
                        toDbl(c[6]), toInt(c[10]),
                        toInt(c[22]), toInt(c[23]), toBool(c[24]),
                        toDbl(c[25]), toStr(c[26]), toStr(c[27]), toStr(c[28]),
                        toInt(c[29]), static_cast<LaptopFormFactor>(toInt(c[30])),
                        toDbl(c[31]));
                    lp->setDescription(toStr(c[11]));
                    lp->setTouchscreen(toBool(c[32]));
                    lp->setUsbPorts(toInt(c[33]));
                    lp->setThunderbolt(toBool(c[34]));
                    double disc = toDbl(c[8]);
                    if (disc > 0)
                        lp->setDiscountSale(disc, DiscountType::FIXED_AMOUNT);
                    loaded.push_back(lp);
                }
                else if (type == "accessory")
                {
                    auto *ac = new Accessory(
                        c[1], c[2], c[3], c[4], c[5],
                        toDbl(c[6]), toInt(c[10]),
                        static_cast<AccessoryCategory>(toInt(c[35])),
                        toStr(c[36]), toStr(c[37]));
                    ac->setDescription(toStr(c[11]));
                    ac->setConnectivity(toStr(c[38]));
                    ac->setWireless(toBool(c[39]));
                    double disc = toDbl(c[8]);
                    if (disc > 0)
                        ac->setDiscountSale(disc, DiscountType::FIXED_AMOUNT);
                    string compat = toStr(c[40]);
                    if (!compat.empty())
                    {
                        stringstream ss(compat);
                        string dev;
                        while (getline(ss, dev, '|'))
                            if (!dev.empty())
                                ac->addCompatibility(dev);
                    }
                    loaded.push_back(ac);
                }
                else
                {
                    cerr << "Warning: unknown type '" << type << "' on line " << lineNum << ", skipping.\n";
                }
            }
            catch (const exception &e)
            {
                cerr << "Error on line " << lineNum << ": " << e.what() << " - skipping.\n";
            }
        }
        file.close();

        for (auto *p : products)
            delete p;
        products = move(loaded);
        cout << "Loaded " << count() << " products from " << FILE_PATH << ".\n";
    }
    // Operator overloading for making the add , remove easier
    Inventory &operator+=(product *p)
    {
        addProduct(p);
        return *this;
    }
    Inventory &operator-=(const string &id)
    {
        removeProduct(id);
        return *this;
    }
    // Summary output "Op Overloading"
    // frind to access inevntory
    friend ostream &operator<<(ostream &out, const Inventory &inv)
    {
        out << "Inventory: " << inv.count() << " product(s) | "
            << inv.countAvailable() << " available | "
            << "Value: $" << fixed << setprecision(2) << inv.totalInventoryValue();
        return out;
    }
};
