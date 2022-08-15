#include <QtTest/QtTest>
#include <iostream>

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ExtESmry.hpp>

#include <opm/io/eclipse/EclFile.hpp>

#include <appl/qsum_cmdf.hpp>


class TestQsummary: public QObject
{
    Q_OBJECT

private slots:

    void test_1a();
    void test_1b();
    void test_1c();
    void test_1d();
    void test_1e();
    void test_1f();
    void test_1g();

    void test_2a();
    void test_2b();
    void test_2c();
    void test_2d();
    void test_2e();
    void test_2f();
    void test_2g();

    void test_3c();
};

// https://doc.qt.io/qt-6/qtest-tutorial.html

// ctest -V for verbose output

// smry_id, name and unit
using var_type = std::tuple<int, std::string, std::string>;

// name, smry_id and smry_key
using param_type = std::tuple<std::string, int, std::string>;
using param_list_type = std::vector<param_type>;

// var, params and expression
using define_type = std::tuple<var_type, param_list_type, std::string>;



bool check_define(const std::vector<define_type>& test, const std::vector<var_type>& ref_var_vect,
                  const std::vector<std::vector<param_type>>& ref_params_vect,
                  const std::vector<std::string>& ref_expr_vect)
{

    if (test.size() != ref_var_vect.size())
        return false;

    for (size_t v = 0; v < ref_var_vect.size(); v++) {
        auto define = test[v];
        auto var = std::get<0>(define);
        auto params = std::get<1>(define);
        auto expr = std::get<2>(define);

        if (std::get<0>(ref_var_vect[v]) != std::get<0>(var))
            return false;

        if (std::get<1>(ref_var_vect[v]) != std::get<1>(var))
            return false;

        if (std::get<2>(ref_var_vect[v]) != std::get<2>(var))
            return false;

        if (params.size() != ref_params_vect[v].size())
            return false;


        for (size_t p = 0; p < params.size(); p++) {
            if (std::get<0>(ref_params_vect[v][p]) != std::get<0>(params[p]))
                return false;

            if (std::get<1>(ref_params_vect[v][p]) != std::get<1>(params[p]))
                return false;

            if (std::get<2>(ref_params_vect[v][p]) != std::get<2>(params[p]))
                return false;
        }

        if (ref_expr_vect[v] != expr)
            return false;
    }

    return true;

}

void TestQsummary::test_1a()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    std::vector<var_type> ref_var_vect;
    std::vector<std::vector<param_type>> ref_params_vect;
    std::vector<std::string> ref_expr_vect;

    {
        std::string cmd_file = "../tests/cmd_files/test1a.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        auto test1a = derived_smry->get_table();
        //derived_smry->print_m_define_table();

        ref_var_vect.resize(1);
        ref_params_vect.resize(1);
        ref_expr_vect.resize(1);

        ref_expr_vect[0] = "(X1*10+X2+X3)/12.0";

        ref_var_vect[0] = {0, "WXX1:PROD-1", "SM3/DAY"};

        ref_params_vect[0] = { {"X1", 0, "WOPR:PROD-1"},
                               {"X2", 1, "WOPR:PROD-1"},
                               {"X3", 2, "WOPR:PROD-1"}
                             };

        QCOMPARE(check_define(test1a, ref_var_vect, ref_params_vect, ref_expr_vect), true);
    }

}

void TestQsummary::test_1b()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    std::vector<var_type> ref_var_vect;
    std::vector<std::vector<param_type>> ref_params_vect;
    std::vector<std::string> ref_expr_vect;

    {
        std::string cmd_file = "../tests/cmd_files/test1b.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        auto test1b = derived_smry->get_table();
        //derived_smry->print_m_define_table();

        ref_var_vect.resize(1);
        ref_params_vect.resize(1);
        ref_expr_vect.resize(1);

        ref_expr_vect[0] = "( X1 * 10 + X2 + X3 ) / 12.0";

        ref_var_vect[0] = {0, "WXX1:PROD-1", "None"};

        ref_params_vect[0] = { {"X1", 0, "WOPR:PROD-1"},
                               {"X2", 1, "WOPR:PROD-1"},
                               {"X3", 2, "WOPR:PROD-1"}
                             };

        QCOMPARE(check_define(test1b, ref_var_vect, ref_params_vect, ref_expr_vect), true);
    }

}

