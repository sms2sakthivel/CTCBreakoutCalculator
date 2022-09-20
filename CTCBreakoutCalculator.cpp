#include <iostream>
#include <unordered_map>
#include <string>
#include <fstream>
#include <locale>
#include <iomanip>

constexpr float operator"" _percent(unsigned long long percent)
{
    return percent * 0.01;
}

constexpr float operator"" _percent(long double percent)
{
    return percent * 0.01;
}

enum class TAX_SLAB
{
    SLAB_0_250000,
    SLAB_250001_500000,
    SLAB_500001_1000000,
    SLAB_ABOVE_1000001
};

enum class SURCHARGE_SLAB
{
    S_SLAB_0_5000000,
    S_SLAB_5000000_10000000,
    S_SLAB_10000001_20000000,
    S_SLAB_20000001_50000000,
    S_SLAB_50000001_100000000,
    S_SLAB_ABOVE_100000001

};

namespace TaxRates
{
    std::unordered_map<TAX_SLAB, float> tax_slabs{
        {TAX_SLAB::SLAB_0_250000, 0_percent},
        {TAX_SLAB::SLAB_250001_500000, 5_percent},
        {TAX_SLAB::SLAB_500001_1000000, 20_percent},
        {TAX_SLAB::SLAB_ABOVE_1000001, 30_percent}};
    std::unordered_map<SURCHARGE_SLAB, float> surcharge_slabs{
        {SURCHARGE_SLAB::S_SLAB_0_5000000, 0_percent},
        {SURCHARGE_SLAB::S_SLAB_5000000_10000000, 10_percent},
        {SURCHARGE_SLAB::S_SLAB_10000001_20000000, 15_percent},
        {SURCHARGE_SLAB::S_SLAB_20000001_50000000, 25_percent},
        {SURCHARGE_SLAB::S_SLAB_50000001_100000000, 37_percent},
        {SURCHARGE_SLAB::S_SLAB_ABOVE_100000001, 37_percent}};
    std::unordered_map<TAX_SLAB, float> health_and_education_cess{
        {TAX_SLAB::SLAB_0_250000, 0_percent},
        {TAX_SLAB::SLAB_250001_500000, 4_percent},
        {TAX_SLAB::SLAB_500001_1000000, 4_percent},
        {TAX_SLAB::SLAB_ABOVE_1000001, 4_percent}};
    std::unordered_map<TAX_SLAB, int> minimum_tax{
        {TAX_SLAB::SLAB_0_250000, 0},
        {TAX_SLAB::SLAB_250001_500000, 0},
        {TAX_SLAB::SLAB_500001_1000000, 12500},
        {TAX_SLAB::SLAB_ABOVE_1000001, 112500}};

