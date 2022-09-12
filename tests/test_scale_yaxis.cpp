#include <QtTest/QtTest>


#include <appl/smry_appl.hpp>


class TestQsummary: public QObject
{
    Q_OBJECT

private slots:

    void test_1a();
    void test_1b();

    void test_2a();
    void test_2b();
    void test_2c();
    void test_2d();
};



SmryAppl::loader_list_type make_loaders(std::vector<std::string>& fname_list)
{
    SmryAppl::loader_list_type loaders;

    std::vector<std::filesystem::path> smry_files;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ESmry>> esmry_loader;
    std::unordered_map<int, std::unique_ptr<Opm::EclIO::ExtESmry>> ext_esmry_loader;

    std::vector<FileType> ftype_list;

    for (auto& filename : fname_list)
       smry_files.push_back(filename);

    for (auto& smry_file: smry_files){
        std::string ext = smry_file.extension().string();
        if (ext == ".SMSPEC")
            ftype_list.push_back(FileType::SMSPEC);
        else if (ext == ".ESMRY")
            ftype_list.push_back(FileType::ESMRY);
        else
            throw std::invalid_argument("Invalid file type");
    }

    for (size_t n = 0; n < fname_list.size(); n++){
        if (ftype_list[n] == FileType::SMSPEC)
            esmry_loader[n] = std::make_unique<Opm::EclIO::ESmry>(fname_list[n]);
        else if (ftype_list[n] == FileType::ESMRY)
            ext_esmry_loader[n] = std::make_unique<Opm::EclIO::ExtESmry>(fname_list[n]);
        else
            throw std::invalid_argument("Invalid file type");
    }

    loaders = std::make_tuple(smry_files, ftype_list, std::move(esmry_loader), std::move(ext_esmry_loader));

    return loaders;
}


void TestQsummary::test_1a()
{
    std::vector<std::string> fname_list;
    fname_list.push_back("../tests/smry_files/SENS0.ESMRY");

    auto loaders = make_loaders(fname_list);

    std::vector<SmryAppl::vect_input_type> vect_list_c1;

    vect_list_c1 = {
                     std::make_tuple(0, "FOPR", -1, false),
                   };


    SmryAppl::input_list_type input_list;

    input_list = { std::make_tuple(vect_list_c1, ""),
                 };

    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_list, derived_smry);

    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    float a_min = static_cast<float>(yaxis->min());
    float a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 3.0) < 1e-6);
    QVERIFY(abs(a_max - 11.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 0.001) < 1e-6);
}

void TestQsummary::test_1b()
{
    std::vector<std::string> fname_list;
    fname_list.push_back("../tests/smry_files/SENS0.ESMRY");

    auto loaders = make_loaders(fname_list);

    std::vector<SmryAppl::vect_input_type> vect_list_c1;

    vect_list_c1 = {
                     std::make_tuple(0, "FOPR", -1, false),
                   };


    SmryAppl::input_list_type input_list;

    input_list = { std::make_tuple(vect_list_c1, "2019-2 2019-6"),
                 };

    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_list, derived_smry);

    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    float a_min = static_cast<float>(yaxis->min());
    float a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 7.6) < 1e-6);
    QVERIFY(abs(a_max - 10.4) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 0.001) < 1e-6);
}



void TestQsummary::test_2a()
{
    std::vector<std::string> fname_list;
    fname_list.push_back("../tests/smry_files/SENS0.ESMRY");

    auto loaders = make_loaders(fname_list);

    std::vector<SmryAppl::vect_input_type> vect_list_c1;

    vect_list_c1 = {
                     std::make_tuple(0, "FOPR", -1, false),
                   };


    SmryAppl::input_list_type input_list;

    input_list = { std::make_tuple(vect_list_c1, ""),
                 };

    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_list, derived_smry);

    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    float a_min = static_cast<float>(yaxis->min());
    float a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 3.0) < 1e-6);
    QVERIFY(abs(a_max - 11.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 0.001) < 1e-6);

    QLineEdit* cmdline = window.get_cmdline();

    QTest::keyClicks(cmdline, ":xrange 2019-2 2019-6");
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Return );

    a_min = static_cast<float>(yaxis->min());
    a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 7.6) < 1e-6);
    QVERIFY(abs(a_max - 10.4) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 0.001) < 1e-6);
}