void TestQsummary::test_1c()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    std::vector<var_type> ref_var_vect;
    std::vector<std::vector<param_type>> ref_params_vect;
    std::vector<std::string> ref_expr_vect;

    {
        std::string cmd_file = "../tests/cmd_files/test1c.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        auto test1c = derived_smry->get_table();
        //derived_smry->print_m_define_table();

        ref_var_vect.resize(3);
        ref_params_vect.resize(3);
        ref_expr_vect.resize(3);

        ref_expr_vect[0] = "(X1*10+X2+X3)/12.0";
        ref_expr_vect[1] = "(X1*10+X2+X3)/12.0";
        ref_expr_vect[2] = "(X1*10+X2+X3)/12.0";

        ref_var_vect[0] = {0, "WXX1:PROD-1", "SM3/DAY"};
        ref_var_vect[1] = {1, "WXX1:PROD-1", "SM3/DAY"};
        ref_var_vect[2] = {2, "WXX1:PROD-1", "SM3/DAY"};

        ref_params_vect[0] = { {"X1", 0, "WOPR:PROD-1"},
                               {"X2", 1, "WOPR:PROD-1"},
                               {"X3", 2, "WOPR:PROD-1"}
                             };

        ref_params_vect[1] = { {"X1", 0, "WOPR:PROD-1"},
                               {"X2", 1, "WOPR:PROD-1"},
                               {"X3", 2, "WOPR:PROD-1"}
                             };

        ref_params_vect[2] = { {"X1", 0, "WOPR:PROD-1"},
                               {"X2", 1, "WOPR:PROD-1"},
                               {"X3", 2, "WOPR:PROD-1"}
                             };

        QCOMPARE(check_define(test1c, ref_var_vect, ref_params_vect, ref_expr_vect), true);
    }
}

void TestQsummary::test_1d()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    std::vector<var_type> ref_var_vect;
    std::vector<std::vector<param_type>> ref_params_vect;
    std::vector<std::string> ref_expr_vect;

    {
        std::string cmd_file = "../tests/cmd_files/test1d.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        auto test1d = derived_smry->get_table();
        //derived_smry->print_m_define_table();

        ref_var_vect.resize(1);
        ref_params_vect.resize(1);
        ref_expr_vect.resize(1);

        ref_expr_vect[0] = "(X1*10+X2+X3)/12.0";

        ref_var_vect[0] = {-1, "WXX1:PROD-1", "SM3/DAY"};

        ref_params_vect[0] = { {"X1", 0, "WOPR:PROD-1"},
                               {"X2", 1, "WOPR:PROD-1"},
                               {"X3", 2, "WOPR:PROD-1"}
                             };

        QCOMPARE(check_define(test1d, ref_var_vect, ref_params_vect, ref_expr_vect), true);
    }

}

void TestQsummary::test_1e()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    std::vector<var_type> ref_var_vect;
    std::vector<std::vector<param_type>> ref_params_vect;
    std::vector<std::string> ref_expr_vect;

    {
        std::string cmd_file = "../tests/cmd_files/test1e.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        auto test1e = derived_smry->get_table();
        //derived_smry->print_m_define_table();

        ref_var_vect.resize(3);
        ref_params_vect.resize(3);
        ref_expr_vect.resize(3);

        ref_expr_vect[0] = "(X1*4+X2)/5.0";
        ref_expr_vect[1] = "(X1*4+X2)/5.0";
        ref_expr_vect[2] = "(X1*4+X2)/5.0";

        ref_var_vect[0] = {0, "WXX1:PROD-X", "SM3/DAY"};
        ref_var_vect[1] = {1, "WXX1:PROD-X", "SM3/DAY"};
        ref_var_vect[2] = {2, "WXX1:PROD-X", "SM3/DAY"};


        ref_params_vect[0] = { {"X1", 0, "WOPR:PROD-1"},
                               {"X2", 0, "WOPR:PROD-2"},
                             };

        ref_params_vect[1] = { {"X1", 1, "WOPR:PROD-1"},
                               {"X2", 1, "WOPR:PROD-2"},
                             };

        ref_params_vect[2] = { {"X1", 2, "WOPR:PROD-1"},
                               {"X2", 2, "WOPR:PROD-2"},
                             };

        QCOMPARE(check_define(test1e, ref_var_vect, ref_params_vect, ref_expr_vect), true);
    }

}