    SURCHARGE_SLAB get_surcharge_from_taxable_income(int &taxable_income)
    {
        if (taxable_income < 50'00'000)
            return SURCHARGE_SLAB::S_SLAB_0_5000000;
        if (taxable_income > 50'00'000 && taxable_income < 1'00'00'000)
            return SURCHARGE_SLAB::S_SLAB_5000000_10000000;
        if (taxable_income > 1'00'00'000 && taxable_income < 2'00'00'000)
            return SURCHARGE_SLAB::S_SLAB_10000001_20000000;
        if (taxable_income > 2'00'00'000 && taxable_income < 5'00'00'000)
            return SURCHARGE_SLAB::S_SLAB_20000001_50000000;
        if (taxable_income > 5'00'00'000 && taxable_income < 10'00'00'000)
            return SURCHARGE_SLAB::S_SLAB_50000001_100000000;
        if (taxable_income > 10'00'00'000)
            return SURCHARGE_SLAB::S_SLAB_ABOVE_100000001;
        return SURCHARGE_SLAB::S_SLAB_0_5000000;
    }

    TAX_SLAB get_tax_slab_taxable_income(int &taxable_income)
    {
        if (taxable_income < 2'50'000)
            return TAX_SLAB::SLAB_0_250000;
        if (taxable_income > 2'50'000 && taxable_income < 5'00'000)
            return TAX_SLAB::SLAB_250001_500000;
        if (taxable_income > 5'00'000 && taxable_income < 10'00'000)
            return TAX_SLAB::SLAB_500001_1000000;
        if (taxable_income > 10'00'000)
            return TAX_SLAB::SLAB_ABOVE_1000001;
        return TAX_SLAB::SLAB_0_250000;
    }

    float get_tax_slab(int taxable_income) { return tax_slabs[get_tax_slab_taxable_income(taxable_income)]; }
    float get_surcharge(int taxable_income) { return surcharge_slabs[get_surcharge_from_taxable_income(taxable_income)]; }
    float get_health_and_education_cess(int taxable_income) { return health_and_education_cess[get_tax_slab_taxable_income(taxable_income)]; }
    int get_minimum_tax(int taxable_income) { return minimum_tax[get_tax_slab_taxable_income(taxable_income)]; }
    int get_taxable_after_minimum_tax(int taxable_income)
    {
        auto slab = get_tax_slab_taxable_income(taxable_income);
        if (slab == TAX_SLAB::SLAB_0_250000)
            return 0;
        if (slab == TAX_SLAB::SLAB_250001_500000)
            return taxable_income - 2'50'000;
        if (slab == TAX_SLAB::SLAB_500001_1000000)
            return taxable_income - 5'00'000;
        if (slab == TAX_SLAB::SLAB_ABOVE_1000001)
            return taxable_income - 10'00'000;
        return 0;
    }
    int standard_deduction = 50'000;
    int house_rent_allowance = 1'00'000;
};

class CTCBreakoutCalculator
{
private:
    float _ctc, _tds, _eepf, _erpf, _gratuvity, _base, _take_home, _taxable, _cess, _surcharge;

public:
    CTCBreakoutCalculator(float ctc)
    {
        _ctc = ctc;
        _base = _ctc * 40_percent;
        _eepf = _erpf = _base * 12_percent;
        if (_eepf > 75'000)
            _eepf = _erpf = 75'000;
        _gratuvity = _base * 4.81_percent;
        _taxable = _ctc - _eepf - _erpf - _gratuvity - TaxRates::standard_deduction - TaxRates::house_rent_allowance;
        if (_taxable < 0)
            _taxable = 0;
        _take_home = _tds = 0;
        get_take_home_salary();
    }

    int get_take_home_salary()
    {
        if (_take_home)
            return _take_home;
        return _take_home = _ctc - _eepf - _erpf - _gratuvity - get_taxes();
    }

    int get_taxes()
    {
        if (_tds)
            return _tds;
        _tds = TaxRates::get_minimum_tax(_taxable) + TaxRates::get_taxable_after_minimum_tax(_taxable) * TaxRates::get_tax_slab(_taxable);
        _cess = _tds * TaxRates::get_health_and_education_cess(_taxable);
        _surcharge = _tds * TaxRates::get_surcharge(_taxable);
        return _tds + _cess + _surcharge;
    }

    friend std::ostream &operator<<(std::ostream &out, const CTCBreakoutCalculator &obj)
    {
        out.imbue(std::locale("en_IN.UTF-8"));
        out << std::showbase;
        out << "CTC         : " << std::put_money(obj._ctc * 100) << std::endl
            << "Base        : " << std::put_money(obj._base * 100) << std::endl
            << "EEPF        : " << std::put_money(obj._eepf * 100) << std::endl
            << "ERPF        : " << std::put_money(obj._erpf * 100) << std::endl
            << "Gratuvity   : " << std::put_money(obj._gratuvity * 100) << std::endl
            << "Taxable     : " << std::put_money(obj._taxable * 100) << std::endl
            << "CESS        : " << std::put_money(obj._cess * 100) << std::endl
            << "Surcharge   : " << std::put_money(obj._surcharge * 100) << std::endl
            << "TDS         : " << std::put_money(obj._tds * 100) << std::endl
            << "TotalTax    : \033[1;31m" << std::put_money((obj._tds + obj._cess + obj._surcharge) * 100) << "\033[0m" << std::endl
            << "TakeHome    : " << std::put_money(obj._take_home * 100) << std::endl
            << "TakeHome PM : \033[1;32m" << std::put_money((obj._take_home / 12) * 100) << "\033[0m" << std::endl
            << std::endl
            << std::endl;
        return out;
    }

    friend std::ofstream &operator<<(std::ofstream &out, const CTCBreakoutCalculator &obj)
    {
        out.imbue(std::locale("en_IN.UTF-8"));
        out << std::showbase;
        out << "CTC         : " << std::put_money(obj._ctc * 100) << std::endl
            << "Base        : " << std::put_money(obj._base * 100) << std::endl
            << "EEPF        : " << std::put_money(obj._eepf * 100) << std::endl
            << "ERPF        : " << std::put_money(obj._erpf * 100) << std::endl
            << "Gratuvity   : " << std::put_money(obj._gratuvity * 100) << std::endl
            << "Taxable     : " << std::put_money(obj._taxable * 100) << std::endl
            << "CESS        : " << std::put_money(obj._cess * 100) << std::endl
            << "Surcharge   : " << std::put_money(obj._surcharge * 100) << std::endl
            << "TDS         : " << std::put_money(obj._tds * 100) << std::endl
            << "TotalTax    : " << std::put_money((obj._tds + obj._cess + obj._surcharge) * 100) << std::endl
            << "TakeHome    : " << std::put_money(obj._take_home * 100) << std::endl
            << "TakeHome PM : " << std::put_money((obj._take_home / 12) * 100) << std::endl
            << std::endl
            << std::endl;
        return out;
    }
};

int main(int argc, char **argv)
{
    std::ofstream file("SalaryBreakout.txt");
    if (argc == 2)
    {
        CTCBreakoutCalculator ctc(std::stof(argv[1]));
        std::cout << ctc;
        file << ctc;
    }
    else
    {
        CTCBreakoutCalculator ctc(0);
        std::cout << ctc;
        file << ctc;
    }
    file.close();
    return 0;
}