void TestQsummary::test_2b()
{
    std::vector<std::string> fname_list;
    fname_list.push_back("../tests/smry_files/SENS0.ESMRY");

    auto loaders = make_loaders(fname_list);

    SmryAppl::input_list_type input_list;

    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_list, derived_smry);

    QLineEdit* cmdline = window.get_cmdline();

    QTest::keyClicks(cmdline, "FGOR");
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Return );

    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    float a_min = static_cast<float>(yaxis->min());
    float a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 80) < 1e-6);
    QVERIFY(abs(a_max - 360) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.0) < 1e-6);

    QTest::keyClicks(cmdline, ":xrange 2019-6 2019-9");
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Return );

    a_min = static_cast<float>(yaxis->min());
    a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 160) < 1e-6);
    QVERIFY(abs(a_max - 320) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.0) < 1e-6);

}


void TestQsummary::test_2c()
{
    std::vector<std::string> fname_list;
    fname_list.push_back("../tests/smry_files/SENS0.ESMRY");
    fname_list.push_back("../tests/smry_files/SENS1.ESMRY");
    fname_list.push_back("../tests/smry_files/SENS2.ESMRY");

    auto loaders = make_loaders(fname_list);

    std::vector<SmryAppl::vect_input_type> vect_list_c1;

    vect_list_c1 = {
                     std::make_tuple(0, "WBHP:PROD-1", -1, false),
                     std::make_tuple(1, "WBHP:PROD-1", -1, false),
                     std::make_tuple(2, "WBHP:PROD-1", -1, false)
                   };


    SmryAppl::input_list_type input_list;

    input_list = { std::make_tuple(vect_list_c1, ""),
                 };

    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_list, derived_smry);

    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    // :ignore zero 1 1

    float a_min = static_cast<float>(yaxis->min());
    float a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 0.0) < 1e-6);
    QVERIFY(abs(a_max - 300.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.00) < 1e-6);

    QLineEdit* cmdline = window.get_cmdline();

    QTest::keyClicks(cmdline, ":ignore zero 1 1");
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Return );

    a_min = static_cast<float>(yaxis->min());
    a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 100.0) < 1e-6);
    QVERIFY(abs(a_max - 240.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.00) < 1e-6);
}

void TestQsummary::test_2d()
{
    std::vector<std::string> fname_list;
    fname_list.push_back("../tests/smry_files/SENS0.ESMRY");
    fname_list.push_back("../tests/smry_files/SENS1.ESMRY");
    fname_list.push_back("../tests/smry_files/SENS2.ESMRY");

    auto loaders = make_loaders(fname_list);

    std::vector<SmryAppl::vect_input_type> vect_list_c1;

    vect_list_c1 = {
                     std::make_tuple(0, "WBHP:PROD-1", -1, false),
                     std::make_tuple(1, "WBHP:PROD-1", -1, false),
                     std::make_tuple(2, "WBHP:PROD-1", -1, false)
                   };


    SmryAppl::input_list_type input_list;

    input_list = { std::make_tuple(vect_list_c1, "2018-12 2019-8"),
                 };

    std::unique_ptr<DerivedSmry> derived_smry;

    SmryAppl window(fname_list, loaders, input_list, derived_smry);

    SmryYaxis* yaxis = window.get_smry_yaxis(0, 0);

    // :ignore zero 1 1

    float a_min = static_cast<float>(yaxis->min());
    float a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 0.0) < 1e-6);
    QVERIFY(abs(a_max - 250.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.00) < 1e-6);

    QLineEdit* cmdline = window.get_cmdline();

    QTest::keyClicks(cmdline, ":ignore zero 1 1");
    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Return );

    a_min = static_cast<float>(yaxis->min());
    a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 100.0) < 1e-6);
    QVERIFY(abs(a_max - 240.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.00) < 1e-6);

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_R, Qt::ControlModifier);

    a_min = static_cast<float>(yaxis->min());
    a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 0.0) < 1e-6);
    QVERIFY(abs(a_max - 300.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.00) < 1e-6);

    QTest::keyEvent(QTest::Click, cmdline, Qt::Key_Z, Qt::ControlModifier);

    a_min = static_cast<float>(yaxis->min());
    a_max = static_cast<float>(yaxis->max());

    QVERIFY(abs(a_min - 100.0) < 1e-6);
    QVERIFY(abs(a_max - 240.0) < 1e-6);
    QVERIFY(abs(yaxis->multiplier() - 1.00) < 1e-6);
}


QTEST_MAIN(TestQsummary)

#include "test_scale_yaxis.moc"