void TestQsummary::test_1f()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    std::vector<var_type> ref_var_vect;
    std::vector<std::vector<param_type>> ref_params_vect;
    std::vector<std::string> ref_expr_vect;

    {
        std::string cmd_file = "../tests/cmd_files/test1f.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        auto test1f = derived_smry->get_table();
        //derived_smry->print_m_define_table();

        ref_var_vect.resize(6);
        ref_params_vect.resize(6);
        ref_expr_vect.resize(6);

        ref_expr_vect[0] = "(X1*4+X2)/5.0";
        ref_expr_vect[1] = "(X1*4+X2)/5.0";
        ref_expr_vect[2] = "(X1*4+X2)/5.0";
        ref_expr_vect[3] = "(X1+X2*3)/4.0";
        ref_expr_vect[4] = "(X1+X2*3)/4.0";
        ref_expr_vect[5] = "(X1+X2*3)/4.0";

        ref_var_vect[0] = {0, "WXX1:PROD-X", "SM3/DAY"};
        ref_var_vect[1] = {1, "WXX1:PROD-X", "SM3/DAY"};
        ref_var_vect[2] = {2, "WXX1:PROD-X", "SM3/DAY"};
        ref_var_vect[3] = {0, "WXX2:PROD-X", "SM3/DAY"};
        ref_var_vect[4] = {1, "WXX2:PROD-X", "SM3/DAY"};
        ref_var_vect[5] = {2, "WXX2:PROD-X", "SM3/DAY"};


        ref_params_vect[0] = { {"X1", 0, "WOPR:PROD-1"},
                               {"X2", 0, "WOPR:PROD-2"},
                             };

        ref_params_vect[1] = { {"X1", 1, "WOPR:PROD-1"},
                               {"X2", 1, "WOPR:PROD-2"},
                             };

        ref_params_vect[2] = { {"X1", 2, "WOPR:PROD-1"},
                               {"X2", 2, "WOPR:PROD-2"},
                             };

        ref_params_vect[3] = { {"X1", 0, "WXX1:PROD-X"},
                               {"X2", 0, "WOPR:PROD-3"},
                             };

        ref_params_vect[4] = { {"X1", 1, "WXX1:PROD-X"},
                               {"X2", 1, "WOPR:PROD-3"},
                             };

        ref_params_vect[5] = { {"X1", 2, "WXX1:PROD-X"},
                               {"X2", 2, "WOPR:PROD-3"},
                             };

        QCOMPARE(check_define(test1f, ref_var_vect, ref_params_vect, ref_expr_vect), true);
    }
}

void TestQsummary::test_1g()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    std::vector<var_type> ref_var_vect;
    std::vector<std::vector<param_type>> ref_params_vect;
    std::vector<std::string> ref_expr_vect;

    {
        std::string cmd_file = "../tests/cmd_files/test1g.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        auto test1g = derived_smry->get_table();
        //derived_smry->print_m_define_table();

        ref_var_vect.resize(9);
        ref_params_vect.resize(9);
        ref_expr_vect.resize(9);

        ref_expr_vect[0] = "X1/(X1+X2)";
        ref_expr_vect[1] = "X1/(X1+X2)";
        ref_expr_vect[2] = "X1/(X1+X2)";
        ref_expr_vect[3] = "X1/(X1+X2)";
        ref_expr_vect[4] = "X1/(X1+X2)";
        ref_expr_vect[5] = "X1/(X1+X2)";
        ref_expr_vect[6] = "X1/(X1+X2)";
        ref_expr_vect[7] = "X1/(X1+X2)";
        ref_expr_vect[8] = "X1/(X1+X2)";

        ref_var_vect[0] = {0, "WWCT2:PROD-1", "None"};
        ref_var_vect[1] = {1, "WWCT2:PROD-1", "None"};
        ref_var_vect[2] = {2, "WWCT2:PROD-1", "None"};
        ref_var_vect[3] = {0, "WWCT2:PROD-2", "None"};
        ref_var_vect[4] = {1, "WWCT2:PROD-2", "None"};
        ref_var_vect[5] = {2, "WWCT2:PROD-2", "None"};
        ref_var_vect[6] = {0, "WWCT2:PROD-3", "None"};
        ref_var_vect[7] = {1, "WWCT2:PROD-3", "None"};
        ref_var_vect[8] = {2, "WWCT2:PROD-3", "None"};


        ref_params_vect[0] = { {"X1", 0, "WWPR:PROD-1"},
                               {"X2", 0, "WOPR:PROD-1"},
                             };

        ref_params_vect[1] = { {"X1", 1, "WWPR:PROD-1"},
                               {"X2", 1, "WOPR:PROD-1"},
                             };

        ref_params_vect[2] = { {"X1", 2, "WWPR:PROD-1"},
                               {"X2", 2, "WOPR:PROD-1"},
                             };

        ref_params_vect[3] = { {"X1", 0, "WWPR:PROD-2"},
                               {"X2", 0, "WOPR:PROD-2"},
                             };

        ref_params_vect[4] = { {"X1", 1, "WWPR:PROD-2"},
                               {"X2", 1, "WOPR:PROD-2"},
                             };

        ref_params_vect[5] = { {"X1", 2, "WWPR:PROD-2"},
                               {"X2", 2, "WOPR:PROD-2"},
                             };

        ref_params_vect[6] = { {"X1", 0, "WWPR:PROD-3"},
                               {"X2", 0, "WOPR:PROD-3"},
                             };

        ref_params_vect[7] = { {"X1", 1, "WWPR:PROD-3"},
                               {"X2", 1, "WOPR:PROD-3"},
                             };

        ref_params_vect[8] = { {"X1", 2, "WWPR:PROD-3"},
                               {"X2", 2, "WOPR:PROD-3"},
                             };

        QCOMPARE(check_define(test1g, ref_var_vect, ref_params_vect, ref_expr_vect), true);
    }
}


void TestQsummary::test_2a()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    Opm::EclIO::EclFile refFile("../tests/REF_DEFINE.DATA");

    refFile.loadData();

    {
        std::string cmd_file = "../tests/cmd_files/test1a.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        auto ref0_data = refFile.get<float>("REF_0");
        auto test_data = derived_smry->get(0, "WXX1:PROD-1");

        QCOMPARE(ref0_data.size(), test_data.size());

        for (size_t n = 0; n < ref0_data.size(); n++)
            QCOMPARE(ref0_data[n], test_data[n]);
    }
}

void TestQsummary::test_2b()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    Opm::EclIO::EclFile refFile("../tests/REF_DEFINE.DATA");

    refFile.loadData();

    {
        std::string cmd_file = "../tests/cmd_files/test1b.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        auto ref1_data = refFile.get<float>("REF_1");
        auto test_data = derived_smry->get(0, "WXX1:PROD-1");

        QCOMPARE(ref1_data.size(), test_data.size());

        for (size_t n = 0; n < ref1_data.size(); n++)
            QCOMPARE(ref1_data[n], test_data[n]);
    }
}

void TestQsummary::test_2c()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    Opm::EclIO::EclFile refFile("../tests/REF_DEFINE.DATA");

    refFile.loadData();

    {
        std::string cmd_file = "../tests/cmd_files/test1c.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        std::vector<std::vector<float>> ref_data;
        std::vector<std::vector<float>> test_data;

        ref_data.push_back(refFile.get<float>("REF_2"));
        ref_data.push_back(refFile.get<float>("REF_3"));
        ref_data.push_back(refFile.get<float>("REF_4"));

        test_data.push_back(derived_smry->get(0, "WXX1:PROD-1"));
        test_data.push_back(derived_smry->get(1, "WXX1:PROD-1"));
        test_data.push_back(derived_smry->get(2, "WXX1:PROD-1"));

        QCOMPARE(test_data.size(), ref_data.size());

        for (size_t n = 0; n < test_data.size(); n++){
            QCOMPARE(test_data[n].size(), ref_data[n].size());

            for (size_t m = 0; m < test_data[n].size(); m++)
                QCOMPARE(test_data[n][m], ref_data[n][m]);
        }
    }
}

void TestQsummary::test_2d()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    Opm::EclIO::EclFile refFile("../tests/REF_DEFINE.DATA");

    refFile.loadData();

    {
        std::string cmd_file = "../tests/cmd_files/test1d.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        std::vector<std::vector<float>> ref_data;
        std::vector<std::vector<float>> test_data;

        ref_data.push_back(refFile.get<float>("REF_5"));
        ref_data.push_back(refFile.get<float>("REF_6"));

        test_data.push_back(derived_smry->get(-1, "TIME"));
        test_data.push_back(derived_smry->get(-1, "WXX1:PROD-1"));

        QCOMPARE(test_data.size(), ref_data.size());

        for (size_t n = 0; n < test_data.size(); n++){
            QCOMPARE(test_data[n].size(), ref_data[n].size());

            for (size_t m = 0; m < test_data[n].size(); m++)
                QCOMPARE(test_data[n][m], ref_data[n][m]);
        }
    }
}

void TestQsummary::test_2e()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    Opm::EclIO::EclFile refFile("../tests/REF_DEFINE.DATA");

    refFile.loadData();

    {
        std::string cmd_file = "../tests/cmd_files/test1e.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        std::vector<std::vector<float>> ref_data;
        std::vector<std::vector<float>> test_data;

        ref_data.push_back(refFile.get<float>("REF_7"));
        ref_data.push_back(refFile.get<float>("REF_8"));
        ref_data.push_back(refFile.get<float>("REF_9"));

        test_data.push_back(derived_smry->get(0, "WXX1:PROD-X"));
        test_data.push_back(derived_smry->get(1, "WXX1:PROD-X"));
        test_data.push_back(derived_smry->get(2, "WXX1:PROD-X"));

        QCOMPARE(test_data.size(), ref_data.size());

        for (size_t n = 0; n < test_data.size(); n++){
            QCOMPARE(test_data[n].size(), ref_data[n].size());

            for (size_t m = 0; m < test_data[n].size(); m++)
                QCOMPARE(test_data[n][m], ref_data[n][m]);
        }
    }
}


void TestQsummary::test_2f()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    Opm::EclIO::EclFile refFile("../tests/REF_DEFINE.DATA");

    refFile.loadData();

    {
        std::string cmd_file = "../tests/cmd_files/test1f.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        std::vector<std::vector<float>> ref_data;
        std::vector<std::vector<float>> test_data;

        ref_data.push_back(refFile.get<float>("REF_10"));
        ref_data.push_back(refFile.get<float>("REF_11"));
        ref_data.push_back(refFile.get<float>("REF_12"));
        ref_data.push_back(refFile.get<float>("REF_13"));
        ref_data.push_back(refFile.get<float>("REF_14"));
        ref_data.push_back(refFile.get<float>("REF_15"));

        test_data.push_back(derived_smry->get(0, "WXX1:PROD-X"));
        test_data.push_back(derived_smry->get(0, "WXX2:PROD-X"));
        test_data.push_back(derived_smry->get(1, "WXX1:PROD-X"));
        test_data.push_back(derived_smry->get(1, "WXX2:PROD-X"));
        test_data.push_back(derived_smry->get(2, "WXX1:PROD-X"));
        test_data.push_back(derived_smry->get(2, "WXX2:PROD-X"));

        QCOMPARE(test_data.size(), ref_data.size());

        for (size_t n = 0; n < test_data.size(); n++){
            QCOMPARE(test_data[n].size(), ref_data[n].size());

            for (size_t m = 0; m < test_data[n].size(); m++)
                QCOMPARE(test_data[n][m], ref_data[n][m]);
        }
    }
}

void TestQsummary::test_2g()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    Opm::EclIO::EclFile refFile("../tests/REF_DEFINE.DATA");

    refFile.loadData();

    {
        std::string cmd_file = "../tests/cmd_files/test1g.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        std::vector<std::vector<float>> ref_data;
        std::vector<std::vector<float>> test_data;

        ref_data.push_back(refFile.get<float>("REF_16"));
        ref_data.push_back(refFile.get<float>("REF_17"));
        ref_data.push_back(refFile.get<float>("REF_18"));
        ref_data.push_back(refFile.get<float>("REF_19"));
        ref_data.push_back(refFile.get<float>("REF_20"));
        ref_data.push_back(refFile.get<float>("REF_21"));
        ref_data.push_back(refFile.get<float>("REF_22"));
        ref_data.push_back(refFile.get<float>("REF_23"));
        ref_data.push_back(refFile.get<float>("REF_24"));

        test_data.push_back(derived_smry->get(0, "WWCT2:PROD-1"));
        test_data.push_back(derived_smry->get(0, "WWCT2:PROD-2"));
        test_data.push_back(derived_smry->get(0, "WWCT2:PROD-3"));
        test_data.push_back(derived_smry->get(1, "WWCT2:PROD-1"));
        test_data.push_back(derived_smry->get(1, "WWCT2:PROD-2"));
        test_data.push_back(derived_smry->get(1, "WWCT2:PROD-3"));
        test_data.push_back(derived_smry->get(2, "WWCT2:PROD-1"));
        test_data.push_back(derived_smry->get(2, "WWCT2:PROD-2"));
        test_data.push_back(derived_smry->get(2, "WWCT2:PROD-3"));

        QCOMPARE(test_data.size(), ref_data.size());

        for (size_t n = 0; n < test_data.size(); n++){
            QCOMPARE(test_data[n].size(), ref_data[n].size());

            for (size_t m = 0; m < test_data[n].size(); m++)
                QCOMPARE(test_data[n][m], ref_data[n][m]);
        }
    }
}

void TestQsummary::test_3c()
{
    int num_files = 3;

    std::vector<FileType> file_type;
    file_type.resize(num_files);

    file_type[0] = FileType::SMSPEC;
    file_type[1] = FileType::ESMRY;
    file_type[2] = FileType::ESMRY;

    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> lodsmry_loader;

    esmry_loader[0] = std::make_unique<Opm::EclIO::ESmry>("../tests/smry_files/SENS0.SMSPEC");
    lodsmry_loader[1] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS1.ESMRY");
    lodsmry_loader[2] = std::make_unique<Opm::EclIO::ExtESmry>("../tests/smry_files/SENS2.ESMRY");

    Opm::EclIO::EclFile refFile("../tests/REF_DEFINE.DATA");

    std::vector<var_type> ref_var_vect;
    std::vector<std::vector<param_type>> ref_params_vect;
    std::vector<std::string> ref_expr_vect;

    {
        std::string cmd_file = "../tests/cmd_files/test3c.txt";
        QsumCMDF cmdfile(cmd_file, num_files, "");

        SmryAppl::input_list_type input_charts;

        cmdfile.make_charts_from_cmd(input_charts, "");

        std::unique_ptr<DerivedSmry> derived_smry;
        derived_smry = std::make_unique<DerivedSmry>(cmdfile, file_type, esmry_loader, lodsmry_loader);

        auto test1g = derived_smry->get_table();
        //derived_smry->print_m_define_table();

        ref_var_vect.resize(9);
        ref_params_vect.resize(9);
        ref_expr_vect.resize(9);

        ref_expr_vect[0] = "X1/(X1+X2)";
        ref_expr_vect[1] = "X1/(X1+X2)";
        ref_expr_vect[2] = "X1/(X1+X2)";
        ref_expr_vect[3] = "X1/(X1+X2)";
        ref_expr_vect[4] = "X1/(X1+X2)";
        ref_expr_vect[5] = "X1/(X1+X2)";
        ref_expr_vect[6] = "X1/(X1+X2)";
        ref_expr_vect[7] = "X1/(X1+X2)";
        ref_expr_vect[8] = "X1/(X1+X2)";

        ref_var_vect[0] = {0, "WWCT2:PROD-1", "None"};
        ref_var_vect[1] = {0, "WWCT2:PROD-2", "None"};
        ref_var_vect[2] = {0, "WWCT2:PROD-3", "None"};
        ref_var_vect[3] = {1, "WWCT2:PROD-1", "None"};
        ref_var_vect[4] = {1, "WWCT2:PROD-2", "None"};
        ref_var_vect[5] = {1, "WWCT2:PROD-3", "None"};
        ref_var_vect[6] = {2, "WWCT2:PROD-1", "None"};
        ref_var_vect[7] = {2, "WWCT2:PROD-2", "None"};
        ref_var_vect[8] = {2, "WWCT2:PROD-3", "None"};


        ref_params_vect[0] = { {"X1", 0, "WWPR:PROD-1"},
                               {"X2", 0, "WOPR:PROD-1"},
                             };

        ref_params_vect[1] = { {"X1", 0, "WWPR:PROD-2"},
                               {"X2", 0, "WOPR:PROD-2"},
                             };

        ref_params_vect[2] = { {"X1", 0, "WWPR:PROD-3"},
                               {"X2", 0, "WOPR:PROD-3"},
                             };

        ref_params_vect[3] = { {"X1", 1, "WWPR:PROD-1"},
                               {"X2", 1, "WOPR:PROD-1"},
                             };

        ref_params_vect[4] = { {"X1", 1, "WWPR:PROD-2"},
                               {"X2", 1, "WOPR:PROD-2"},
                             };

        ref_params_vect[5] = { {"X1", 1, "WWPR:PROD-3"},
                               {"X2", 1, "WOPR:PROD-3"},
                             };

        ref_params_vect[6] = { {"X1", 2, "WWPR:PROD-1"},
                               {"X2", 2, "WOPR:PROD-1"},
                             };

        ref_params_vect[7] = { {"X1", 2, "WWPR:PROD-2"},
                               {"X2", 2, "WOPR:PROD-2"},
                             };

        ref_params_vect[8] = { {"X1", 2, "WWPR:PROD-3"},
                               {"X2", 2, "WOPR:PROD-3"},
                             };

        QCOMPARE(check_define(test1g, ref_var_vect, ref_params_vect, ref_expr_vect), true);


        std::vector<std::vector<float>> ref_data;
        std::vector<std::vector<float>> test_data;

        ref_data.push_back(refFile.get<float>("REF_25"));
        ref_data.push_back(refFile.get<float>("REF_26"));
        ref_data.push_back(refFile.get<float>("REF_27"));
        ref_data.push_back(refFile.get<float>("REF_28"));
        ref_data.push_back(refFile.get<float>("REF_29"));
        ref_data.push_back(refFile.get<float>("REF_30"));
        ref_data.push_back(refFile.get<float>("REF_31"));
        ref_data.push_back(refFile.get<float>("REF_32"));
        ref_data.push_back(refFile.get<float>("REF_33"));

        test_data.push_back(derived_smry->get(0, "WWCT2:PROD-1"));
        test_data.push_back(derived_smry->get(0, "WWCT2:PROD-2"));
        test_data.push_back(derived_smry->get(0, "WWCT2:PROD-3"));
        test_data.push_back(derived_smry->get(1, "WWCT2:PROD-1"));
        test_data.push_back(derived_smry->get(1, "WWCT2:PROD-2"));
        test_data.push_back(derived_smry->get(1, "WWCT2:PROD-3"));
        test_data.push_back(derived_smry->get(2, "WWCT2:PROD-1"));
        test_data.push_back(derived_smry->get(2, "WWCT2:PROD-2"));
        test_data.push_back(derived_smry->get(2, "WWCT2:PROD-3"));

        QCOMPARE(test_data.size(), ref_data.size());

        for (size_t n = 0; n < test_data.size(); n++){
            QCOMPARE(test_data[n].size(), ref_data[n].size());

            for (size_t m = 0; m < test_data[n].size(); m++)
                QCOMPARE(test_data[n][m], ref_data[n][m]);
        }


    }
}



QTEST_MAIN(TestQsummary)

#include "test_cmdf_define.moc"
